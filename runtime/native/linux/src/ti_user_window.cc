#include "ti_types.h"
#include <stdlib.h>

#define STUB printf("__FILE__:__LINE__: called a stub method!\n")

void TiUserWindow::window_object_cleared(JSContextRef context, JSObjectRef window) {

    JSObjectRef global_object = JSContextGetGlobalObject(context);

    this->context = context;
    this->object = window;
    this->value = window;
    this->global = TiObject(context, global_object);

    TiRuntime ti_runtime = TiRuntime(this);
    this->global.set_property("tiRuntime", ti_runtime);

    /* this object is accessed by tiRuntime.Window.currentWindow */
    this->bind_method("hide", &TiUserWindow::hide_cb);
    this->bind_method("show", &TiUserWindow::show_cb);
    this->bind_method("isUsingChrome", &TiUserWindow::is_using_chrome_cb);
    this->bind_method("isFullScreen", &TiUserWindow::is_full_screen_cb);
    this->bind_method("getId", &TiUserWindow::get_id_cb);
    this->bind_method("open", &TiUserWindow::open_cb);
    this->bind_method("close", &TiUserWindow::close_cb);
    this->bind_method("getX", &TiUserWindow::get_x_cb);
    this->bind_method("setX", &TiUserWindow::set_x_cb);
    this->bind_method("getY", &TiUserWindow::get_y_cb);
    this->bind_method("setY", &TiUserWindow::set_y_cb);
    this->bind_method("getWidth", &TiUserWindow::get_width_cb);
    this->bind_method("setWidth", &TiUserWindow::set_width_cb);
    this->bind_method("getHeight", &TiUserWindow::get_height_cb);
    this->bind_method("setHeight", &TiUserWindow::set_height_cb);
    this->bind_method("getBounds", &TiUserWindow::get_bounds_cb);
    this->bind_method("setBounds", &TiUserWindow::set_bounds_cb);
    this->bind_method("getTitle", &TiUserWindow::get_title_cb);
    this->bind_method("setTitle", &TiUserWindow::set_title_cb);
    this->bind_method("getUrl", &TiUserWindow::get_url_cb);
    this->bind_method("setUrl", &TiUserWindow::set_url_cb);
    this->bind_method("isResizable", &TiUserWindow::is_resizable_cb);
    this->bind_method("setResizable", &TiUserWindow::set_resizable_cb);
    this->bind_method("isMaximizable", &TiUserWindow::is_maximizable_cb);
    this->bind_method("setMaimizable", &TiUserWindow::set_maximizable_cb);
    this->bind_method("isMinimizable", &TiUserWindow::is_minimizable_cb);
    this->bind_method("setMinizable", &TiUserWindow::set_minimizable_cb);
    this->bind_method("isCloseable", &TiUserWindow::is_closeable_cb);
    this->bind_method("setCloseable", &TiUserWindow::set_closeable_cb);
    this->bind_method("isVisible", &TiUserWindow::is_visible_cb);
    this->bind_method("setVisible", &TiUserWindow::set_visible_cb);
    this->bind_method("getTransparency", &TiUserWindow::get_transparency_cb);
    this->bind_method("setTransparency", &TiUserWindow::set_transparency_cb);
}

TiValue TiUserWindow::hide_cb(size_t num_args, TiValue args[]) {
    this->hide();
    return this->undefined();
}

TiValue TiUserWindow::show_cb(size_t num_args, TiValue args[]) {
    this->show();
    return this->undefined();
}

TiValue TiUserWindow::is_using_chrome_cb(size_t num_args, TiValue args[]) {
    bool answer = this->is_using_chrome();
    return this->new_value(answer);
}

TiValue TiUserWindow::is_using_scrollbars_cb(size_t num_args, TiValue args[]) {
    bool answer = this->is_using_scrollbars();
    return this->new_value(answer);
}

TiValue TiUserWindow::is_full_screen_cb(size_t num_args, TiValue args[]) {
    bool answer = this->is_full_screen();
    return this->new_value(answer);
}

