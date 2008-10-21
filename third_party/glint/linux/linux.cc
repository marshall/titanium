// Copyright 2008, Google Inc.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//  3. Neither the name of Google Inc. nor the names of its contributors may be
//     used to endorse or promote products derived from this software without
//     specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
// EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Implementation of Platform interface and PlatformWindow for Linux
#include "glint/linux/linux.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <map>
#include <queue>
#include <set>
#include <string>
#include <vector>

#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include "glint/crossplatform/core_util.h"
#include "glint/include/bitmap.h"
#include "glint/include/color.h"
#include "glint/include/message.h"
#include "glint/include/point.h"
#include "glint/include/rectangle.h"
#include "glint/include/root_ui.h"
#include "glint/include/simple_text.h"
#include "glint/posix/posix.h"

//#define UNIMPLEMENTED() CrashWithMessage("%s: unimplemented\n", __FUNCTION__)
#define UNIMPLEMENTED()

using namespace glint;

namespace glint_linux {

// An arbitrary cut-off for how to convert to 1-bit transparency
// based on how the shadow looked.
const int kAlphaCutoffForOneBitTransparency = 70;

typedef std::map<std::string, std::vector<char> > LinuxResourceMap;

class LinuxWindow;

template<class T>
LinuxWindow* GetLinuxWindow(T* p) {
  return reinterpret_cast<LinuxWindow*>(p);
}

template<class T>
PangoAttrList* GetAttrList(T* p) {
  return reinterpret_cast<PangoAttrList*>(p);
}

void ScopedGObjectFree::operator()(void* p) const {
  if (p)
    g_object_unref(G_OBJECT(p));
}

void ScopedPangoAttrListFree::operator()(void* p) const {
  if (p)
    pango_attr_list_unref(reinterpret_cast<PangoAttrList*>(p));
}

void ScopedPangoLayoutIterFree::operator()(void* p) const {
  if (p)
    pango_layout_iter_free(reinterpret_cast<PangoLayoutIter*>(p));
}

void cairo_fill(GdkDrawable* drawable,
                double r,
                double g,
                double b,
                double a,
                int x,
                int y,
                int width,
                int height) {
  cairo_t *cr = gdk_cairo_create(drawable);
  cairo_set_source_rgba(cr, r, g, b, a);
  cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
  cairo_new_path(cr);
  cairo_rectangle(cr, x, y, width, height);
  cairo_clip(cr);
  cairo_paint(cr);
  cairo_destroy(cr);
}

GdkColormap* GetColorMap(GdkScreen* screen) {
  GdkColormap* colormap = gdk_screen_get_rgba_colormap(screen);
  if (colormap) {
    return colormap;
  }
  return gdk_screen_get_rgb_colormap(screen);
}

bool draw_text_layout(PangoLayout* pango_layout,
                      Color foreground,
                      int width,
                      int height,
                      Bitmap* target) {
  ASSERT(pango_layout != NULL);
  GdkScreen* screen = gdk_screen_get_default();
  GdkColormap* colormap = GetColorMap(screen);
  if (!colormap)
    return false;
  ScopedPixmap pixmap(gdk_pixmap_new(NULL, width, height,
                                     gdk_colormap_get_visual(colormap)->depth));
  gdk_drawable_set_colormap(pixmap.get(), colormap);
  if (gdk_drawable_get_colormap(pixmap.get()) != colormap)
    return false;
  cairo_fill(pixmap.get(), 0.0, 0.0, 0.0, 1.0, 0, 0, width, height);
  ScopedGC gc(gdk_gc_new(pixmap.get()));

  GdkColor white = {0xffffffff, 65535, 65535, 65535};
  if (!gdk_colormap_alloc_color(colormap, &white, FALSE, FALSE))
    return false;
  gdk_draw_layout_with_colors(pixmap.get(),
                              gc.get(),
                              0,
                              0,
                              pango_layout,
                              &white,
                              NULL);
  GdkPixbuf* pixbuf = GDK_PIXBUF(target->platform_bitmap());
  if (pixbuf == NULL)
    return false;
  ASSERT(gdk_pixbuf_get_width(pixbuf) == width &&
      gdk_pixbuf_get_height(pixbuf) == height);
  if (gdk_pixbuf_get_from_drawable(pixbuf,
                                   pixmap.get(),
                                   NULL,
                                   0,
                                   0,
                                   0,
                                   0,
                                   width,
                                   height) == NULL) {
    return false;
  }

  // pixel conversion
  int row_stride = gdk_pixbuf_get_rowstride(pixbuf);
  guchar* line = gdk_pixbuf_get_pixels(pixbuf);
  if (!line)
    return false;
  for (int i = 0; i < height; ++i) {
    guchar* data = line;
    for (int j = 0; j < width; ++j) {
      guchar alpha = data[1];
      data[0] = static_cast<int>(foreground.blue()) * alpha / 255;
      data[1] = static_cast<int>(foreground.green()) * alpha / 255;
      data[2] = static_cast<int>(foreground.red()) * alpha / 255;
      data[3] = alpha;
      data += 4;
    }
    line += row_stride;
  }
  return true;
}

class LinuxWindow {
 public:
  static LinuxWindow* Create(RootUI* ui) {
    ASSERT(ui != NULL);

    scoped_ptr<LinuxWindow> linux_window(new LinuxWindow(ui));
    if (!linux_window->Initialize()) {
      return NULL;
    }
    return linux_window.release();
  }

