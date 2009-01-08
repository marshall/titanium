#ifndef TI_APP_MODULE_H_
#define TI_APP_MODULE_H_

#include <kroll/kroll.h>

namespace ti {

class AppModule :
	public kroll::Module
{
	KROLL_MODULE_CLASS(AppModule)

public:

	/*virtual kroll::Value* Get(const char *name, kroll::BoundObject *context);
	virtual void Set(const char *name, kroll::Value *value, kroll::BoundObject *context);*/
	//kroll::Value* Call(const char *name, const kroll::ValueList &args, kroll::BoundObject *context);

	//virtual std::vector<std::string> GetPropertyNames ();
};

}
#endif
