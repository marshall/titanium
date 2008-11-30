// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the TITANIUM_DLL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// TITANIUM_DLL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef TITANIUM_DLL_EXPORTS
#define TITANIUM_DLL_API __declspec(dllexport)
#else
#define TITANIUM_DLL_API __declspec(dllimport)
#endif

#include <string>
#include <map>
#include "ti_app_config.h"

using namespace std;

extern "C" {
TITANIUM_DLL_API int __cdecl runTitaniumApp(wstring &appXmlPath);
TITANIUM_DLL_API int __cdecl runTitaniumAppDev(wstring &appXmlPath, wstring &projectPath, wstring &runtimePath, map<wstring,wstring> &pluginPaths);
}
