#ifndef TI_NATIVE_H_
#define TI_NATIVE_H_

#include "webkit/glue/cpp_bound_class.h"
#include "webkit/glue/webview.h"

class TiWebShell;

class TiRuntime : public CppBoundClass
{
	TiWebShell *tiWebShell;
public:
	TiRuntime(TiWebShell *tiWebShell);
	~TiRuntime(void);

	void debug (const CppArgumentList &args, CppVariant *result);
	void getResourcePath(const CppArgumentList &args, CppVariant *result);
	void include (const CppArgumentList &args, CppVariant *result);
};

#endif