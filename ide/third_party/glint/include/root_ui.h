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

// RootUI is a root of display tree. The display tree itself
// consists of Node instances, with root node having an instance of
// RootUI as a peer. You can always go straight to the RootUI
// object by using Node::GetParentUI() method.

#ifndef GLINT_INCLUDE_ROOT_UI_H__
#define GLINT_INCLUDE_ROOT_UI_H__

#include <map>
#include <string>
#include "glint/include/array.h"
#include "glint/include/cursors.h"
#include "glint/include/base_object.h"
#include "glint/include/message.h"
#include "glint/include/node.h"
#include "glint/include/point.h"
#include "glint/include/rectangle.h"
#include "glint/include/work_item.h"

namespace glint {

class AnimationTimeline;
class Bitmap;
class PlatformWindow;

typedef void (*CloseHandler)(void* user_info);

class RootUI : public BaseObject {
 public:
  explicit RootUI(bool topmost);
  virtual ~RootUI();

  void StartSnapshots(std::string snapshot_directory) {
    snapshot_directory_ = snapshot_directory;
  }

  void StopSnapshots() { snapshot_directory_.clear(); }

  // Should this UI just "Hide()" instead of
  // "CloseAndDeleteAsync()" when there is an InteractiveClose?
  void set_hide_on_interactive_close(bool hide_on_interactive_close) {
    hide_on_interactive_close_ = hide_on_interactive_close;
  }

  // Schedules the given work item to run after layout but before rendering.
  // This gives the work item an opportunity to "fix up" the automatically
  // computed layout before rendering so there is no flicker. Examples
  // may include centering the node of unknown in advance size on the screen,
  // or having some node "hovering" near another one with dynamic size.
  // Takes ownership of work_item.
  bool PostLayoutWorkItem(WorkItem* work_item);

  Node* FindNodeById(const std::string &id);

  bool CloseAndDeleteAsync();

  // Called before the RootUI is destroyed.
  // Disconnect from UI nodes, they are about to go away.
  virtual void OnClose() {
  }

  bool StartMouseCapture(Node *capture_node);
  void EndMouseCapture();

  Node *root_node() const { return root_node_; }
  bool set_root_node(Node *node);

  //
  // Starting from here, there are methods that are typically not used by
  // the developers who are using Glint. They are used by Glint components to
  // call into each other.

  // Schedules a work item to call UpdateUI later.
  void Invalidate();

  virtual MessageResultCode HandleMessage(const Message &message);

  bool SetCaptionText(const std::string& text);
  bool BringToForeground();

  bool StartWindowFlash();
  bool StopWindowFlash();

  bool SetCursor(Cursors cursor_type);

  bool SetIcon(const char* icon_name);

  // new focus_node can be NULL to 'reset' keyboard focus.
  bool SetFocus(Node* focus_node);

  // An interactive close (user clicked on a close button, etc.)
  // Removes the window from the screen and from the taskbar/dock.
  void InteractiveClose();
  // Removes the window from the screen and from the taskbar/dock.
  void Hide();
  // Removes the window from the screen and leaves "unminimize" OS controls.
  void Minimize();
  // Shows window but does not switch input into it (no focus change).
  void Show();
  // Shows window and switches input into it.
  void ShowInteractive();

  bool is_on_taskbar() const { return is_on_taskbar_; }
  bool has_focus() const { return has_focus_; }

  // Broadcast message to all nodes in the subtree of 'node', including itself.
  // Broadcasted messages can not be 'handled' and they do not return results.
  static void BroadcastMessage(Node *node, const Message &message);

  // Directs message to the node that has keyboard focus.
  // If nobody has focus, directs it to the root node.
  void RouteToFocusedNode(const Message &message);

  // in screen coordinates
  const Rectangle& final_bounds() const { return final_bounds_; }

  Bitmap *bitmap() const { return bitmap_; }

  // Returns node that currently has mouse capture. May return NULL.
  Node *capture_node() { return capture_node_; }

  // Returns node that currently has keyboard focus. May return NULL.
  Node *focus_node() { return focus_node_; }

  bool follow_window_position() const { return follow_window_position_; }

  void set_follow_window_position(bool follow_window_position) {
    follow_window_position_ = follow_window_position;
  }

  PlatformWindow* GetPlatformWindow();
  bool ClosePlatformWindow();

  // Animation Support
  void AddPendingAnimation(AnimationTimeline* animation);
  void RemovePendingAnimation(AnimationTimeline* animation);

#ifdef GLINT_ENABLE_XML
  static BaseObject *CreateInstance() { return new RootUI(true); }
  static SetPropertyResult SetChild(BaseObject *node,
                                    BaseObject *value) {
    static_cast<RootUI*>(node)->set_root_node(static_cast<Node*>(value));
    return PROPERTY_OK;
  }
#endif  // GLINT_ENABLE_XML

 private:
  bool DoLayoutLoop();
  bool UpdateUI();
  bool Layout();
  bool Draw();

  // Takes a snapshot of the display elements rooted at this node
  bool TakeSnapshot();

  void PostUpdate();
  bool CreatePlatformWindow();
  bool UpdatePlatformWindow(const Rectangle& previous_bounds);

  // Hit testing and routing of the mouse (or other pointer-like) messages.
  // The logic of routing has 3 steps:
  //
  // 1. Collect a 'hit test chain' of nodes that happen to be under a given
  //    mouse location.
  // 2. Walking the chain top-down, call HandleMessage on the nodes in
  //    the chain. If any of them returns MESSAGE_HANDLED, stop whole
  //    processing. This way, parents and containers have an opportunity to
  //    interrupt processing otherwise directed to their descendants.
  // 3. Walking bottom-up, call HandleMessage on the nodes again ("bubbling
  //    phase"). Again, terminate the whole processing on the first node
  //    returning MESSAGE_HANDLED.
  bool CollectNodesUnderPoint(Vector point, HitTestChain *chain);

  bool FireMouseEvent(const Message &message,
                      const HitTestResult &hit_test_result,
                      bool bubble);

  void HandleMouseMessage(const Message &message);
  void HandleWorkItem(WorkItem *work_item);

  bool SendLocalEvent(Node* node, GlintMessages code);

  void StartPendingAnimations(real64 current_time);

  void OptimizeDirtyRectangles();

  void UpdateRootNodeMargin(Point offset);

  void InvalidateSubtree(Node *root);

  PlatformWindow *platform_window_;
  Node* root_node_;
  Bitmap* bitmap_;

  Node* capture_node_;
  Node* focus_node_;

  Rectangle final_bounds_;

  Array<WorkItem> layout_work_items_;
  Array<Rectangle> dirty_rectangles_;

  std::map<const AnimationTimeline*, AnimationTimeline*> pending_animations_;
  typedef std::map<const AnimationTimeline*,
                   AnimationTimeline*>::iterator AnimationIterator;

  Message last_mouse_message_;

  bool is_invalidated_ ;
  bool update_posted_;
  bool is_topmost_;
  bool is_snapshot_pending_;
  bool is_on_taskbar_;
  bool has_focus_;
  bool hide_on_interactive_close_;
  bool follow_window_position_;
  std::string snapshot_directory_;

  DISALLOW_EVIL_CONSTRUCTORS(RootUI);
};

}  // namespace glint

#endif  // GLINT_INCLUDE_ROOT_UI_H__
