#include "ti_runtime.h"
#include "ti_web_shell.h"
#include "Resource.h"

#include <string>
#include <fstream>

TiRuntime::TiRuntime(TiWebShell *tiWebShell)
{
	this->tiWebShell = tiWebShell;

	BindMethod("debug", &TiRuntime::debug);
	BindMethod("getResourcePath", &TiRuntime::getResourcePath);
	BindMethod("include", &TiRuntime::include);
}

TiRuntime::~TiRuntime()
{
}

void TiRuntime::debug (const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0)
		printf("[titanium:debug]: %s\n", args[0].ToString().c_str());
}

void TiRuntime::getResourcePath(const CppArgumentList &args, CppVariant *result)
{
	std::wstring resourcePath = tiWebShell->getResourcesPath();

	result->Set(WideToUTF8(resourcePath));
}

void TiRuntime::include(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0) {
		std::string relativeName = args[0].ToString();
		tiWebShell->include(relativeName);
	}
}
