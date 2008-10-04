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


DECLARE_DISPATCHER(Appcelerator);

Appcelerator *app;

// static
template<>
void Dispatcher<Appcelerator>::Init() 
{
  RegisterMethod("readFile", &Appcelerator::ReadFile);
  RegisterMethod("writeFile", &Appcelerator::WriteFile);
  RegisterMethod("createProject", &Appcelerator::CreateProject);
  RegisterMethod("compileProject", &Appcelerator::CompileProject);
  RegisterMethod("boot", &Appcelerator::BootAppcelerator);
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

  std::vector<uint8> data;
  if (File::ReadFileToVector(filename.c_str(), &data)) 
  {
	  std::string16 content;
	  if (data.size() > 0 && UTF8ToString16(reinterpret_cast<char*>(&data[0]), &content)) 
	  {
		  if (content.length() > 0)
		  {
			  	context->SetReturnValue(JSPARAM_STRING16, &content);
				return;
		  }
	  }
  }
  std::string16 empty = std::string16(STRING16(L""));
  context->SetReturnValue(JSPARAM_STRING16, &empty);
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

void Appcelerator::CompileProject(JsCallContext *context)
{
	std::string16 filename;
    JsRootedCallback *complete = NULL;
    JsRootedCallback *output = NULL;
    JsArgument argv[] = 
    { 
	    { JSPARAM_REQUIRED, JSPARAM_STRING16, &filename },
	    { JSPARAM_OPTIONAL, JSPARAM_FUNCTION, &output },
        { JSPARAM_OPTIONAL, JSPARAM_FUNCTION, &complete }
    };
 
    context->GetArguments(ARRAYSIZE(argv), argv);

    if (context->is_exception_set()) {
		return;
  	}
    
	std::string cmd;
	String16ToUTF8(filename.c_str(),&cmd);

	ProcessThread *t = new ProcessThread(cmd.c_str(),output,complete,this);
	t->Start();

	bool returnValue = true;
	context->SetReturnValue(JSPARAM_BOOL, &returnValue);
}

std::string String16ToString (std::string16 string16) {
	std::string str;
	String16ToUTF8(string16.c_str(), &str);
	return str;
}

void Appcelerator::CreateProject(JsCallContext *context)
{
	std::string16 path, name, service, serverURL;
	JsRootedCallback *complete = NULL, *output = NULL;
	
	JsArgument argv[] = {
		{ JSPARAM_REQUIRED, JSPARAM_STRING16, &path },
		{ JSPARAM_REQUIRED, JSPARAM_STRING16, &name },
		{ JSPARAM_REQUIRED, JSPARAM_STRING16, &service },
		{ JSPARAM_OPTIONAL, JSPARAM_STRING16, &serverURL },
		{ JSPARAM_OPTIONAL, JSPARAM_FUNCTION, &output },
		{ JSPARAM_OPTIONAL, JSPARAM_FUNCTION, &complete }
	};
	
	context->GetArguments(ARRAYSIZE(argv), argv);
	
	if (context->is_exception_set()) {
		return;
	}
	
	std::string command = "/usr/bin/app create:project ";
	command += "\"" + String16ToString(path) + "\" ";
	command += "\"" + String16ToString(name) + "\" ";
	command += "\"" + String16ToString(service) + "\" ";
	if (serverURL.length() > 0) {
		command += " --server=" + String16ToString(serverURL);
	}
	
	std::cout << "executing: " << command << std::endl;
	ProcessThread *t = new ProcessThread(command.c_str(), output, complete, this);
	t->Start();
	
	bool returnValue = true;
	context->SetReturnValue(JSPARAM_BOOL, &returnValue);
}

void Appcelerator::BootAppcelerator (JsCallContext *context)
{
	std::string16 path;
	
	JsArgument argv[] = {
		{ JSPARAM_REQUIRED, JSPARAM_STRING16, &path },
		{ JSPARAM_REQUIRED, JSPARAM_OBJECT, &this->bootCallback }
	};
	
	context->GetArguments(ARRAYSIZE(argv), argv);
	if (context->is_exception_set()) {
		return;
	}
	
	rubyWrapper.RunScript(String16ToUTF8(path).c_str());
}
