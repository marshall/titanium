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

#ifndef GEARS_DESKTOP_DROP_TARGET_FF_H__
#define GEARS_DESKTOP_DROP_TARGET_FF_H__
#ifdef OFFICIAL_BUILD
// The Drag-and-Drop API has not been finalized for official builds.
#else

#include <gecko_internal/nsIDragService.h>
#include <gecko_sdk/include/nsIDOMEventListener.h>
#include "gears/base/common/base_class.h"
#include "gears/base/common/js_runner.h"
#include "gears/base/common/js_types.h"
#include "gears/base/common/scoped_refptr.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

class DropTarget
    : public nsIDOMEventListener,
      public JsEventHandlerInterface {
 public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMEVENTLISTENER

  scoped_refptr<ModuleEnvironment> module_environment_;
  scoped_ptr<JsEventMonitor> unload_monitor_;
  scoped_ptr<JsRootedCallback> on_drag_enter_;
  scoped_ptr<JsRootedCallback> on_drag_over_;
  scoped_ptr<JsRootedCallback> on_drag_leave_;
  scoped_ptr<JsRootedCallback> on_drop_;

  DropTarget() {}

  // This is the JsEventHandlerInterface callback, not the
  // nsIDOMEventListener one. The latter is declared by the
  // NS_DECL_NSIDOMEVENTLISTENER above.
  virtual void HandleEvent(JsEventType event_type);

  bool GetDroppedFiles(nsIDragSession *drag_session,
                       JsArray *files_out,
                       std::string16 *error_out);

 private:
  DISALLOW_EVIL_CONSTRUCTORS(DropTarget);
};

#endif  // OFFICIAL_BUILD
#endif  // GEARS_DESKTOP_DROP_TARGET_FF_H__
