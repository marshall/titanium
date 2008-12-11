#include <stdio.h>
#include <stdlib.h>

#include "ti_types.h"

TiApp::TiApp(TiWindow* window) : TiApiPoint(window) {
  this->bind_method("debug", &TiApp::debug);
}

TiValue TiApp::debug(size_t num_args, TiValue args[]) {

    if (num_args <= 0) {
        return this->undefined();
    }

    char* message = args[0].get_chars();
    if (message == NULL) {
        return this->undefined();
    }

    printf("%s\n", message);
    free(message);

    return this->undefined();
}
