#include "ti_types.h"
#include <stdlib.h>

#define STUB printf("__FILE__:__LINE__: called a stub method!\n")

void TiUserWindow::WindowObjectCleared(JSContextRef context, JSObjectRef window) {

    JSObjectRef global_object = JSContextGetGlobalObject(context);

    this->context = context;
    this->object = window;
    this->value = window;
    this->global = TiObject(context, global_object);

    TiRuntime ti_runtime = TiRuntime(this);
    this->global.SetProperty("tiRuntime", ti_runtime);

    /* this object is accessed by tiRuntime.Window.currentWindow */
    this->BindMethod("hide", &TiUserWindow::hide_cb);
    this->BindMethod("show", &TiUserWindow::show_cb);
    this->BindMethod("isUsingChrome", &TiUserWindow::is_using_chrome_cb);
    this->BindMethod("isFullScreen", &TiUserWindow::is_full_screen_cb);
    this->BindMethod("getId", &TiUserWindow::get_id_cb);
    this->BindMethod("open", &TiUserWindow::open_cb);
    this->BindMethod("close", &TiUserWindow::close_cb);
    this->BindMethod("getX", &TiUserWindow::get_x_cb);
    this->BindMethod("setX", &TiUserWindow::set_x_cb);
    this->BindMethod("getY", &TiUserWindow::get_y_cb);
    this->BindMethod("setY", &TiUserWindow::set_y_cb);
    this->BindMethod("getWidth", &TiUserWindow::get_width_cb);
    this->BindMethod("setWidth", &TiUserWindow::set_width_cb);
    this->BindMethod("getHeight", &TiUserWindow::get_height_cb);
    this->BindMethod("setHeight", &TiUserWindow::set_height_cb);
    this->BindMethod("getBounds", &TiUserWindow::get_bounds_cb);
    this->BindMethod("setBounds", &TiUserWindow::set_bounds_cb);
    this->BindMethod("getTitle", &TiUserWindow::get_title_cb);
    this->BindMethod("setTitle", &TiUserWindow::set_title_cb);
    this->BindMethod("getUrl", &TiUserWindow::get_url_cb);
    this->BindMethod("setUrl", &TiUserWindow::set_url_cb);
    this->BindMethod("isResizable", &TiUserWindow::is_resizable_cb);
    this->BindMethod("setResizable", &TiUserWindow::set_resizable_cb);
    this->BindMethod("isMaximizable", &TiUserWindow::is_maximizable_cb);
    this->BindMethod("setMaimizable", &TiUserWindow::set_maximizable_cb);
    this->BindMethod("isMinimizable", &TiUserWindow::is_minimizable_cb);
    this->BindMethod("setMinizable", &TiUserWindow::set_minimizable_cb);
    this->BindMethod("isCloseable", &TiUserWindow::is_closeable_cb);
    this->BindMethod("setCloseable", &TiUserWindow::set_closeable_cb);
    this->BindMethod("isVisible", &TiUserWindow::is_visible_cb);
    this->BindMethod("setVisible", &TiUserWindow::set_visible_cb);
    this->BindMethod("getTransparency", &TiUserWindow::get_transparency_cb);
    this->BindMethod("setTransparency", &TiUserWindow::set_transparency_cb);
}

TiValue TiUserWindow::hide_cb(size_t num_args, TiValue args[]) {
    this->Hide();
    return this->Undefined();
}

TiValue TiUserWindow::show_cb(size_t num_args, TiValue args[]) {
    this->Show();
    return this->Undefined();
}

TiValue TiUserWindow::is_using_chrome_cb(size_t num_args, TiValue args[]) {
    bool answer = this->IsUsingChrome();
    return this->NewValue(answer);
}

TiValue TiUserWindow::is_using_scrollbars_cb(size_t num_args, TiValue args[]) {
    bool answer = this->IsUsingScrollbars();
    return this->NewValue(answer);
}

TiValue TiUserWindow::is_full_screen_cb(size_t num_args, TiValue args[]) {
    bool answer = this->IsFullScreen();
    return this->NewValue(answer);
}

