#include "ti_types.h"

TiWindow::TiWindow(TiUserWindow* window) : TiApiPoint(window) {
    this->SetProperty("currentWindow", *window);
}

