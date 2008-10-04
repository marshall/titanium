
#ifndef GEARS_APPCELERATOR_H__
#define GEARS_APPCELERATOR_H__

#include "gears/base/common/base_class.h"
#include "gears/base/common/common.h"
#include "gears/base/common/js_runner.h"
#include "gears/base/common/thread.h"
#include "gears/base/common/async_router.h"

#include "gears/appcelerator/ruby_wrapper.h"

class ProcessThreadListener {
 public:
  virtual void ProcessCallback(int eventType,JsRootedCallback *cb,void *event){}
};

class Appcelerator
    : public ModuleImplBaseClass,
      public ProcessThreadListener,
      public JsEventHandlerInterface {
 public:
  static const std::string kModuleName;
  JsObject bootCallback;
  bool rubyBooted;
  RubyWrapper rubyWrapper;
  
  Appcelerator();

  // IN: -
  // OUT: -
  void ReadFile(JsCallContext *context);
  void WriteFile(JsCallContext *context);
  void CompileProject(JsCallContext *context);
  void CreateProject(JsCallContext *context);
  void BootAppcelerator(JsCallContext *context);

  void ProcessCallback(int eventType,JsRootedCallback *cb,void *event);
  JsObject& GetBootCallback() { return bootCallback; }
  
 private:

  // JsEventHandlerInterface implementation.
  virtual void HandleEvent(JsEventType event_type);

  ~Appcelerator();
  DISALLOW_EVIL_CONSTRUCTORS(Appcelerator);
};

#define PROCESS_CALLBACK_OUTPUT 1
#define PROCESS_CALLBACK_COMPLETED 2

class ProcessOutputFunctor : public AsyncFunctor {
public:
	ProcessOutputFunctor(ProcessThreadListener *tl,JsRootedCallback *c,std::string16 out) : listener(tl),callback(c),out(out) { }
    void Run() {
		this->listener.get()->ProcessCallback(PROCESS_CALLBACK_OUTPUT,this->callback.get(),(void*)out.c_str());
    }
	scoped_ptr<ProcessThreadListener> listener;
	scoped_ptr<JsRootedCallback> callback;
	std::string16 out;
};

class ProcessCompleteFunctor : public AsyncFunctor {
public:
	ProcessCompleteFunctor(ProcessThreadListener *tl,JsRootedCallback *c,int rc) : listener(tl),callback(c),rc(rc) { }
    void Run() {
		this->listener.get()->ProcessCallback(PROCESS_CALLBACK_COMPLETED,this->callback.get(),(void*)rc);
    }
	scoped_ptr<ProcessThreadListener> listener;
	scoped_ptr<JsRootedCallback> callback;
	int rc;
};

class ProcessThread : public Thread
{
	public:
		ProcessThread(const char*, JsRootedCallback *ocb, JsRootedCallback *ccb, ProcessThreadListener *l);
		~ProcessThread();
		void Run();
	private:
		const char *cmd;
		scoped_ptr<JsRootedCallback> outputCallback;		
		scoped_ptr<JsRootedCallback> completeCallback;		
		scoped_ptr<ProcessThreadListener> listener;
		ThreadId threadid;
};
#endif // GEARS_APPCELERATOR_H__