TiValue TiUserWindow::get_id_cb(size_t num_args, TiValue args[]) {
    std::string ret = this->GetId();
    return this->NewValue(ret);
}

TiValue TiUserWindow::open_cb(size_t num_args, TiValue args[]) {
    this->Open();
    return this->Undefined();
}

TiValue TiUserWindow::close_cb(size_t num_args, TiValue args[]) {
    this->Close();
    return this->Undefined();
}

TiValue TiUserWindow::get_x_cb(size_t num_args, TiValue args[]) {
    double ret = this->GetX();
    return this->NewValue(ret);
}

TiValue TiUserWindow::set_x_cb(size_t num_args, TiValue args[]) {
    if (num_args > 0) {
        double x = args[0].GetNumber();
        this->SetX(x);
    }
    return this->Undefined();
}

TiValue TiUserWindow::get_y_cb(size_t num_args, TiValue args[]) {
    double ret = this->GetY();
    return this->NewValue(ret);
}

TiValue TiUserWindow::set_y_cb(size_t num_args, TiValue args[]) {
    if (num_args > 0) {
        double y = args[0].GetNumber();
        this->SetY(y);
    }
    return this->Undefined();
}

TiValue TiUserWindow::get_width_cb(size_t num_args, TiValue args[]) {
    double ret = this->GetWidth();
    return this->NewValue(ret);
}

TiValue TiUserWindow::set_width_cb(size_t num_args, TiValue args[]) {
    if (num_args > 0) {
        double w = args[0].GetNumber();
        this->SetWidth(w);
    }
    return this->Undefined();
}

TiValue TiUserWindow::get_height_cb(size_t num_args, TiValue args[]) {
    double ret = this->GetHeight();
    return this->NewValue(ret);
}

TiValue TiUserWindow::set_height_cb(size_t num_args, TiValue args[]) {
    if (num_args > 0) {
        double h = args[0].GetNumber();
        this->SetHeight(h);
    }
    return this->Undefined();
}

TiValue TiUserWindow::get_bounds_cb(size_t num_args, TiValue args[]) {
    TiBounds bounds = this->GetBounds();
    TiObject b = this->NewObject();
    b.SetProperty("x", this->NewValue(bounds.x));
    b.SetProperty("y", this->NewValue(bounds.y));
    b.SetProperty("width", this->NewValue(bounds.width));
    b.SetProperty("height", this->NewValue(bounds.height));

    return b;
}

TiValue TiUserWindow::set_bounds_cb(size_t num_args, TiValue args[]) {

    if (num_args > 0) {
        TiBounds bounds;
        TiObject b = args[0].GetObject();
        bounds.x = b.GetProperty("x").GetNumber();
        bounds.y = b.GetProperty("y").GetNumber();
        bounds.width = b.GetProperty("width").GetNumber();
        bounds.height = b.GetProperty("height").GetNumber();
        this->SetBounds(bounds);
    }
    return this->Undefined();
}

TiValue TiUserWindow::get_title_cb(size_t num_args, TiValue args[]) {
    std::string ret = this->GetTitle();
    return this->NewValue(ret);
}

TiValue TiUserWindow::set_title_cb(size_t num_args, TiValue args[]) {
    if (num_args > 0) {
        std::string title = args[0].GetString();
        this->SetTitle(title);
    }
    return this->Undefined();
}

TiValue TiUserWindow::get_url_cb(size_t num_args, TiValue args[]) {
    std::string ret = this->GetUrl();
    return this->NewValue(ret);
}

TiValue TiUserWindow::set_url_cb(size_t num_args, TiValue args[]) {
    if (num_args > 0) {
        std::string url = args[0].GetString();
        this->SetUrl(url);
    }
    return this->Undefined();
}

TiValue TiUserWindow::is_resizable_cb(size_t num_args, TiValue args[]) {
    bool answer = this->IsResizable();
    return this->NewValue(answer);
}

TiValue TiUserWindow::set_resizable_cb(size_t num_args, TiValue args[]) {
    if (num_args > 0) {
        bool value = args[0].GetBool();
        this->SetResizable(value);
    }
    return this->Undefined();
}

