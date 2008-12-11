#ifndef __TI_TYPES_H
#define __TI_TYPES_H

#include <webkit/webkit.h>
#include <JavaScriptCore/JSObjectRef.h>
#include <JavaScriptCore/JSStringRef.h>
#include <JavaScriptCore/JSContextRef.h>
#include "js_wrapper.h"

class TiWindow : public TiObject {
    public:
        TiWindow() {}
        TiWindow(JSContextRef, JSObjectRef);


    protected:
        TiObject global;
};

class TiApiPoint : public TiObject {
    public:
        TiApiPoint(TiWindow* window);
        TiWindow* get_window();

    protected:
        TiWindow* window;

};

class TiRuntime : public TiApiPoint {
    public:
        TiRuntime(TiWindow* window);

    protected:
};

class TiApp : public TiApiPoint {
    public:
        TiApp(TiWindow* window);
        TiValue debug(size_t num_args, TiValue args[]);
};

class TiFilesystem : public TiApiPoint {
    public:
        TiFilesystem(TiWindow* window);
};

class TiFile : public TiApiPoint {
    public:
        TiFile(TiWindow* window);
};

class TiDesktop : public TiApiPoint {
    public:
        TiDesktop(TiWindow* window);
};

class TiMenu : public TiApiPoint {
    public:
        TiMenu(TiWindow* window);
};

class TiUserWindow : public TiApiPoint {
    public:
        TiUserWindow(TiWindow* window);
};

#endif
