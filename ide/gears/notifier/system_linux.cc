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

#ifdef OFFICIAL_BUILD
  // The notification API has not been finalized for official builds.
#else
#if defined(LINUX) && !defined(OS_MACOSX)
#include "gears/notifier/system.h"

#include <assert.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <pango/pango.h>
#include <pango/pangocairo.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include <string>

#include "gears/base/common/common.h"
#include "gears/base/common/file.h"
#include "gears/base/common/string_utils.h"
#include "third_party/google_perftools/src/base/sysinfo.h"
#include "third_party/glint/include/rectangle.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

// The character width for Sans 10pt.
const int kDefaultCharacterWidth = 7552;

// Cleanup/free functions for various types of objects
class ScopedGFree {
 public:
  void operator()(void *x) const {
    g_free(x);
  }
};

class ScopedPangoFontDescriptionFree {
 public:
  void operator()(void *x) const {
    pango_font_description_free(reinterpret_cast<PangoFontDescription*>(x));
  }
};

class ScopedGObjectUnref {
 public:
  void operator()(void *x) const {
    g_object_unref(x);
  }
};

class ScopedPangoFontMetricsUnref {
 public:
  void operator()(void *x) const {
    pango_font_metrics_unref(reinterpret_cast<PangoFontMetrics*>(x));
  }
};

// Various types of smart pointers for various types of objects with different
// cleanup requirements.
typedef scoped_ptr_malloc<char, ScopedGFree> ScopedCharGFree;
typedef scoped_ptr_malloc<PangoFontDescription, ScopedPangoFontDescriptionFree>
    ScopedPangoFontDescription;
typedef scoped_ptr_malloc<PangoContext, ScopedGObjectUnref>
    ScopedPangoContext;
typedef scoped_ptr_malloc<PangoFontMetrics, ScopedPangoFontMetricsUnref>
    ScopedPangoFontMetrics;

// Get the current module path. This is the path where the module of the
// currently running code sits.
static std::string16 GetCurrentModuleFilename() {
  ProcMapsIterator maps_iterator(0);
  if (!maps_iterator.Valid()) {
    return std::string16();
  }

  uint64 start = 0, end = 0, offset = 0;
  char *flags = NULL;
  char *filename = NULL;
  int64 inode = 0;

  unsigned int address_current_module =
      reinterpret_cast<unsigned int>(GetCurrentModuleFilename);
  while (maps_iterator.Next(&start, &end, &flags, &offset, &inode, &filename)) {
    if (start <= address_current_module && address_current_module <= end) {
      std::string16 module_filename;
      UTF8ToString16(filename, &module_filename);
      return module_filename;
    }
  }
  return std::string16();
}

std::string16 GetCurrentModulePath() {
  std::string16 path = GetCurrentModuleFilename();
  size_t idx = path.rfind('/');
  if (idx != std::string16::npos) {
    path = path.erase(idx);
  }
  return path;
}

std::string System::GetResourcePath() {
  std::string16 resource_path = GetCurrentModulePath();
  resource_path.append(STRING16(L"/notifier_resources"));
  std::string result;
  String16ToUTF8(resource_path.c_str(), &result);
  return result;
}

bool System::GetUserDataLocation(std::string16 *path, bool create_if_missing) {
  assert(path);

  *path = STRING16(L"~/.");
  *path += STRING16(PRODUCT_SHORT_NAME);

  if (create_if_missing && !File::DirectoryExists(path->c_str())) {
    if (!File::RecursivelyCreateDir(path->c_str())) {
      return false;
    }
  }

  return true;
}