TiValue TiUserWindow::get_id_cb(size_t num_args, TiValue args[]) {
    std::string ret = this->get_id();
    return this->new_value(ret);
}

TiValue TiUserWindow::open_cb(size_t num_args, TiValue args[]) {
    this->open();
    return this->undefined();
}

TiValue TiUserWindow::close_cb(size_t num_args, TiValue args[]) {
    this->close();
    return this->undefined();
}

TiValue TiUserWindow::get_x_cb(size_t num_args, TiValue args[]) {
    double ret = this->get_x();
    return this->new_value(ret);
}

TiValue TiUserWindow::set_x_cb(size_t num_args, TiValue args[]) {
    if (num_args > 0) {
        double x = args[0].get_number();
        this->set_x(x);
    }
    return this->undefined();
}

TiValue TiUserWindow::get_y_cb(size_t num_args, TiValue args[]) {
    double ret = this->get_y();
    return this->new_value(ret);
}

TiValue TiUserWindow::set_y_cb(size_t num_args, TiValue args[]) {
    if (num_args > 0) {
        double y = args[0].get_number();
        this->set_y(y);
    }
    return this->undefined();
}

TiValue TiUserWindow::get_width_cb(size_t num_args, TiValue args[]) {
    double ret = this->get_width();
    return this->new_value(ret);
}

TiValue TiUserWindow::set_width_cb(size_t num_args, TiValue args[]) {
    if (num_args > 0) {
        double w = args[0].get_number();
        this->set_width(w);
    }
    return this->undefined();
}

TiValue TiUserWindow::get_height_cb(size_t num_args, TiValue args[]) {
    double ret = this->get_height();
    return this->new_value(ret);
}

TiValue TiUserWindow::set_height_cb(size_t num_args, TiValue args[]) {
    if (num_args > 0) {
        double h = args[0].get_number();
        this->set_height(h);
    }
    return this->undefined();
}

TiValue TiUserWindow::get_bounds_cb(size_t num_args, TiValue args[]) {
    TiBounds bounds = this->get_bounds();
    TiObject b = this->new_object();
    b.set_property("x", this->new_value(bounds.x));
    b.set_property("y", this->new_value(bounds.y));
    b.set_property("width", this->new_value(bounds.width));
    b.set_property("height", this->new_value(bounds.height));

    return b;
}

TiValue TiUserWindow::set_bounds_cb(size_t num_args, TiValue args[]) {

    if (num_args > 0) {
        TiBounds bounds;
        TiObject b = args[0].get_object();
        bounds.x = b.get_property("x").get_number();
        bounds.y = b.get_property("y").get_number();
        bounds.width = b.get_property("width").get_number();
        bounds.height = b.get_property("height").get_number();
        this->set_bounds(bounds);
    }
    return this->undefined();
}

TiValue TiUserWindow::get_title_cb(size_t num_args, TiValue args[]) {
    std::string ret = this->get_title();
    return this->new_value(ret);
}

TiValue TiUserWindow::set_title_cb(size_t num_args, TiValue args[]) {
    if (num_args > 0) {
        std::string title = args[0].get_string();
        this->set_title(title);
    }
    return this->undefined();
}

TiValue TiUserWindow::get_url_cb(size_t num_args, TiValue args[]) {
    std::string ret = this->get_url();
    return this->new_value(ret);
}

TiValue TiUserWindow::set_url_cb(size_t num_args, TiValue args[]) {
    if (num_args > 0) {
        std::string url = args[0].get_string();
        this->set_url(url);
    }
    return this->undefined();
}

TiValue TiUserWindow::is_resizable_cb(size_t num_args, TiValue args[]) {
    bool answer = this->is_resizable();
    return this->new_value(answer);
}

TiValue TiUserWindow::set_resizable_cb(size_t num_args, TiValue args[]) {
    if (num_args > 0) {
        bool value = args[0].get_bool();
        this->set_resizable(value);
    }
    return this->undefined();
}

