#include <stdlib.h>
#include "ti_types.h"

TiRuntime::TiRuntime(TiUserWindow* window) : TiApiPoint(window) {

    TiApp ti_app = TiApp(window);
    this->SetProperty("App", ti_app);

    TiWindow ti_window = TiWindow(window);
    this->SetProperty("Window", ti_window);

    //TiDock ti_dock = TiDock(window);
    //this->SetProperty("Dock", ti_dock);

    //TiFilesystem ti_filesystem = TiFilesystem(window);
    //this->SetProperty("Filesystem", ti_filesystem);

    //TiFile ti_file = TiFile(window);
    //this->SetProperty("File", ti_file);

    //TiDesktop ti_desktop = TiDesktop(window);
    //this->SetProperty("Desktop", ti_desktop);

    //TiMenu ti_menu = TiMenu(window);
    //this->SetProperty("Menu", ti_menu);

}
