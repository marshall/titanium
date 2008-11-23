#ifndef TI_NATIVE_H_
#define TI_NATIVE_H_

#include "webkit/glue/cpp_bound_class.h"
#include "webkit/glue/webview.h"

class TIWebShell;

class TiRuntime : public CppBoundClass
{
	TIWebShell *tiWebShell;
public:
	TiRuntime(TIWebShell *tiWebShell);
	~TiRuntime(void);

	void debug (const CppArgumentList &args, CppVariant *result);
	void getResourcePath(const CppArgumentList &args, CppVariant *result);
	void include (const CppArgumentList &args, CppVariant *result);
	void hide (const CppArgumentList &args, CppVariant *result);
	void show (const CppArgumentList &args, CppVariant *result);
};

#endif