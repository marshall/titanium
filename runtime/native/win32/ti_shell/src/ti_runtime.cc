#include "ti_runtime.h"
#include "ti_web_shell.h"
#include "Resource.h"

#include <string>
#include <fstream>

TiRuntime::TiRuntime (TIWebShell *ti_web_shell)
{
	this->ti_web_shell = ti_web_shell;

	BindMethod("debug", &TiRuntime::debug);
	BindMethod("getResourcePath", &TiRuntime::getResourcePath);
	BindMethod("include", &TiRuntime::include);
	BindMethod("show", &TiRuntime::show);
	BindMethod("hide", &TiRuntime::hide);
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
	std::wstring resourcePath = ti_web_shell->getResourcesPath();

	result->Set(WideToUTF8(resourcePath));
}

void TiRuntime::include(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0) {
		std::string relativeName = args[0].ToString();
		ti_web_shell->include(relativeName);
	}
}

void TiRuntime::hide (const CppArgumentList &args, CppVariant *result)
{
	this->ti_web_shell->showWindow(SW_HIDE);
	this->ti_web_shell->createTrayIcon();
}

void TiRuntime::show (const CppArgumentList &args, CppVariant *result)
{
	this->ti_web_shell->showWindow(SW_SHOW);
	this->ti_web_shell->removeTrayIcon();
}