  ~LinuxWindow() {
    Destroy();
  }

  bool Show() {
    if (!window_)
      return false;
    gtk_widget_show(window_);
    return true;
  }

  bool Hide() {
    if (!window_)
      return false;
    gtk_widget_hide(window_);
    return true;
  }

  void Present() { gtk_window_present(GTK_WINDOW(window_)); }
  GtkWindow* GetWindow() { return GTK_WINDOW(window_); }
  GdkPixbuf* GetPixbuf() { return EnsurePixbuf(); }

  void Destroy() {
    if (window_)
      gtk_widget_destroy(window_);
  }

  bool SetTitle(const std::string& text) {
    if (!window_)
      return false;
    gtk_window_set_title(GTK_WINDOW(window_), text.c_str());
    return true;
  }

  bool SetUrgent(bool urgent) {
    if (!window_)
      return false;
    gtk_window_set_urgency_hint(GTK_WINDOW(window_), urgent);
    return true;
  }

  bool GetSize(int* width, int* height) {
    if (!window_)
      return false;
    *width = width_;
    *height = height_;
    return true;
  }

  void Resize(int width, int height) {
    if (!window_)
      return;
    width_ = width;
    height_ = height;
    gtk_window_resize(GTK_WINDOW(window_), width_, height_);
  }

  void Move(int x, int y) {
    if (!window_)
      return;

    gtk_window_move(GTK_WINDOW(window_), x, y);
  }

  bool Minimize() {
    if (!window_)
      return false;
    gtk_window_iconify(GTK_WINDOW(window_));
    return true;
  }

  void CopyPixbuf(int dest_x, int dest_y,
                  GdkPixbuf* src, GdkRectangle* src_rect) {
    if (!window_)
      return;
    gdk_pixbuf_copy_area(src, src_rect->x, src_rect->y,
                         src_rect->width, src_rect->height,
                         EnsurePixbuf(), dest_x, dest_y);

    // Set-up transparency (1 bit alpha) when compositing isn't supported..
    if (!is_compositing_supported()) {
      ScopedPixmap mask(
          gdk_pixmap_new(NULL, src_rect->width, src_rect->height, 1));
      gdk_pixbuf_render_threshold_alpha(
          src, mask.get(), src_rect->x, src_rect->y,
          0, 0, src_rect->width, src_rect->height,
          kAlphaCutoffForOneBitTransparency);
      gdk_window_shape_combine_mask(window_->window, mask.get(),
                                    src_rect->x, src_rect->y);
    }
  }

  void Invalidate(GdkRectangle* rect) {
    if (!window_)
      return;
    gdk_window_invalidate_rect(window_->window, rect, false);
  }

