#include "gears/appcelerator/appcelerator.h"

#include <limits>

#include "gears/base/common/base_class.h"
#include "gears/base/common/common.h"

#include "gears/base/common/file.h"
#include "gears/base/common/thread.h"

#include "gears/base/common/dispatcher.h"
#include "gears/base/common/module_wrapper.h"
#include "gears/base/common/js_runner.h"

#include <iostream>
#include <fstream>

void _debug (const char* file, int line, const char* message) {
	printf("[titanium] [%s:%d] %s\n", file, line, message);
}


DECLARE_DISPATCHER(Appcelerator);

Appcelerator *app;

// static
template<>
void Dispatcher<Appcelerator>::Init() 
{
  RegisterMethod("readFile", &Appcelerator::ReadFile);
  RegisterMethod("writeFile", &Appcelerator::WriteFile);
	RegisterMethod("getFileTree", &Appcelerator::GetFileTree);
	RegisterMethod("getUserHome", &Appcelerator::GetUserHome);
}

const std::string Appcelerator::kModuleName("Appcelerator");

Appcelerator::Appcelerator() : ModuleImplBaseClass(kModuleName) 
{
	app = this;
	rubyBooted = false;
}

Appcelerator::~Appcelerator() 
{
}

void Appcelerator::HandleEvent(JsEventType event_type) 
{
  assert(event_type == JSEVENT_UNLOAD);
}

void Appcelerator::WriteFile(JsCallContext *context) 
{
  std::string16 filename;
  std::string16 content;

  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &filename },
	{ JSPARAM_REQUIRED, JSPARAM_STRING16, &content }
  };

  context->GetArguments(ARRAYSIZE(argv), argv);

  if (context->is_exception_set())
    return;

  // ensure there's a file
  File::CreateNewFile(filename.c_str());

  bool returnValue = false;

  std::string str_utf8;
  if (String16ToUTF8(content.data(), content.length(), &str_utf8))
  {
	  std::vector<uint8> buffer0;
	  const void *data = str_utf8.c_str();
  	  size_t length = str_utf8.size();
	  buffer0.insert(buffer0.end(), (uint8 *)data, ((uint8 *)data)+length);
	  
	  if (File::WriteVectorToFile(filename.c_str(),&buffer0))
	  {
	     returnValue = true;
	  }
  }

  context->SetReturnValue(JSPARAM_BOOL, &returnValue);
}

void Appcelerator::ReadFile(JsCallContext *context) 
{
  std::string16 filename;
	
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &filename }
  };

  context->GetArguments(ARRAYSIZE(argv), argv);

  if (context->is_exception_set())
    return;

	printf("opening file %s\n", String16ToUTF8(filename).c_str());
	
	std::ifstream filestream;
	std::string contents;
	
	filestream.open(String16ToUTF8(filename).c_str());
	if (filestream.is_open()) {
		std::string str;
		std::getline(filestream, str);
		while (filestream) {
			contents += std::string(str + "\n");
			std::getline(filestream, str);
		}
		filestream.close();
		
		printf("return value: %s\n", contents.c_str());
	}
	
	std::string16 contents16(UTF8ToString16(contents).c_str());
	
  context->SetReturnValue(JSPARAM_STRING16, &contents16);
}

ProcessThread::ProcessThread(const char* c, JsRootedCallback *oc,  JsRootedCallback *cc,ProcessThreadListener *l)  : cmd(c)
{
	ThreadMessageQueue *msg_q = ThreadMessageQueue::GetInstance();
	msg_q->InitThreadMessageQueue();
	this->threadid = msg_q->GetCurrentThreadId();

	this->outputCallback.reset(oc);
	this->completeCallback.reset(cc);
	this->listener.reset(l);
}

ProcessThread::~ProcessThread()
{
	this->listener.release();
	this->outputCallback.release();
	this->completeCallback.release();
}

void ProcessThread::Run()
{
	FILE *f = popen(this->cmd,"r");

	JsRootedCallback *ocb = this->outputCallback.get();

	char str[LINE_MAX];

	while ( fgets(str,LINE_MAX,f) != NULL )
	{
		std::string16 output = UTF8ToString16(str);
		std::cout << output.c_str() << std::endl;

    AsyncRouter::GetInstance()->CallAsync(this->threadid, new ProcessOutputFunctor(this->listener.get(),ocb,output));
	}


	int rc = pclose(f);
    AsyncRouter::GetInstance()->CallAsync(this->threadid, new ProcessCompleteFunctor(this->listener.get(),this->completeCallback.get(),rc));
}