TiValue TiUserWindow::is_maximizable_cb(size_t num_args, TiValue args[]) {
    bool answer = this->is_maximizable();
    return this->new_value(answer);
}

TiValue TiUserWindow::set_maximizable_cb(size_t num_args, TiValue args[]) {
    if (num_args > 0) {
        bool value = args[0].get_bool();
        this->set_maximizable(value);
    }
    return this->undefined();
}

TiValue TiUserWindow::is_minimizable_cb(size_t num_args, TiValue args[]) {
    bool answer = this->is_minimizable();
    return this->new_value(answer);
}

TiValue TiUserWindow::set_minimizable_cb(size_t num_args, TiValue args[]) {
    if (num_args > 0) {
        bool value = args[0].get_bool();
        this->set_minimizable(value);
    }
    return this->undefined();
}

TiValue TiUserWindow::is_closeable_cb(size_t num_args, TiValue args[]) {
    bool answer = this->is_closeable();
    return this->new_value(answer);
}

TiValue TiUserWindow::set_closeable_cb(size_t num_args, TiValue args[]) {
    if (num_args > 0) {
        bool value = args[0].get_bool();
        this->set_closeable(value);
    }
    return this->undefined();
}

TiValue TiUserWindow::is_visible_cb(size_t num_args, TiValue args[]) {
    bool answer = this->is_visible();
    return this->new_value(answer);
}

TiValue TiUserWindow::set_visible_cb(size_t num_args, TiValue args[]) {
    if (num_args > 0) {
        bool value = args[0].get_bool();
        this->set_visible(value);
    }
    return this->undefined();
}

TiValue TiUserWindow::get_transparency_cb(size_t num_args, TiValue args[]) {
    double answer = this->get_transparency();
    return this->new_value(answer);
}

TiValue TiUserWindow::set_transparency_cb(size_t num_args, TiValue args[]) {
    if (num_args > 0) {
        double value = args[0].get_number();
        this->set_transparency(value);
    }
    return this->undefined();
}


void TiUserWindow::show() { STUB; }
void TiUserWindow::hide() { STUB; }
bool TiUserWindow::is_using_chrome() { STUB; }
bool TiUserWindow::is_using_scrollbars() { STUB; }
bool TiUserWindow::is_full_screen() { STUB; }
std::string TiUserWindow::get_id() { STUB; }
void TiUserWindow::open() { STUB; }
void TiUserWindow::close() { STUB; }
double TiUserWindow::get_x() { STUB; }
void TiUserWindow::set_x(double x) { STUB; }
double TiUserWindow::get_y() { STUB; }
void TiUserWindow::set_y(double y) { STUB; }
double TiUserWindow::get_width() { STUB; }
void TiUserWindow::set_width(double width) { STUB; }
double TiUserWindow::get_height() { STUB; }
void TiUserWindow::set_height(double height) { STUB; }
TiBounds TiUserWindow::get_bounds() { STUB; }
void TiUserWindow::set_bounds(TiBounds bounds) { STUB; }
std::string TiUserWindow::get_title() { STUB; }
void TiUserWindow::set_title(std::string title) { STUB; }
std::string TiUserWindow::get_url() { STUB; }
void TiUserWindow::set_url(std::string url) { STUB; }
bool TiUserWindow::is_resizable() { STUB; }
void TiUserWindow::set_resizable(bool resizable) { STUB; }
bool TiUserWindow::is_maximizable() { STUB; }
void TiUserWindow::set_maximizable(bool maximizable) { STUB; }
bool TiUserWindow::is_minimizable() { STUB; }
void TiUserWindow::set_minimizable(bool minimizable) { STUB; }
bool TiUserWindow::is_closeable() { STUB; }
void TiUserWindow::set_closeable(bool closeable) { STUB; }
bool TiUserWindow::is_visible() { STUB; }
void TiUserWindow::set_visible(bool visible) { STUB; }
double TiUserWindow::get_transparency() { STUB; }
void TiUserWindow::set_transparency(double transparency) { STUB; }
