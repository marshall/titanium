#include <stdlib.h>
#include "ti_types.h"

TiRuntime::TiRuntime(TiUserWindow* window) : TiApiPoint(window) {

    TiApp ti_app = TiApp(window);
    this->set_property("App", ti_app);

    TiWindow ti_window = TiWindow(window);
    this->set_property("Window", ti_window);

    //TiDock ti_dock = TiDock(window);
    //this->set_property("Dock", ti_dock);

    //TiFilesystem ti_filesystem = TiFilesystem(window);
    //this->set_property("Filesystem", ti_filesystem);

    //TiFile ti_file = TiFile(window);
    //this->set_property("File", ti_file);

    //TiDesktop ti_desktop = TiDesktop(window);
    //this->set_property("Desktop", ti_desktop);

    //TiMenu ti_menu = TiMenu(window);
    //this->set_property("Menu", ti_menu);

}