  // In GDK, pointer events are automatically tracked after a button is pressed
  // even after the pointer leaves the window, until the button is released,
  // so it's usually really necessary to do anything here, but if the user wants
  // to call this in another situation, then this can be useful.
  bool StartMouseCapture() {
    if (!window_)
      return false;

    static const int kMouseEvents =(GDK_POINTER_MOTION_MASK |
                                    GDK_BUTTON_MOTION_MASK |
                                    GDK_BUTTON_PRESS_MASK |
                                    GDK_BUTTON_RELEASE_MASK);
    return gdk_pointer_grab(window_->window, TRUE,
                            static_cast<GdkEventMask>(kMouseEvents), NULL,
                            NULL, GDK_CURRENT_TIME) == GDK_GRAB_SUCCESS;
  }

  void ScreenChanged() {
    GdkScreen* screen = gtk_widget_get_screen(window_);
    GdkColormap* colormap = GetColorMap(screen);
    if (!colormap)
      Destroy();
    else
      gtk_widget_set_colormap(window_, colormap);
  }

  gboolean HandleGenericMouseEvent(GdkEvent* event) {
    Message message;
    switch (event->type) {
      case GDK_MOTION_NOTIFY:
        message.code = GL_MSG_MOUSEMOVE;
        break;
      case GDK_BUTTON_PRESS:
      case GDK_BUTTON_RELEASE:
      {
        guint button = reinterpret_cast<GdkEventButton*>(event)->button;
        switch (button) {
          case 1:
            message.code = event->type == GDK_BUTTON_PRESS ?
                GL_MSG_LBUTTONDOWN : GL_MSG_LBUTTONUP;
            break;
          case 2:
            message.code = event->type == GDK_BUTTON_PRESS ?
                GL_MSG_MBUTTONDOWN : GL_MSG_MBUTTONUP;
            break;
          case 3:
            message.code = event->type == GDK_BUTTON_PRESS ?
                GL_MSG_RBUTTONDOWN : GL_MSG_RBUTTONUP;
            break;
          default:  // not handled
            return FALSE;
        }
        break;
      }
      default:
        return FALSE;
    }
    message.ui = ui_;
    message.platform_window = this;
    gdouble x = 0.0, y = 0.0;
    gdk_event_get_root_coords(event, &x, &y);
    message.screen_mouse_position = message.mouse_position = Vector(
        static_cast<glint::real32>(x), static_cast<glint::real32>(y));
    return ui_->HandleMessage(message) == MESSAGE_HANDLED;
  }

  bool is_compositing_supported() const {
    if (!window_)
      return false;
#if GTK_CHECK_VERSION(2, 10, 0)
    return gtk_widget_is_composited(window_);
#else
    GdkScreen* screen = gtk_widget_get_screen(window_);
    return NULL != gdk_screen_get_rgba_colormap(screen);
#endif
  }

  RootUI* ui() {
    return ui_;
  }

 private:
  LinuxWindow(RootUI* ui)
    : pixbuf_(NULL),
      ui_(ui),
      window_(gtk_window_new(GTK_WINDOW_POPUP)),
      width_(kDefaultWidth),
      height_(kDefaultHeight),
      idle_timer_id_(0) {
    ASSERT(ui != NULL);
  }

  bool Initialize() {
    if (!window_) {
      return false;
    }

    gtk_widget_set_app_paintable(window_, TRUE);
    gtk_window_set_default_size(GTK_WINDOW(window_),
                                width_, height_);
    gtk_window_set_decorated(GTK_WINDOW(window_), FALSE);
    gtk_window_set_type_hint(GTK_WINDOW(window_),
#if GTK_CHECK_VERSION(2, 10, 0)
                             GDK_WINDOW_TYPE_HINT_NOTIFICATION
#else
                             GDK_WINDOW_TYPE_HINT_DIALOG
#endif
                             );
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(window_), TRUE);
    gtk_window_set_skip_pager_hint(GTK_WINDOW(window_), TRUE);
    g_signal_connect(window_, "screen-changed",
                     G_CALLBACK(ScreenChangedHandler), this);
    g_signal_connect(window_, "expose-event", G_CALLBACK(ExposeHandler), this);
    g_signal_connect(window_, "destroy", G_CALLBACK(DestroyHandler), this);
    g_signal_connect(window_, "motion-notify-event",
                     G_CALLBACK(GenericMouseHandler), this);
    g_signal_connect(window_, "button-press-event",
                     G_CALLBACK(GenericMouseHandler), this);
    g_signal_connect(window_, "button-release-event",
                     G_CALLBACK(GenericMouseHandler), this);
    gtk_widget_add_events(window_,
                          GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK |
                          GDK_BUTTON_RELEASE_MASK);
    idle_timer_id_ = g_timeout_add(10, SendIdleMessageHandler, this);
    if (idle_timer_id_ == 0) {
      return false;
    }
    ScreenChanged();
    return true;
  }

