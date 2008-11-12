#ifndef TI_NATIVE_H_
#define TI_NATIVE_H_

#include "webkit/glue/cpp_bound_class.h"

class TIWebShell;

class TiNative : public CppBoundClass
{
	TIWebShell *ti_web_shell;
public:
	TiNative(TIWebShell *ti_web_shell);

	void debug (const CppArgumentList &args, CppVariant *result);
	void getResourcePath(const CppArgumentList &args, CppVariant *result);
};

#endif