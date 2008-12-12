#include "ti_types.h"

TiWindow::TiWindow(TiUserWindow* window) : TiApiPoint(window) {
    this->set_property("currentWindow", *window);
}

