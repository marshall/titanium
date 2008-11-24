#include "ti_runtime.h"
#include "ti_web_shell.h"
#include "ti_window_factory.h"
#include "Resource.h"

#include <string>
#include <fstream>

TiRuntime::TiRuntime(TiWebShell *tiWebShell)
{
	this->tiWebShell = tiWebShell;
	
	tiApp = new TiApp(tiWebShell);
	tiWindowFactory = new TiWindowFactory();

	App.Set(tiApp->ToNPObject());
	Window.Set(tiWindowFactory->ToNPObject());

	BindProperty("App", &App);
	BindProperty("Window", &Window);
}

TiRuntime::~TiRuntime()
{
}
