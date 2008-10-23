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

#include <fstream>
#include <sstream>
#include "glint/include/root_ui.h"
#include "glint/crossplatform/tile_plane.h"
#include "glint/crossplatform/core_util.h"
#include "glint/include/animation_timeline.h"
#include "glint/include/bitmap.h"
#include "glint/include/current_time.h"
#include "glint/include/draw_stack.h"
#include "glint/include/snapshot_stack.h"

// Uncomment to dump UpdateUI time into debugger log.
// #define UPDATE_PERF

namespace glint {

static const real64 kObscuredCheckInterval = 0.1;  // 100 ms

class CloseAndDeleteWindowWorkItem : public WorkItem {
 public:
  explicit CloseAndDeleteWindowWorkItem(RootUI* root_ui)
    : root_ui_(root_ui) {
    ASSERT(root_ui);
  }

  virtual void Run() {
    delete root_ui_;
    root_ui_ = NULL;
  }

 private:
  RootUI* root_ui_;
  DISALLOW_EVIL_CONSTRUCTORS(CloseAndDeleteWindowWorkItem);
};

RootUI::RootUI(bool topmost)
    : platform_window_(NULL),
      root_node_(NULL),
      bitmap_(NULL),
      capture_node_(NULL),
      focus_node_(NULL),
      is_invalidated_(false),
      update_posted_(false),
      is_snapshot_pending_(false),
      is_on_taskbar_(false),
      has_focus_(false),
      hide_on_interactive_close_(false),
      follow_window_position_(true) {
  is_topmost_ = topmost;
}

RootUI::~RootUI() {
  set_root_node(NULL);
  ClosePlatformWindow();
}

bool RootUI::CloseAndDeleteAsync() {
  platform()->PostWorkItem(this, new CloseAndDeleteWindowWorkItem(this));
  OnClose();
  return true;
}

bool RootUI::ClosePlatformWindow() {
  delete bitmap_;
  bitmap_ = NULL;
  if (platform_window_) {
    platform()->DeleteInvisibleWindow(platform_window_);
    platform_window_ = NULL;
  }
  return true;
}

// 'bubble' means the message processing is going from children to parents
// messages are processed in both directions - first from root down (notify
// phase) then from the bottom up (bubble phase).
bool RootUI::FireMouseEvent(const Message &message,
                            const HitTestResult &hit_test_result,
                            bool bubble) {
  Node *node = hit_test_result.node;
  if (node) {
    Message new_message = message;
    new_message.bubble = bubble;
    new_message.mouse_position = hit_test_result.local_point;
    MessageResultCode code = node->HandleMessage(new_message);
    if (code == MESSAGE_HANDLED)
      return true;
  }
  return false;
}

void RootUI::HandleMouseMessage(const Message &message) {
  // Broadcast mousemove in addition to notify/fire sequence.
  // This allows nodes that are not under mouse to receive mousemoves
  // and form events like MouseEnter/MouseLeave
  if (message.code == GL_MSG_MOUSEMOVE) {
    Message mousemove_message = message;
    mousemove_message.code = GL_MSG_MOUSEMOVE_BROADCAST;
    BroadcastMessage(root_node_, mousemove_message);
  }

  HitTestChain chain;
  if (!CollectNodesUnderPoint(message.screen_mouse_position, &chain))
    return;

  if (chain.length() == 0)
    return;

  // first phase - notify from parents down. To distinguish this phase, we use
  // XXX_NOTIFY message codes.
  Message notify_message = message;
  switch (message.code) {
    case GL_MSG_LBUTTONDOWN:
      notify_message.code = GL_MSG_LBUTTONDOWN_NOTIFY;
      break;
    case GL_MSG_LBUTTONUP:
      notify_message.code = GL_MSG_LBUTTONUP_NOTIFY;
      break;
    case GL_MSG_RBUTTONDOWN:
      notify_message.code = GL_MSG_RBUTTONDOWN_NOTIFY;
      break;
    case GL_MSG_RBUTTONUP:
      notify_message.code = GL_MSG_RBUTTONUP_NOTIFY;
      break;
    case GL_MSG_MBUTTONDOWN:
      notify_message.code = GL_MSG_MBUTTONDOWN_NOTIFY;
      break;
    case GL_MSG_MBUTTONUP:
      notify_message.code = GL_MSG_MBUTTONUP_NOTIFY;
      break;
    case GL_MSG_MOUSEMOVE:
      notify_message.code = GL_MSG_MOUSEMOVE_NOTIFY;
      break;
    default:
      break;
  }

  notify_message.user_data = chain[chain.length() - 1]->node;
  for (int i = 0; i < chain.length(); ++i) {
    // 'false' means 'notify' phase.
    if (FireMouseEvent(notify_message, *chain[i], false))
      return;
  }

  // second phase - event bubbling (from chidlren to parents)
  Message bubble_message = message;
  bubble_message.user_data = chain[chain.length() - 1]->node;
  for (int i = chain.length() - 1; i >= 0; --i) {
    // 'true' means 'bubble' phase.
    if (FireMouseEvent(bubble_message, *chain[i], true))
      return;
  }

  // If it was MOUSEMOVE and nobody handled it, set the cursor.
  if (message.code == GL_MSG_MOUSEMOVE) {
    SetCursor(CURSOR_POINTER);
  }
}

void RootUI::HandleWorkItem(WorkItem *work_item) {
  if (work_item) {
    work_item->Run();
    delete work_item;
  } else {
    update_posted_ = false;
    UpdateUI();
  }
}

void RootUI::BroadcastMessage(Node *node, const Message &message) {
  if (!node)
    return;
  for (Node *child = node->first_child();
       child;
       child = child->next_sibling()) {
    BroadcastMessage(child, message);
  }
  node->HandleMessage(message);
}

void RootUI::RouteToFocusedNode(const Message &message) {
  Node* node = focus_node_ ? focus_node_ : root_node_;

  if (!node)
    return;

  node->HandleMessage(message);
}

void RootUI::UpdateRootNodeMargin(Point offset) {
  if (!root_node_ ||
      !root_node_->previously_rendered())
    return;

  Point computed_origin = final_bounds_.origin();
  int dx = offset.x - computed_origin.x;
  int dy = offset.y - computed_origin.y;

  // Since rendering bounds (and actual position of the underlying window)
  // are computed using floating point matrices, various rounding effects
  // may cause 1-off errors. If we don't have this hysteresis of 1,
  // we may get an auto-generating effect.
  if (abs(dx) > 1 || abs(dy) > 1) {
    Rectangle margin;
    margin.Set(root_node_->margin());
    margin.set_left(margin.left() + dx);
    margin.set_top(margin.top() + dy);
    root_node_->set_margin(margin);
  }
}

void RootUI::InvalidateSubtree(Node *root) {
  if (!root)
    return;
  root->Invalidate();
  for (Node *child = root->first_child();
       child;
       child = child->next_sibling()) {
    InvalidateSubtree(child);
  }
}

MessageResultCode RootUI::HandleMessage(const Message &message) {
  ASSERT(message.ui == this);
  int code = message.code;

  switch (code) {
    case GL_MSG_LBUTTONDOWN:
    case GL_MSG_LBUTTONUP:
    case GL_MSG_RBUTTONDOWN:
    case GL_MSG_RBUTTONUP:
    case GL_MSG_MBUTTONDOWN:
    case GL_MSG_MBUTTONUP:
    case GL_MSG_MOUSEMOVE:
      last_mouse_message_ = message;
      HandleMouseMessage(message);
      return MESSAGE_HANDLED;

    case GL_MSG_WORK_ITEM:
      HandleWorkItem(message.work_item);
      return MESSAGE_HANDLED;

    case GL_MSG_KEYDOWN:
    case GL_MSG_KEYUP:
    case GL_MSG_COPY:
    case GL_MSG_CUT:
    case GL_MSG_PASTE:
    case GL_MSG_SELECT_ALL:
    case GL_MSG_CLOSE_DOCUMENT:
      RouteToFocusedNode(message);
      return MESSAGE_HANDLED;

    // We get these two from platform, notify the local focused node
    // that the window (and therefore local focused node) got focus.
    case GL_MSG_KILLFOCUS:
      has_focus_ = false;
      RouteToFocusedNode(message);
      return MESSAGE_HANDLED;

    case GL_MSG_SETFOCUS:
      has_focus_ = true;
      RouteToFocusedNode(message);
      StopWindowFlash();
      return MESSAGE_HANDLED;

    case GL_MSG_IDLE:
      is_snapshot_pending_ = !snapshot_directory_.empty();
      BroadcastMessage(root_node_, message);
      return MESSAGE_HANDLED;

    case GL_MSG_MOUSELEAVE:
      last_mouse_message_.Clear();
      BroadcastMessage(root_node_, message);
      return MESSAGE_HANDLED;

    case GL_MSG_CAPTURELOST:
      if (capture_node_) {
        capture_node_->HandleMessage(message);
        // don't delete, the node is still alive just without capture.
        capture_node_ = NULL;
      }
      return MESSAGE_HANDLED;

    case GL_MSG_WINDOW_POSITION_CHANGED:
      // If follow_window_position_ is false, do not update the root node margin
      // to avoid double adjustment.
      if (follow_window_position_) {
        UpdateRootNodeMargin(message.window_position);
      }
      break;

    case GL_MSG_DISPLAY_SETTINGS_CHANGED: {
#ifdef WIN32
      // Move the window. Windows changes its position without sending
      // corresponding messages (WM_WINDOWPOSCHANGED). One repro is
      // Media Player 9 or 10 as described above.
      UpdateRootNodeMargin(message.window_position);
#endif
      break;
    }

    default:
      break;
  }

  Node *root = root_node();
  if (root)
    return root->HandleMessage(message);

  // nothing happened, continue search for other handlers
  return MESSAGE_CONTINUE;
}

bool RootUI::set_root_node(Node *node) {
  if (node != root_node_) {
    delete root_node_;
    root_node_ = node;
    if (node) {
      node->SetParentUI(this);
      if (!platform_window_) {
        CHECK(CreatePlatformWindow());
      }
    }
    Invalidate();
  }
  return true;
}

bool RootUI::CreatePlatformWindow() {
  ASSERT(platform_window_ == NULL);
  platform_window_ = platform()->CreateInvisibleWindow(
    this, is_topmost_, !is_topmost_);
  return platform_window_ ? true : false;
}

bool RootUI::UpdatePlatformWindow(const Rectangle& previous_bounds) {
  if (platform_window_ && bitmap_) {
    Point screen_origin(final_bounds_.origin());
    bool screen_origin_changed = (previous_bounds.origin() != screen_origin);

    Size screen_size(final_bounds_.size());

    Rectangle* area = NULL;

#ifdef OSX
    // glint always pads the bitmap it allocates, and on OSX, the padding ends
    // up at high X and low Y. The high X gets clipped, but the low Y needs to
    // be accounted for.
    area = new Rectangle(0, bitmap_->size().height - screen_size.height,
                         screen_size.width, bitmap_->size().height);
#endif

    CHECK(platform()->UpdateInvisibleWindow(platform_window_,
      (screen_origin_changed ? &screen_origin : NULL),
      screen_size,
      bitmap_->platform_bitmap(),
      NULL,
      area));

    if (area != NULL) {
      delete area;
      area = NULL;
    }
  }
  return true;
}

void RootUI::Invalidate() {
  if (!is_invalidated_) {
    PostUpdate();
    is_invalidated_ = true;
  }
}

bool RootUI::Layout() {
  if (root_node()) {
    // no constraints for the root element
    Size required = root_node()->ComputeRequiredSize(Size::HugeSize());
    Rectangle bounds(Point(), required);
    root_node()->SetLayoutBounds(bounds);

    // ComputeDrawingBounds needs drawing stack without target bitmap.
    DrawStack stack(NULL);
    final_bounds_.Reset();
    dirty_rectangles_.Reset();

    if (!root_node()->ComputeDrawingBounds(&stack,
                                           &final_bounds_,
                                           &dirty_rectangles_)) {
      return false;
    }
    // if whole UI is transparent, the final_bounds become empty.
    // UpdateTransparentWindow does not like dealing with empty windows,
    // so make it 1x1 pixel.
    if (final_bounds_.IsEmpty()) {
      final_bounds_.Set(0, 0, 1, 1);
    }
  }
  is_invalidated_ = false;
  return true;
}

// Layout is a loop - calculate layout, then run user-scheduled layout tasks.
// These tasks run until first invalidation - then the layout is done again.
// This mechanism guarantees that user-scheduler post-layout tasks will be
// executed only when layout is 'clean'.
bool RootUI::DoLayoutLoop() {
  // TODO(dimich): pretty random, but usually ok. If the layout is not
  // stabilized after this amount of loops, we'll render incomplete
  // picture. Need to add posted invalidation for this case.
  int layout_cycles = 153;
  while (is_invalidated_ && (--layout_cycles > 0)) {
    if (!Layout())
      return false;

    // Make a copy of post-layout work items queue.
    // We will run them through until they all done.
    // This may generate other work items - for the next layout cycle.
    Array<WorkItem> work_items_copy;
    work_items_copy.Swap(&layout_work_items_);

    while (work_items_copy.length() > 0) {
      WorkItem* item = work_items_copy[0];
      item->Run();
      work_items_copy.RemoveAt(0);
      // If running the item caused invalidation - stop dispatching
      // the items and clean up layout again.
      if (is_invalidated_) {
        if (!Layout())
          return false;
      }
    }

    // If the mouse is over the root ui (the last_mouse_message_ was not
    // cleared by GL_MSG_MOUSELEAVE event), we have to simulate a mouse move
    // to trigger hittest and proper mouse events if the objects underneath
    // the mouse moved and changed.
    if (last_mouse_message_.code != GL_MSG_IDLE) {
      // In case the last mouse message was not a "mouse move", make it "mouse
      // move". The rest of information (pressed buttons, modifiers and
      // location) should stay the same as in the last mouse message.
      last_mouse_message_.code = GL_MSG_MOUSEMOVE;
      HandleMessage(last_mouse_message_);
      if (is_invalidated_)
        continue;
    }

    // We are past layout and layout work items. So it's in a stable snapshot.
    // Now, start all pending (new) animations. They will grab post-trigger
    // (final) property values as they are being started.
    StartPendingAnimations(CurrentTime::Seconds());
  }
  return true;
}

bool RootUI::UpdateUI() {
  if (!is_invalidated_)
    return true;

#ifdef UPDATE_PERF
  real64 start_time = CurrentTime::Seconds();
#endif

#ifdef DEBUG_DIRTY_RECTS
  platform()->Trace("UpdateUI: %s\n",
                    ((root_node_ && root_node_->layout_invalidated()) ?
                    "layout+render" : "render_only"));
#endif

  Rectangle previous_bounds;
  previous_bounds.Set(final_bounds_);

  // Do layout to determine final_bounds
  if (!DoLayoutLoop())
    return false;

  // First, make sure our backing bitmap is of proper size
  // Make it change in steps to avoid frequent reallocations when
  // UI tree changes its bounds
  Size final_size = final_bounds_.size();
  int required_width = final_size.width;
  int required_height = final_size.height;

  if (!bitmap_ ||
      bitmap_->size().width != required_width ||
      bitmap_->size().height != required_height) {
    delete bitmap_;
    bitmap_ = new Bitmap(Point(), Size(required_width, required_height));
    dirty_rectangles_.Reset();
    Rectangle* whole_bitmap = new Rectangle();
    whole_bitmap->Set(final_bounds_);
    dirty_rectangles_.Add(whole_bitmap);
  }

  // render if no more work left
  if (!is_invalidated_) {
    CHECK(Draw());
    CHECK(UpdatePlatformWindow(previous_bounds));

    if (is_snapshot_pending_) {
      is_snapshot_pending_ = false;
      if (!TakeSnapshot())  // snapshot failed: no more snapshots
        snapshot_directory_.clear();
    }
  } else {
    PostUpdate();
  }

#ifdef UPDATE_PERF
    static real64 total_duration = 0;
    static int samples = 0;
    real64 duration_ms = (CurrentTime::Seconds() - start_time) * 1000.0;
    total_duration += duration_ms;
    ++samples;
    real64 average = total_duration / samples;
    platform()->TraceAlways("UPDATE_PERF: %.1f ms, average: %.1f\n",
                            duration_ms, average);
#endif

  return true;
}

void RootUI::InteractiveClose() {
  if (hide_on_interactive_close_) {
    Hide();
    return;
  }
  CloseAndDeleteAsync();
}

void RootUI::Hide() {
  if (platform()->HideWindow(platform_window_)) {
    has_focus_ = false;
    is_on_taskbar_ = false;
  }
}

void RootUI::Minimize() {
  if (platform()->MinimizeWindow(platform_window_)) {
    has_focus_ = false;
    is_on_taskbar_ = true;
  }
}

void RootUI::ShowInteractive() {
  if (platform()->ShowInteractiveWindow(platform_window_)) {
    has_focus_ = true;
    is_on_taskbar_ = true;
  }
}

void RootUI::Show() {
  if (platform()->ShowWindow(platform_window_)) {
    has_focus_ = false;
    is_on_taskbar_ = true;
  }
}

// Dirty rectangles reported by various nodes across the tree may overlap
// or abutt. In this case, we want to merge them into a minimal set of
// non-overlapping rectangles.
void RootUI::OptimizeDirtyRectangles() {
  if (dirty_rectangles_.length() <= 1) {
#ifdef DEBUG_DIRTY_RECTS
    Rectangle rect(*dirty_rectangles_[0]);
    platform()->Trace("Render rect: %d,%d %d,%d\n",
                      rect.left(), rect.top(), rect.right(), rect.bottom());
#endif
    return;
  }

  Plane plane;
  for (int i = 0; i < dirty_rectangles_.length(); ++i) {
    Rectangle* dirty_rectangle = dirty_rectangles_[i];
    if (!dirty_rectangle->IsEmpty()) {
      plane.CreateTile(*dirty_rectangle, Tile::FIRST_USER_TYPE);
    }
  }

  dirty_rectangles_.Reset();
  AreaIterator iterator(&plane, final_bounds_);
  while (iterator.GetNextTile()) {
    if (iterator.current()->type == Tile::FIRST_USER_TYPE) {
      Rectangle* new_rect = new Rectangle(iterator.current()->origin(),
                                          iterator.current()->size());
#ifdef DEBUG_DIRTY_RECTS
      platform()->Trace("DR: %d,%d %d,%d\n",
                        new_rect->left(),
                        new_rect->top(),
                        new_rect->right(),
                        new_rect->bottom());
#endif
      dirty_rectangles_.Add(new_rect);
    }
  }
}

bool RootUI::Draw() {
  ASSERT(!is_invalidated_);
  if (!root_node_ ||
      !bitmap_ ||
      bitmap_->size().width <= 0 ||
      bitmap_->size().height <= 0)
    return true;

  // Since bitmap only covers area inside the final_bounds_, set
  // destination offset on it.
  bitmap_->set_origin(final_bounds_.origin());
  // Note: order of these 2 lines is important, the DrawStack ctor
  // uses offset of the target bitmap to set initial clip.
  DrawStack stack(bitmap());
  OptimizeDirtyRectangles();

  for (int i = 0; i < dirty_rectangles_.length(); ++i) {
    Rectangle* dirty_rectangle = dirty_rectangles_[i];
    dirty_rectangle->Intersect(final_bounds_);

    // Erase content in the dirty area
    bitmap_->Fill(colors::kTransparent, *dirty_rectangle);

    Transform identity;
    CHECK(stack.Push(identity, *dirty_rectangle, false, 255));
    CHECK(root_node()->Draw(&stack));
    stack.Pop();
  }

#ifdef VISUALIZE_DIRTY_RECTANGLES
  for (int i = 0; i < dirty_rectangles_.length(); ++i) {
    Rectangle* dirty_rectangle = dirty_rectangles_[i];
    dirty_rectangle->Intersect(final_bounds_);
    Color random_color;
    random_color.Set(0x40,
                     rand() % 255,    // NOLINT (asks for rand_r)
                     rand() % 255,    // NOLINT (asks for rand_r)
                     rand() % 255);   // NOLINT (asks for rand_r)
    random_color.Premultiply();
    bitmap()->Fill(random_color, *dirty_rectangle);
  }
#endif

  return true;
}

bool RootUI::TakeSnapshot() {
  if (root_node()) {
    SnapshotStack stack;
    Transform identity;
    stack.Push(identity, "root");
    bool success = root_node()->Snapshot(&stack);
    stack.Pop();
    if (success) {
      std::stringstream sstream;
      int current_100ns_time = static_cast<int>(CurrentTime::Seconds() * 10e7);
      sstream << snapshot_directory_ << '\\' << current_100ns_time;
      std::ofstream dump_file(sstream.str().c_str(), std::ios_base::trunc);
      if (dump_file.good()) {
        dump_file << stack.xml();
        dump_file.close();
      } else {
        success = false;
      }
    }
    return success;
  }
  return true;
}

void RootUI::PostUpdate() {
  if (!update_posted_) {
    platform()->PostWorkItem(this, NULL);
    update_posted_ = true;
  }
}

bool RootUI::StartMouseCapture(Node *capture_node) {
  if (!capture_node)
    return false;
  RootUI *root_ui = capture_node->GetRootUI();
  if (root_ui != this)
    return false;

  // If the capture is just switching within the app, do only local signaling.
  if (capture_node_) {
    capture_node_ = capture_node;
    SendLocalEvent(capture_node_, GL_MSG_CAPTURELOST);
    return true;
  } else {
    capture_node_ = capture_node;
    return platform()->StartMouseCapture(this);
  }
}

void RootUI::EndMouseCapture() {
  capture_node_ = NULL;
  platform()->EndMouseCapture();
}

bool RootUI::SetCaptionText(const std::string& text) {
  return platform()->SetWindowCaptionText(platform_window_, text);
}

bool RootUI::BringToForeground() {
  return platform()->BringWindowToForeground(platform_window_);
}

bool RootUI::StartWindowFlash() {
  return platform()->StartWindowFlash(platform_window_);
}

bool RootUI::StopWindowFlash() {
  return platform()->StopWindowFlash(platform_window_);
}

bool RootUI::SetCursor(Cursors cursor_type) {
  return platform()->SetCursor(cursor_type);
}

bool RootUI::SetIcon(const char* icon_name) {
  return platform()->SetIcon(platform_window_, icon_name);
}

bool RootUI::SendLocalEvent(Node* node, GlintMessages code) {
  if (!node)
    node = root_node_;

  if (!node)
    return false;

  Message message;
  message.code = code;
  message.ui = this;
  message.platform_window = platform_window_;

  node->HandleMessage(message);
  return true;
}

bool RootUI::SetFocus(Node* focus_node) {
  if (focus_node == focus_node_)
    return true;

  if (focus_node_) {
    SendLocalEvent(focus_node_, GL_MSG_KILLFOCUS);
    focus_node_ = NULL;
  }

  if (focus_node) {
    SendLocalEvent(focus_node, GL_MSG_SETFOCUS);
    focus_node_ = focus_node;
    platform()->SetFocus(platform_window_);
  }

  return true;
}

bool RootUI::CollectNodesUnderPoint(Vector point, HitTestChain *chain) {
  ASSERT(chain && chain->length() == 0);
  if (!chain || chain->length() != 0)
    return false;

  // In case of mouse capture, route the event directly to the capturing node
  if (capture_node_) {
    // Find total accumulated transform from screen space to local space.
    Transform to_local;
    Node *node = capture_node_;
    while (node) {
      Transform transform;
      node->GetTransformToLocal(&transform);
      to_local.AddPostTransform(transform);
      node = node->parent();
    }

    HitTestResult* result = new HitTestResult();
    if (!result)
      return false;
    result->node = capture_node_;
    result->local_point = to_local.TransformVector(point);
    chain->Add(result);
    return true;
  }

  Node *node = root_node_;
  if (!node)
    return true;
  // HitTest is similar to rendering - same transforms, clips and
  // composition apply. So we use DrawStack and same recursive approach as
  // for rendering.
  DrawStack stack(NULL);
  return node->HitTestDeep(&stack, point, chain);
}

Node* RootUI::FindNodeById(const std::string &id) {
  Node *root = root_node_;
  if (!root)
    return NULL;
  return root->FindNodeById(id);
}

bool RootUI::PostLayoutWorkItem(WorkItem* work_item) {
  layout_work_items_.Add(work_item);
  return true;
}

void RootUI::AddPendingAnimation(AnimationTimeline* animation) {
  AnimationIterator found = pending_animations_.find(animation);
  if (found != pending_animations_.end())
    return;
  pending_animations_[animation] = animation;
  Invalidate();
}

void RootUI::RemovePendingAnimation(AnimationTimeline* animation) {
  AnimationIterator found = pending_animations_.find(animation);
  if (found == pending_animations_.end())
    return;
  pending_animations_.erase(found);
}

void RootUI::StartPendingAnimations(real64 current_time) {
  for (AnimationIterator it = pending_animations_.begin();
       it != pending_animations_.end();
       ++it) {
    ASSERT(it->second->state() == AnimationTimeline::INITIALIZED);
    it->second->Advance(current_time);
  }
  pending_animations_.clear();
}

PlatformWindow* RootUI::GetPlatformWindow() {
  return platform_window_;
}

}  // namespace glint