bool InternalGetMainScreenWorkArea(glint::Rectangle *bounds) {
  assert(bounds);

  Display *display = GDK_DISPLAY();
  if (display == NULL) {
    return false;
  }
  Atom workarea_atom = XInternAtom(display, "_NET_WORKAREA", false);
  if (workarea_atom == 0) {
    return false;
  }
  Window window = GDK_ROOT_WINDOW();
  Atom type_returned = AnyPropertyType;
  int format_returned = 0;
  unsigned char *value_returned = NULL;
  unsigned long items_returned = 0;
  unsigned long remaining_bytes = 0;
  // TODO(levin): Handle multiple workspaces (and index
  // appropriately into workarea to get the boundaries for the
  // current one).
  XGetWindowProperty(display, window, workarea_atom, 0, 4,
                     false, XA_CARDINAL, &type_returned, &format_returned,
                     &items_returned, &remaining_bytes, &value_returned);
  if (!value_returned) {
    return false;
  }
  if (items_returned != 4 ||
      format_returned != XA_CARDINAL) {
    XFree(value_returned);
    return false;
  }

  int *work_area = reinterpret_cast<int*>(value_returned);
  bounds->Set(work_area[0],
              work_area[1],
              work_area[0] + work_area[2],
              work_area[1] + work_area[3]);
  XFree(value_returned);
  return true;
}

void System::GetMainScreenWorkArea(glint::Rectangle *bounds) {
  assert(bounds);

  if (InternalGetMainScreenWorkArea(bounds)) {
    return;
  }
  // As a last resort, if we couldn't get the work area,
  // we'll return the dimensions of the full screen.
  GdkRectangle rectangle;
  gdk_screen_get_monitor_geometry(gdk_screen_get_default(), 0, &rectangle);
  bounds->Set(rectangle.x,
              rectangle.y,
              rectangle.x + rectangle.width,
              rectangle.y + rectangle.height);
}

int GetDefaultCharacterWidth() {
  GtkSettings *settings = gtk_settings_get_default();
  if (!settings)
    return kDefaultCharacterWidth;

  ScopedCharGFree name;
  g_object_get(G_OBJECT(settings), "gtk-font-name", &name, NULL);
  if (!name.get())
    return kDefaultCharacterWidth;

  ScopedPangoFontDescription description(
      pango_font_description_from_string(name.get()));
  if (!description.get())
    return kDefaultCharacterWidth;

  // According to docs, the map "is owned by Pango and must not be freed."
  PangoFontMap *map = pango_cairo_font_map_get_default();
  if (!map)
    return kDefaultCharacterWidth;

  ScopedPangoContext context(
      pango_cairo_font_map_create_context(PANGO_CAIRO_FONT_MAP(map)));
  if (!context.get())
    return kDefaultCharacterWidth;

  PangoFont *font = pango_font_map_load_font(
      map, context.get(), description.get());
  if (!font)
    return kDefaultCharacterWidth;

  ScopedPangoFontMetrics metrics(pango_font_get_metrics(font, 0));
  if (!metrics.get())
    return kDefaultCharacterWidth;

  return pango_font_metrics_get_approximate_char_width(metrics.get());
}

double System::GetSystemFontScaleFactor() {
  double scale_factor = 1.0;

  GtkSettings *settings = gtk_settings_get_default();
  if (settings &&
      g_object_class_find_property(G_OBJECT_GET_CLASS(G_OBJECT(settings)),
                                   "gtk-xft-dpi")) {
    int dpi = 96;
    g_object_get(G_OBJECT(settings), "gtk-xft-dpi", &dpi, NULL);
    scale_factor *= static_cast<double>(dpi) / (96.0 * PANGO_SCALE);
  }

  int default_width = GetDefaultCharacterWidth();
  scale_factor *= default_width / static_cast<double>(kDefaultCharacterWidth);
  return scale_factor;
}

int System::ShowContextMenu(const MenuItem *menu_items,
                            size_t menu_items_count,
                            glint::RootUI *root_ui) {
  // TODO: Implement this.
  return -1;
}

void System::ShowNotifierPreferences() {
  // TODO: Implement this.
  assert(false);
}

bool System::OpenUrlInBrowser(const char16 *wide_url) {
  return false;
}

#endif  // defined(LINUX) && !defined(OS_MACOSX)
#endif  // OFFICIAL_BUILD