void Appcelerator::ProcessCallback(int eventType,JsRootedCallback *handler,void *event)
{
	if (handler)
	{
		JsParamType type;
		switch(eventType)
		{
			case PROCESS_CALLBACK_OUTPUT:
			{
				type = JSPARAM_STRING16;
				break;
			}
			case PROCESS_CALLBACK_COMPLETED:
			{
				type = JSPARAM_INT;
				break;
			}
		}
		JsParamToSend send_argv[] = {
			{ type, &event }
		};
	  	GetJsRunner()->InvokeCallback(handler, ARRAYSIZE(send_argv), send_argv, NULL);
	}
}

/**
 * Retrieve all files and directories under a specific file system path
 */
void Appcelerator::GetFileTree(JsCallContext *context)
{
	std::string16 path16;
	bool recursive = true;
	
	JsArgument argv[] = {
		{ JSPARAM_REQUIRED, JSPARAM_STRING16, &path16 },
		{ JSPARAM_OPTIONAL, JSPARAM_BOOL, &recursive}
	};
	
	context->GetArguments(ARRAYSIZE(argv), argv);
	if (context->is_exception_set()) return;
	
	std::string path = String16ToUTF8(path16);
	
	DIR *dir;
	if ((dir = opendir(path.c_str())) == NULL) {
		std::cerr << "Error(" << errno << ") opening " << path << std::endl;
		return;
	}
	
	scoped_ptr<JsArray> fileTree(GetJsRunner()->NewArray());
	AddToFileTree(path, dir, fileTree.get());
	
	closedir(dir);
	
	context->SetReturnValue(JSPARAM_ARRAY, fileTree.get());
}

void Appcelerator::AddToFileTree(std::string path, DIR *dir, JsArray *array)
{	
	struct dirent *dirEntry;
	int i = 0;
	while ((dirEntry = readdir(dir)) != NULL) {
		if (strcmp(dirEntry->d_name, ".") == 0) continue;
		if (strcmp(dirEntry->d_name, "..") == 0) continue;
	
		scoped_ptr<JsObject> fileObject(GetJsRunner()->NewObject());
		
		if (dirEntry->d_type == DT_DIR) {
			DIR *dir;
			std::string newpath = path + "/" + dirEntry->d_name;
			
			if ((dir = opendir(newpath.c_str())) == NULL) {
				std::cerr << "Error(" << errno << ") opening " << newpath << std::endl;
				return;
			}

			CreateJsFileObject(UTF8ToString16(newpath.c_str()), UTF8ToString16(dirEntry->d_name), 0, true, fileObject.get());
			
			scoped_ptr<JsArray> fileTree(GetJsRunner()->NewArray());
			AddToFileTree(newpath, dir, fileTree.get());
			
			closedir(dir);
			
			if (!fileObject->SetPropertyArray(STRING16(L"files"), fileTree.get()))
			{
				std::cerr << "Error: couldn't set \"files\" property on object" << std::endl;
				return;
			}
		}
		else if (dirEntry->d_type == DT_REG) {
			struct stat fileStats;
			std::string filePath = path + "/" + dirEntry->d_name;
			
			stat(filePath.c_str(), &fileStats);
			
			CreateJsFileObject(UTF8ToString16(filePath.c_str()), UTF8ToString16(dirEntry->d_name), (int)fileStats.st_size, false, fileObject.get());
		}
		
		if (!array->SetElementObject(i, fileObject.get())) {
			std::cerr << "Failed to add File to array." << std::endl;
			return;
    }
		i++;
	}
}

void Appcelerator::CreateJsFileObject(std::string16 path, std::string16 name, int size, bool isDir, JsObject *object)
{
	if (!object->SetPropertyString(STRING16(L"path"), path)) {
		std::cerr << "Failed to set property \"path\" on file object." << std::endl;
	}
	if (!object->SetPropertyString(STRING16(L"name"), name)) {
		std::cerr << "Failed to set property \"name\" on file object." << std::endl;
	}
	if (!object->SetPropertyInt(STRING16(L"size"), size)) {
		std::cerr << "Failed to set property \"size\" on file object." << std::endl;
	}
	if (!object->SetPropertyBool(STRING16(L"isDir"), isDir)) {
		std::cerr << "Failed to set property \"isDir\" on file object." << std::endl;
	}
}

#include <pwd.h>
#include <unistd.h>
void Appcelerator::GetUserHome(JsCallContext *context)
{
	char *username = getlogin();
	struct passwd *pwent = getpwnam(username);
	std::string16 homeDir(UTF8ToString16(std::string(pwent->pw_dir)).c_str());
	
	context->SetReturnValue(JSPARAM_STRING16, &homeDir);
}