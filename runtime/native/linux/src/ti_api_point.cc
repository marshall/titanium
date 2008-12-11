#include "ti_types.h"

TiApiPoint::TiApiPoint(TiWindow *window) {
    JSObjectRef object = JSObjectMake(window->get_context(), NULL, NULL);
    this->context = window->get_context();
    this->object = object;
    this->value = object;
}

TiWindow* TiApiPoint::get_window() {
    return this->window;
}