  static const int kDefaultWidth = 100, kDefaultHeight = 100;
  static const int kDefaultColor = 0x0080FF80;

  static void ScreenChangedHandler(GtkWidget* widget, GdkScreen* old_screen,
                                   gpointer user_data) {
    LinuxWindow* linux_window = GetLinuxWindow(user_data);
    ASSERT(linux_window->window_ == widget);
    linux_window->ScreenChanged();
  }

  static gboolean ExposeHandler(GtkWidget* widget,
                                GdkEventExpose* event,
                                gpointer user_data) {
    LinuxWindow* linux_window = GetLinuxWindow(user_data);
    ASSERT(linux_window->window_ == widget);
    linux_window->PaintRect(&event->area);
    return FALSE;
  }

  static void DestroyHandler(GtkWidget* widget, gpointer user_data) {
    LinuxWindow* linux_window = GetLinuxWindow(user_data);
    ASSERT(linux_window->window_ == widget);
    linux_window->Cleanup();
  }

  static gboolean GenericMouseHandler(GtkWidget* widget,
                                      GdkEvent* event,
                                      gpointer user_data) {
    LinuxWindow* linux_window = GetLinuxWindow(user_data);
    ASSERT(linux_window->window_ == widget);
    return linux_window->HandleGenericMouseEvent(event);
  }

  static gboolean SendIdleMessageHandler(gpointer user_data) {
    LinuxWindow* linux_window = GetLinuxWindow(user_data);
    GetLinuxWindow(user_data);
    Message message;
    message.ui = linux_window->ui_;
    message.platform_window = linux_window;
    message.code = GL_MSG_IDLE;
    linux_window->ui_->HandleMessage(message);
    return true;
  }

  void Cleanup() {
    ui_ = NULL;
    window_ = NULL;
    if (pixbuf_) {
      g_object_unref(pixbuf_);
      pixbuf_ = NULL;
    }
    if (idle_timer_id_) {
      g_source_remove(idle_timer_id_);
      idle_timer_id_ = 0;
    }
  }

  void PaintRect(GdkRectangle* rect) {
    if (!window_)
      return;
    gint transfer_width, transfer_height;
    gdk_drawable_get_size(window_->window, &transfer_width, &transfer_height);
    transfer_width -= rect->x;
    transfer_height -= rect->y;
    if (transfer_width <= 0 || transfer_height <= 0)
      return;
    if (transfer_width > rect->width)
        transfer_width = rect->width;
    if (transfer_height > rect->height)
        transfer_height = rect->height;
    EnsurePixbuf();
    cairo_fill(window_->window, 0.0, 0.0, 0.0, 0.0,
               rect->x, rect->y, transfer_width, transfer_height);
    gdk_draw_pixbuf(window_->window, NULL, pixbuf_, rect->x, rect->y,
                    rect->x, rect->y, transfer_width, transfer_height,
                    GDK_RGB_DITHER_NONE, 0, 0);
  }

  GdkPixbuf* EnsurePixbuf() {
    if (!pixbuf_) {
      pixbuf_ = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, width_, height_);
      gdk_pixbuf_fill(pixbuf_, kDefaultColor);
    } else {
      gint old_width = gdk_pixbuf_get_width(pixbuf_);
      gint old_height = gdk_pixbuf_get_height(pixbuf_);

      if (width_ != old_width || height_ != old_height) {
        GdkPixbuf* new_pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8,
                                               width_, height_);

        if (old_width < width_ || old_height < height_)
          gdk_pixbuf_fill(new_pixbuf, kDefaultColor);
        gdk_pixbuf_copy_area(pixbuf_, 0, 0, old_width, old_height,
                             new_pixbuf, 0, 0);
        g_object_unref(pixbuf_);
        pixbuf_ = new_pixbuf;
      }
    }
    return pixbuf_;
  }

  GdkPixbuf* pixbuf_;
  RootUI* ui_;
  GtkWidget* window_;
  gint width_, height_;
  gint idle_timer_id_;
  DISALLOW_EVIL_CONSTRUCTORS(LinuxWindow);
};

