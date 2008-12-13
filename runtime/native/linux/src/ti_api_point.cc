#include "ti_types.h"

TiApiPoint::TiApiPoint(TiUserWindow *window) {
    JSObjectRef object = JSObjectMake(window->GetContext(), NULL, NULL);
    this->context = window->GetContext();
    this->object = object;
    this->value = object;
}

TiUserWindow* TiApiPoint::GetWindow() {
    return this->window;
}