TiValue TiUserWindow::is_maximizable_cb(size_t num_args, TiValue args[]) {
    bool answer = this->IsMaximizable();
    return this->NewValue(answer);
}

TiValue TiUserWindow::set_maximizable_cb(size_t num_args, TiValue args[]) {
    if (num_args > 0) {
        bool value = args[0].GetBool();
        this->SetMaximizable(value);
    }
    return this->Undefined();
}

TiValue TiUserWindow::is_minimizable_cb(size_t num_args, TiValue args[]) {
    bool answer = this->IsMinimizable();
    return this->NewValue(answer);
}

TiValue TiUserWindow::set_minimizable_cb(size_t num_args, TiValue args[]) {
    if (num_args > 0) {
        bool value = args[0].GetBool();
        this->SetMinimizable(value);
    }
    return this->Undefined();
}

TiValue TiUserWindow::is_closeable_cb(size_t num_args, TiValue args[]) {
    bool answer = this->IsCloseable();
    return this->NewValue(answer);
}

TiValue TiUserWindow::set_closeable_cb(size_t num_args, TiValue args[]) {
    if (num_args > 0) {
        bool value = args[0].GetBool();
        this->SetCloseable(value);
    }
    return this->Undefined();
}

TiValue TiUserWindow::is_visible_cb(size_t num_args, TiValue args[]) {
    bool answer = this->IsVisible();
    return this->NewValue(answer);
}

TiValue TiUserWindow::set_visible_cb(size_t num_args, TiValue args[]) {
    if (num_args > 0) {
        bool value = args[0].GetBool();
        this->SetVisible(value);
    }
    return this->Undefined();
}

TiValue TiUserWindow::get_transparency_cb(size_t num_args, TiValue args[]) {
    double answer = this->GetTransparency();
    return this->NewValue(answer);
}

TiValue TiUserWindow::set_transparency_cb(size_t num_args, TiValue args[]) {
    if (num_args > 0) {
        double value = args[0].GetNumber();
        this->SetTransparency(value);
    }
    return this->Undefined();
}


void TiUserWindow::Show() { STUB; }
void TiUserWindow::Hide() { STUB; }
bool TiUserWindow::IsUsingChrome() { STUB; }
bool TiUserWindow::IsUsingScrollbars() { STUB; }
bool TiUserWindow::IsFullScreen() { STUB; }
std::string TiUserWindow::GetId() { STUB; }
void TiUserWindow::Open() { STUB; }
void TiUserWindow::Close() { STUB; }
double TiUserWindow::GetX() { STUB; }
void TiUserWindow::SetX(double x) { STUB; }
double TiUserWindow::GetY() { STUB; }
void TiUserWindow::SetY(double y) { STUB; }
double TiUserWindow::GetWidth() { STUB; }
void TiUserWindow::SetWidth(double width) { STUB; }
double TiUserWindow::GetHeight() { STUB; }
void TiUserWindow::SetHeight(double height) { STUB; }
TiBounds TiUserWindow::GetBounds() { STUB; }
void TiUserWindow::SetBounds(TiBounds bounds) { STUB; }
std::string TiUserWindow::GetTitle() { STUB; }
void TiUserWindow::SetTitle(std::string title) { STUB; }
std::string TiUserWindow::GetUrl() { STUB; }
void TiUserWindow::SetUrl(std::string url) { STUB; }
bool TiUserWindow::IsResizable() { STUB; }
void TiUserWindow::SetResizable(bool resizable) { STUB; }
bool TiUserWindow::IsMaximizable() { STUB; }
void TiUserWindow::SetMaximizable(bool maximizable) { STUB; }
bool TiUserWindow::IsMinimizable() { STUB; }
void TiUserWindow::SetMinimizable(bool minimizable) { STUB; }
bool TiUserWindow::IsCloseable() { STUB; }
void TiUserWindow::SetCloseable(bool closeable) { STUB; }
bool TiUserWindow::IsVisible() { STUB; }
void TiUserWindow::SetVisible(bool visible) { STUB; }
double TiUserWindow::GetTransparency() { STUB; }
void TiUserWindow::SetTransparency(double transparency) { STUB; }