void Close(LinuxWindow* window) {
  window->Destroy();
}

class LinuxPlatform : public glint_posix::PosixPlatform {
 public:
  LinuxPlatform() {
    Display* display = GDK_DISPLAY();
    if (display != NULL) {
      workarea_atom_ = XInternAtom(display, "_NET_WORKAREA", false);
    }
    GdkWindow* window = gdk_screen_get_root_window(gdk_screen_get_default());
    gdk_window_set_events(window, static_cast<GdkEventMask>(
                              gdk_window_get_events(window) |
                              GDK_PROPERTY_CHANGE_MASK));
    gdk_window_add_filter(window, WindowEventFilter, this);
  }

  ~LinuxPlatform() {
    GdkWindow* window = gdk_screen_get_root_window(gdk_screen_get_default());
    gdk_window_remove_filter(window, WindowEventFilter, this);
  }

  virtual bool is_compositing_supported(PlatformWindow* window) const {
    ASSERT(window);
    LinuxWindow* linux_window = GetLinuxWindow(window);
    return linux_window->is_compositing_supported();
  }

  virtual PlatformBitmap* CreateBitmap(int width, int height) {
    GdkPixbuf* pixbuf(gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8,
                                     width, height));
    ASSERT(gdk_pixbuf_get_n_channels(pixbuf) == 4);
    return reinterpret_cast<PlatformBitmap*>(pixbuf);
  }

  virtual Color* LockPixels(PlatformBitmap* bitmap) {
    GdkPixbuf* gdk_pixbuf = GDK_PIXBUF(bitmap);
    ASSERT(gdk_pixbuf);
    ASSERT(gdk_pixbuf_get_rowstride(gdk_pixbuf) ==
           gdk_pixbuf_get_width(gdk_pixbuf) * static_cast<int>(sizeof(Color)));
    return reinterpret_cast<Color*>(gdk_pixbuf_get_pixels(gdk_pixbuf));
  }

  virtual void UnlockPixels(PlatformBitmap* bitmap, Color* pixels) {
    // nothing to do
  }

  virtual void DeleteBitmap(PlatformBitmap* bitmap) {
    // TODO(levin): be more tolerant of invalid pointers
    ASSERT(bitmap);
    g_object_unref(G_OBJECT(bitmap));
  }

  virtual PlatformWindow* CreateInvisibleWindow(RootUI* ui,
                                                bool topmost,
                                                bool taskbar) {
    if (!ui)
      return NULL;
    LinuxWindow* linux_window = LinuxWindow::Create(ui);
    if (!linux_window) {
      return NULL;
    }
    all_windows_.insert(ui);
    return reinterpret_cast<PlatformWindow*>(linux_window);
  }

  virtual void DeleteInvisibleWindow(PlatformWindow* window) {
    LinuxWindow* linux_window = GetLinuxWindow(window);
    all_windows_.erase(linux_window->ui());
    return linux_window->Destroy();
  }

  virtual bool UpdateInvisibleWindow(PlatformWindow* window,
                                     Point* screen_origin,
                                     Size screen_size,
                                     PlatformBitmap* bitmap,
                                     Point* bitmap_window_offset,
                                     Rectangle* bitmap_area) {
    LinuxWindow* linux_window = GetLinuxWindow(window);
    GdkPixbuf* src_pixbuf = GDK_PIXBUF(bitmap);

    linux_window->Resize(screen_size.width, screen_size.height);
    if (screen_origin)
      linux_window->Move(screen_origin->x, screen_origin->y);

    if (!src_pixbuf)
      return true;

    int dest_width = 0, dest_height = 0;
    linux_window->GetSize(&dest_width, &dest_height);

    GdkRectangle src_rect;
    Point dest_point;

    if (bitmap_window_offset) {
      dest_point.x = bitmap_window_offset->x;
      dest_point.y = bitmap_window_offset->y;
      if (dest_point.x >= dest_width || dest_point.y >= dest_height)
        return true;
    } else {
      dest_point.x = dest_point.y = 0;
    }

    if (bitmap_area) {
      src_rect.x = bitmap_area->left();
      src_rect.y = bitmap_area->top();
      src_rect.width = bitmap_area->right() - bitmap_area->left();
      src_rect.height = bitmap_area->bottom() - bitmap_area->top();
    } else {
      // clip rectangle to maximum sensible size
      src_rect.x = src_rect.y = 0;
      src_rect.width = gdk_pixbuf_get_width(src_pixbuf);
      src_rect.height = gdk_pixbuf_get_height(src_pixbuf);
    }
    src_rect.width = std::min(src_rect.width, dest_width - dest_point.x);
    src_rect.height = std::min(src_rect.height, dest_height - dest_point.y);
    linux_window->CopyPixbuf(dest_point.x, dest_point.y, src_pixbuf, &src_rect);
    src_rect.x = dest_point.x;
    src_rect.y = dest_point.y;
    linux_window->Invalidate(&src_rect);
    return true;
  }

  virtual bool MinimizeWindow(PlatformWindow* window) {
    return GetLinuxWindow(window)->Minimize();
  }

  virtual bool HideWindow(PlatformWindow* window) {
    return GetLinuxWindow(window)->Hide();
  }

  virtual bool ShowWindow(PlatformWindow* window) {
    return GetLinuxWindow(window)->Show();
  }

  virtual bool ShowInteractiveWindow(PlatformWindow* window) {
    // TODO(levin): what's the difference between interactive and non-?
    return GetLinuxWindow(window)->Show();
  }

  virtual void* GetWindowNativeHandle(PlatformWindow* window) {
    return GetLinuxWindow(window);
  }

  virtual bool PostWorkItem(RootUI* ui, WorkItem* work_item) {
    // PostWorkItem is safe to call only from the same thread as the one
    // consuming the work items, namely the one running the event loop.
    Message message;
    message.code = GL_MSG_WORK_ITEM;
    message.ui = ui;
    message.work_item = work_item;
    work_items_.push(message);
    if (work_items_.size() == 1)
      g_idle_add_full(G_PRIORITY_HIGH, HighIdleHandler, this, NULL);
    return true;
  }

  virtual bool StartMouseCapture(RootUI* ui) {
    return GetLinuxWindow(ui->GetPlatformWindow())->StartMouseCapture();
  }

  virtual void EndMouseCapture() {
    gdk_pointer_ungrab(GDK_CURRENT_TIME);
  }

  virtual bool SetCursor(Cursors cursor_type) {
    UNIMPLEMENTED();
    return false;
  }

  virtual bool SetIcon(PlatformWindow* window, const char* icon_name) {
    UNIMPLEMENTED();
    return false;
  }

  virtual bool SetFocus(PlatformWindow* window) {
    // TODO(levin): what's the difference between focus and foreground?
    GetLinuxWindow(window)->Present();
    return true;
  }

  virtual void RunMessageLoop() {
    gtk_main();
  }

  virtual bool SetWindowCaptionText(PlatformWindow* window,
                                    const std::string& text) {
    return GetLinuxWindow(window)->SetTitle(text);
  }

  virtual bool BringWindowToForeground(PlatformWindow* window) {
    // TODO(levin): what's the difference between focus and foreground?
    GetLinuxWindow(window)->Present();
    return true;
  }

  virtual bool StartWindowFlash(PlatformWindow* window) {
    return GetLinuxWindow(window)->SetUrgent(true);
  }

  virtual bool StopWindowFlash(PlatformWindow* window) {
    return GetLinuxWindow(window)->SetUrgent(false);
  }

  virtual bool LoadBitmapFromResource(const std::string& name, Bitmap* result) {
    UNIMPLEMENTED();
    return false;
  }

  virtual bool LoadBitmapFromFile(const std::string& file_name, Bitmap** result) {
    if (!result)
      return false;

    ASSERT(*result == NULL);
    ScopedPixbuf gdk_pixbuf(gdk_pixbuf_new_from_file(file_name.c_str(),
                                                     NULL));
    if (!gdk_pixbuf.get())
      return false;

    int width = gdk_pixbuf_get_width(gdk_pixbuf.get());
    int height = gdk_pixbuf_get_height(gdk_pixbuf.get());
    *result = new Bitmap(Point(), Size(width, height));
    gdk_pixbuf_copy_area(gdk_pixbuf.get(), 0, 0, width, height,
                         GDK_PIXBUF((*result)->platform_bitmap()), 0, 0);
    (*result)->PremultiplyAlpha();
    return true;
  }

  virtual bool GetResourceByName(const char* name, void** data, int* size) {
    LinuxResourceMap::iterator resource = resources_.find(name);
    if (resource == resources_.end()) {
      struct stat file_stats;
      if (stat(name, &file_stats)) {
        return false;
      }
      std::vector<char>  buf(file_stats.st_size);
      int fd = open(name, O_RDONLY);
      if (!fd) {
        return false;
      }
      if (file_stats.st_size != read(fd, &buf[0], buf.size())) {
        return false;
      }

      char* key = new char[strlen(name) + 1];
      strcpy(key, name);
      resources_[key] = buf;
      resource = resources_.find(name);
    }

    *size = resource->second.size();
    *data = &(resource->second[0]);
    return true;
  }

  virtual bool GetDefaultFontDescription(FontDescription* font) {
    *font = FontDescription();
    font->family_name = "Sans";
    font->height = 10;
    font->bold = false;
    font->italic = false;
    font->underline = false;
    font->strike_out = false;
    return true;
  }

  virtual PlatformFont* GetDefaultFont() {
    FontDescription desc;
    if (!GetDefaultFontDescription(&desc))
      return NULL;
    return CreateFontFromDescription(desc);
  }

  virtual PlatformFont* CreateFontFromDescription(const FontDescription &font) {
    ScopedAttrList attr_list(pango_attr_list_new());
    ExtendAndInsertAttribute(attr_list.get(),
                            pango_attr_family_new(font.family_name.c_str()));
    ExtendAndInsertAttribute(attr_list.get(),
                             pango_attr_size_new(font.height * PANGO_SCALE));
    ExtendAndInsertAttribute(
        attr_list.get(),
        pango_attr_weight_new(
            font.bold ? PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL));
    ExtendAndInsertAttribute(
        attr_list.get(),
        pango_attr_style_new(
            font.italic ? PANGO_STYLE_ITALIC : PANGO_STYLE_NORMAL));
    ExtendAndInsertAttribute(attr_list.get(),
                             pango_attr_underline_new(font.underline ?
                                                      PANGO_UNDERLINE_SINGLE :
                                                      PANGO_UNDERLINE_NONE));
    ExtendAndInsertAttribute(attr_list.get(),
                             pango_attr_strikethrough_new(font.strike_out));
    return reinterpret_cast<PlatformFont*>(attr_list.release());
  }

  virtual void ReleaseFont(PlatformFont* platform_font) {
    pango_attr_list_unref(GetAttrList(platform_font));
  }

  void MeasureTextLayout(PangoLayout* pango_layout, Rectangle* bounds) {
    ASSERT(pango_layout != NULL && bounds != NULL);
    PangoRectangle rect;
    pango_layout_get_extents(pango_layout, NULL, &rect);
    bounds->set_left(0);
    bounds->set_top(0);

    // Convert from Pango units to pixel units, rounding up.
    bounds->set_right(
        ((rect.x + rect.width) + (PANGO_SCALE - 1)) / PANGO_SCALE);
    bounds->set_bottom(
        ((rect.y + rect.height) + (PANGO_SCALE - 1)) / PANGO_SCALE);
  }

  virtual bool MeasureSimpleText(PlatformFont* platform_font,
                                 const std::string &text,
                                 int wrapping_width,
                                 bool single_line,
                                 bool use_ellipsis,
                                 Rectangle* bounds) {
    ScopedLayout pango_layout(GetTextLayout(GetAttrList(platform_font),
                                            text,
                                            wrapping_width,
                                            single_line,
                                            use_ellipsis));
    if (pango_layout.get() == NULL)
      return false;
    MeasureTextLayout(pango_layout.get(), bounds);
    return true;
  }

  virtual bool DrawSimpleText(PlatformFont* platform_font,
                              const std::string &text,
                              Bitmap* target,
                              const Rectangle& clip,
                              Color foreground,
                              bool single_line,
                              bool use_ellipsis) {
    int width = clip.size().width;
    int height = clip.size().height;
    ScopedLayout pango_layout(GetTextLayout(GetAttrList(platform_font),
                                            text,
                                            width,
                                            single_line,
                                            use_ellipsis));
    return draw_text_layout(pango_layout.get(),
                            foreground,
                            width,
                            height,
                            target);
  }

 private:
  static gboolean HighIdleHandler(gpointer data) {
    std::queue<Message>& work_items =
        reinterpret_cast<LinuxPlatform*>(data)->work_items_;
    while (!work_items.empty()) {
      Message& work_item_msg = work_items.front();

      if (!work_item_msg.ui) {
        if (work_item_msg.work_item) {
          work_item_msg.work_item->Run();
          delete work_item_msg.work_item;
        }
      } else {
        work_item_msg.ui->HandleMessage(work_item_msg);
      }
      work_items.pop();
    }
    return FALSE;
  }

  static void ExtendAndInsertAttribute(PangoAttrList* list,
                                       PangoAttribute* attr) {
    attr->start_index = 0;
    attr->end_index = G_MAXUINT;
    pango_attr_list_insert(list, attr);
  }

  PangoLayout* GetTextLayout(PangoAttrList* attr_list,
                             const std::string &text,
                             int wrapping_width,
                             bool single_line,
                             bool use_ellipsis) {
    ScopedContext pango_context(gdk_pango_context_get());
    ScopedLayout pango_layout(pango_layout_new(pango_context.get()));
    pango_layout_set_attributes(pango_layout.get(),
                                attr_list);
    pango_layout_set_text(pango_layout.get(), text.data(), text.length());
    if (single_line) {
      pango_layout_set_width(pango_layout.get(), -1);
    } else {
      pango_layout_set_width(pango_layout.get(), wrapping_width * PANGO_SCALE);
      pango_layout_set_wrap(pango_layout.get(), PANGO_WRAP_WORD_CHAR);
    }
    pango_layout_set_ellipsize(pango_layout.get(), use_ellipsis && single_line ?
        PANGO_ELLIPSIZE_END : PANGO_ELLIPSIZE_NONE);
    return pango_layout.release();
  }

  void BroadcastMessage(Message& message) {
    Message send = message;
    for (std::set<RootUI*>::iterator it = all_windows_.begin();
         it != all_windows_.end();
         ++it) {
      send.ui = *it;
      send.platform_window = send.ui->GetPlatformWindow();
      (*it)->HandleMessage(send);
    }
  }

  static GdkFilterReturn WindowEventFilter(GdkXEvent* xevent,
                                           GdkEvent*,
                                           gpointer arg) {
    assert(arg);
    LinuxPlatform* this_ptr = reinterpret_cast<LinuxPlatform*>(arg);

    XEvent* event = reinterpret_cast<XEvent*>(xevent);
    switch (event->xany.type) {
      case PropertyNotify:
        if (event->xproperty.atom == this_ptr->workarea_atom_) {
          Message message;
          message.code = GL_MSG_DISPLAY_SETTINGS_CHANGED;
          this_ptr->BroadcastMessage(message);
        }
        break;
      default:
        break;
    }
    return GDK_FILTER_CONTINUE;
  }


  std::queue<Message> work_items_;
  LinuxResourceMap resources_;
  std::set<RootUI*> all_windows_;
  Atom workarea_atom_;

  DISALLOW_EVIL_CONSTRUCTORS(LinuxPlatform);
};

}  // namespace glint_linux

namespace glint {
Platform* Platform::Create() {
  // TODO(levin): Consider if this is an ok place or do we need
  // a Platform::Initialize()? gtk_init should only be called once.
  gtk_init(0, NULL);
  return new glint_linux::LinuxPlatform();
}
}  // namespace glint
