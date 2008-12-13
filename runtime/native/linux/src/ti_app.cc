#include <stdio.h>
#include <stdlib.h>

#include "ti_types.h"

TiApp::TiApp(TiUserWindow* window) : TiApiPoint(window) {
    this->BindMethod("debug", &TiApp::Debug);
}

TiValue TiApp::Debug(size_t num_args, TiValue args[]) {

    if (num_args <= 0) {
        return this->Undefined();
    }
    std::string message = args[0].GetString();
    printf("%s\n", message.c_str());

    return this->Undefined();
}
