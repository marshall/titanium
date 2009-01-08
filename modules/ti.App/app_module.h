#ifndef TI_APP_MODULE_H_
#define TI_APP_MODULE_H_

// this is basically pseudo code
// until i can see what's going on in kroll
#include <kroll.h>

namespace ti {

class AppModule : public kroll::Module, public kroll::BoundObject
{
	KROLL_MODULE_CLASS(AppModule)

public:

	virtual kroll::Value* Get(const char *name, kroll::BoundObject *context);
	virtual void Set(const char *name, kroll::Value *value, kroll::BoundObject *context);
	virtual kroll::Value* Call(const char *name, const kroll::ArgList &args, kroll::BoundObject *context);

	virtual std::vector<std::string> GetPropertyNames ();
};

}
#endif
