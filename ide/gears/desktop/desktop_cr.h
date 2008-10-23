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

#ifndef GEARS_DESKTOP_DESKTOP_CR_H__
#define GEARS_DESKTOP_DESKTOP_CR_H__

#include "gears/desktop/desktop.h"
#include "third_party/chrome/gears_api.h"
#include "gears/ui/chrome/html_dialog_cr.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

class ShortcutIconHandler;

class ModelessShortcutsDialog {
 public:
  ModelessShortcutsDialog(CPBrowsingContext context);
  virtual ~ModelessShortcutsDialog();

  // Show the shortcuts dialog with the given 'shortcut_data' from CPAPI.
  bool ShowDialog(GearsShortcutData2 *shortcut_data);

  // Handle the results.
  void ResultsReady();

 private:
  // Start HttpRequests to fetch the icon data.  This will fill in the icon's
  // png_data when the data is ready.  The main reason to do this is to avoid
  // the sync fetch done by Desktop - Chrome can't do synch fetches in the
  // browser process.
  bool PrefetchIcons();

  // Called when an icon has been fetched.  The index refers to a handler
  // in icon_handler_.
  void IconRequestFinished(int index);

  // Cancel any icon fetches in progress.
  void AbortPrefetchIcons();

  // Returns true if all icons have been fetched.
  bool AreIconsReady();

  // Tell the Desktop object that results are ready.  We don't do this until
  // the dialog is closed and icons are all fetched.
  void HandleDialogResults();

  scoped_ptr<Desktop> desktop_;

  Desktop::ShortcutInfo shortcut_info_;

  CPBrowsingContext browsing_context_;

  // A pointer to the original data provided to us, so we can send it back to
  // Chrome.
  GearsShortcutData2 *shortcut_data_;

  // True if DialogClosed has been called.
  bool results_ready_;
  scoped_ptr<HtmlDialog> dialog_;

  // Requests for icon prefetching.
  scoped_ptr<ShortcutIconHandler> icon_handler_[4];
  friend class ShortcutIconHandler;
  DISALLOW_EVIL_CONSTRUCTORS(ModelessShortcutsDialog);
};

// Build a list of all shortcuts for use by Chrome.
bool GetAllShortcuts(GearsShortcutList *shortcut_list);

#endif  // GEARS_DESKTOP_DESKTOP_CR_H__
