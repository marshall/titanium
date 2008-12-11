#include <stdlib.h>
#include "ti_types.h"

TiRuntime::TiRuntime(TiWindow* window) : TiApiPoint(window) {

    TiApp ti_app = TiApp(window);
    this->set_property("App", ti_app);

    //TiApp ti_dock = TiDock(window);
    //obj.set_property("Dock", ti_dock);

    //TiApp ti_filesystem = TiFilesystem(window);
    //obj.set_property("Filesystem", ti_filesystem);

    //TiApp ti_file = TiFile(window);
    //obj.set_property("File", ti_file);

    //TiApp ti_desktop = TiDesktop(window);
    //obj.set_property("Desktop", ti_desktop);

    //TiApp ti_menu = TiMenu(window);
    //obj.set_property("Menu", ti_menu);

    //TiApp ti_user_window = TiUserWindow(window);
    //obj.set_property("Window", ti_user_window);
}
