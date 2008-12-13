#include <stdio.h>
#include <stdlib.h>

#include "ti_types.h"

TiApp::TiApp(TiUserWindow* window) : TiApiPoint(window) {
    this->bind_method("debug", &TiApp::debug);
}

TiValue TiApp::debug(size_t num_args, TiValue args[]) {

    if (num_args <= 0) {
        return this->undefined();
    }
    std::string message = args[0].get_string();
    printf("%s\n", message.c_str());

    return this->undefined();
}
