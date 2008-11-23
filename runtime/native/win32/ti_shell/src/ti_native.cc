#include "ti_native.h"
#include "ti_web_shell.h"
#include "Resource.h"

#include <string>
#include <fstream>

TiNative::TiNative (TiWebShell *ti_web_shell)
{
	this->ti_web_shell = ti_web_shell;

	BindMethod("debug", &TiNative::debug);
	BindMethod("getResourcePath", &TiNative::getResourcePath);
	BindMethod("include", &TiNative::include);
	BindMethod("show", &TiNative::show);
	BindMethod("hide", &TiNative::hide);
}

TiNative::~TiNative()
{
}

void TiNative::debug (const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0)
		printf("[titanium:debug]: %s\n", args[0].ToString().c_str());
}

void TiNative::getResourcePath(const CppArgumentList &args, CppVariant *result)
{
	std::wstring resourcePath = ti_web_shell->getTiWindow()->getApp()->getResourcePath();

	result->Set(WideToUTF8(resourcePath));
}

void TiNative::include(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0) {
		std::string relativeName = args[0].ToString();
		ti_web_shell->include(relativeName);
	}
}

void TiNative::hide (const CppArgumentList &args, CppVariant *result)
{
	this->ti_web_shell->showWindow(SW_HIDE);
	this->ti_web_shell->createTrayIcon();
}

void TiNative::show (const CppArgumentList &args, CppVariant *result)
{
	this->ti_web_shell->showWindow(SW_SHOW);
	this->ti_web_shell->removeTrayIcon();
}