#include "ti_types.h"
#include <stdlib.h>

TiWindow::TiWindow(JSContextRef context, JSObjectRef window)
        : TiObject(context, window) {

    JSObjectRef global_object = JSContextGetGlobalObject(context);
    this->global = TiObject(context, global_object);

    TiRuntime ti_runtime = TiRuntime(this);
    this->global.set_property("tiRuntime", ti_runtime);

}

