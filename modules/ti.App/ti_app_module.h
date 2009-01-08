#ifndef TI_APP_MODULE_H_
#define TI_APP_MODULE_H_

// this is basically pseudo code
// until i can see what's going on in kroll
#include <kroll.h>

namespace ti {

class AppModule : public kroll::Module, kroll::BoundObject
{
	KROLL_MODULE(AppModule)


public:

	virtual TiValue* Get(std::string &name, TiBoundObject *context_local);
	virtual void Set(std::string &name, TiValue *value, TiBoundObject *context_local);
	virtual TiValue* Call(std::string &name, const kroll::ArgList &args, TiBoundObject *context_local);
};

}
#endif
