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

#ifndef GEARS_DESKTOP_DRAG_AND_DROP_REGISTRY_H__
#define GEARS_DESKTOP_DRAG_AND_DROP_REGISTRY_H__
#ifdef OFFICIAL_BUILD
// The Drag-and-Drop API has not been finalized for official builds.
#else

#include "gears/base/common/base_class.h"
#include "gears/base/common/js_types.h"

// A DropTarget is an opaque, platform-specific class that represents a DOM
// element that handles drag and drop events, for example of files dragged
// from the Desktop to the browser.
class DropTarget;
class JsDomElement;

class DragAndDropRegistry {
 public:
  static DropTarget *RegisterDropTarget(ModuleImplBaseClass *sibling_module,
                                        JsDomElement &dom_element,
                                        JsObject &js_callbacks,
                                        std::string16 *error_out);

 private:
  // Currently, a DropTarget automatically unregisters itself, on page unload,
  // but in the future we might provide a programmatic way to deregister a
  // drop target via a JavaScript API, and when we do, we'll promote this
  // method from private to public.
  static void UnregisterDropTarget(DropTarget *drop_target);

  friend class DropTarget;
  DISALLOW_EVIL_CONSTRUCTORS(DragAndDropRegistry);
};

#endif  // OFFICIAL_BUILD
#endif  // GEARS_DESKTOP_DRAG_AND_DROP_REGISTRY_H__
