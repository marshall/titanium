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

// Declaration of the Node class - base class for the Glint display tree.
// Objects of type Node (normally some specializations of the Node class) form
// n-ary tree which is a compositing tree for UI rendering, and also routing
// structure for events (such as mouse events).
// The Node implements following functions:
// 1. Treeness
// 2. Automatic layout (measurements with constraint, positioning)
// 3. Calculation of the actual drawing bounds (to allocate proper buffers)
// 4. Rendering
// 5. Hit-testing
// 6. Event routing
// The class derived from Node typically should override all virtual methods
// declared here.

#ifndef GLINT_INCLUDE_NODE_H__
#define GLINT_INCLUDE_NODE_H__

#include <string>
#include "glint/include/array.h"
#include "glint/include/base_object.h"
#include "glint/include/color.h"
#include "glint/include/message.h"
#include "glint/include/rectangle.h"
#include "glint/include/transform.h"
#include "glint/include/type_system.h"

namespace glint {

class AnimationTimeline;
class DrawStack;
class Node;
class SnapshotStack;

// HitTest chain is an array of HitTestResult's and describes
// a set of nodes that happen to be under a given point. Used
// for mouse message routing. See Node::HitTestDeep for more info.
struct HitTestResult : public BaseObject {
  Node* node;
  Vector local_point;
};
typedef Array<HitTestResult> HitTestChain;

// To process Glint messages, one can derive from Node and
// override HandleMessage(). Alternative is to create a handler
// derived from this class and add it to the Node via
// Node::AddHandler. Latter method avoids deriving a new node type.
class MessageHandler : public BaseObject {
 public:
  virtual ~MessageHandler() {}
  virtual MessageResultCode HandleMessage(const Message& message) = 0;
};

enum VerticalAlignment {
  Y_TOP,
  Y_CENTER,
  Y_BOTTOM,
  Y_FILL
};

enum HorizontalAlignment {
  X_LEFT,
  X_CENTER,
  X_RIGHT,
  X_FILL
};

enum AnimatedTransition {
  ALPHA_TRANSITION = 0,
  MOVE_TRANSITION,
  USER_TRANSITION,
  LAST_TRANSITION
};

class Node : public BaseObject {
 public:
  Node();
  virtual ~Node();

  Node* first_child() const {
    return first_child_;
  }

  Node* last_child() const {
    return first_child_ ? first_child_ ->prev_sibling_ : NULL;
  }

  Node* prev_sibling() const {
    return parent_ ?
      (parent_->first_child_ == this ? NULL : prev_sibling_) :
      NULL;
  }

  Node* next_sibling() const {
    return next_sibling_;
  }

  Node* parent() const { return parent_; }

  RootUI* GetRootUI() const {
    for (const Node* node = this; node; node = node->parent_) {
      if (node->is_root()) return node->parent_ui_;
    }
    return NULL;
  }

  // Invalidates the node. Causes re-layout and re-drawing.
  void Invalidate();

  bool layout_invalidated() const {
    return layout_invalidated_;
  }

  bool previously_rendered() const {
    return previously_rendered_;
  }

  // Causes re-drawing only - does not trigger layout recalc.
  // Assumes that layout and drawing bounds are not changing - good
  // example is a background color change or a caret blinking.
  void InvalidateDrawing(const Rectangle& bounds);

  bool is_root() const { return is_root_; }
  void SetParentUI(RootUI* ui);

  // Adds a child to the end of the children list.
  // Parent node takes ownership of the child node, calling 'delete' on
  // any disconnected children or when it is itself deleted.
  bool AddChild(Node* new_child);

  // Disconnects the child and deletes it. This is typical way to remove the
  // child, as all the objects in Glint typically assume ownership of their
  // 'subobjects'.
  bool RemoveNode(Node* child);
  bool RemoveChildren();

  // Does not delete the child node, merely disconnects it.
  bool DisconnectNode(Node* child);
  bool DisconnectChildren();

  std::string id() const { return id_; }
  void set_id(const std::string& id) { id_ = id; }

  // Finds a node by id in the subtree of this node
  Node* FindNodeById(const std::string& id);

  // Adds a message handler instance to the internal list if message handlers.
  // This node takes ownership of the handler (deletes it when it is itself
  // deleted).
  bool AddHandler(MessageHandler* handler);
  virtual MessageResultCode HandleMessage(const Message& message);

  const Transform& transform() const { return transform_; }
  void set_transform(const Transform& transform) {
    if (!transform_.IsClose(transform)) {
      transform_.Set(transform);
      Invalidate();
    }
  }

  int alpha() const { return alpha_; }
  void set_alpha(int alpha) {
    if (alpha_ != alpha) {
      TriggerTransition(ALPHA_TRANSITION);
      alpha_ = alpha;
      Invalidate();
    }
  }

  const Rectangle& margin() const { return margin_; }
  void set_margin(const Rectangle& margin) {
    // Can not use IsEqual here because it includes checking
    // rectangles for IsEmpty(), and margin 'rectangel' is not a rectangle
    // but rather set of 4 independent numbers, which may make it 'empty' while
    // it is a very valid margin rect (for example, 10,10,10,10 is empty but
    // has to be set anyways.
    if (margin_.left() != margin.left() ||
        margin_.top() != margin.top() ||
        margin_.right() != margin.right() ||
        margin_.bottom() != margin.bottom()) {
      margin_.Set(margin);
      Invalidate();
    }
  }

  int min_width() const { return min_size_.width; }
  void set_min_width(int min_width) {
    if (min_size_.width != min_width) {
      min_size_.width = min_width;
      Invalidate();
    }
  }

  int min_height() const { return min_size_.height; }
  void set_min_height(int min_height) {
    if (min_size_.height != min_height) {
      min_size_.height = min_height;
      Invalidate();
    }
  }

  int max_width() const { return max_size_.width; }
  void set_max_width(int max_width) {
    if (max_size_.width != max_width) {
      max_size_.width = max_width;
      Invalidate();
    }
  }

  int max_height() const { return max_size_.height; }
  void set_max_height(int max_height) {
    if (max_size_.height != max_height) {
      max_size_.height = max_height;
      Invalidate();
    }
  }

  // Automatic Layout of nodes sometimes results in clipping.
  // Nodes can be clipped as a result of constraints on layout which can be
  // created by max_width/max_height properties or other conditions.
  // The clip computed by layout applies to the node and its subtree, clipping
  // all descendants too. Generally, this produces desired effect - for example,
  // if the user resizes a UI window to be smaller then UI elements can handle,
  // it makes sense to start clipping whole blocks of UI.
  // However, sometimes a node should render its content outside of layout
  // constraints (visualize tabs sticking out of the book). For these cases,
  // there is 'ignore_clip' property.

  // Indicates that the node is going to be clipped because it doesn't fit
  // into layout constraints.
  bool clip_at_bounds() const { return clip_at_bounds_; }

  // Sets flag preventing layout from clipping this node.
  bool ignore_clip() const { return ignore_clip_; }
  void set_ignore_clip(bool ignore_clip) {
    if (ignore_clip_ != ignore_clip) {
      ignore_clip_ = ignore_clip;
      Invalidate();
    }
  }

  VerticalAlignment vertical_alignment() const {
    return vertical_alignment_;
  }
  void set_vertical_alignment(VerticalAlignment alignment) {
    if (vertical_alignment_ != alignment) {
      vertical_alignment_ = alignment;
      Invalidate();
    }
  }

  HorizontalAlignment horizontal_alignment() const {
    return horizontal_alignment_;
  }
  void set_horizontal_alignment(HorizontalAlignment alignment) {
    if (horizontal_alignment_ != alignment) {
      horizontal_alignment_ = alignment;
      Invalidate();
    }
  }

  Color background() const { return background_; }
  void set_background(Color color) {
    if (background_ != color) {
      background_ = color;
      Invalidate();
    }
  }

  // XML Support.
  #ifdef GLINT_ENABLE_XML
  static BaseObject* CreateInstance() { return new Node(); }
  // Helper to set dimension-like property. Takes a pointer to
  // a private variable and does typical checks (>0, format, etc)
  static SetPropertyResult SetSizeFromString(const std::string& value,
                                             int* size);
  static SetPropertyResult SetAlpha(BaseObject* node,
                                    const std::string& value);
  static SetPropertyResult SetChild(BaseObject* node,
                                    BaseObject* value);
  static SetPropertyResult SetHorizontalAlignment(BaseObject* node,
                                                  const std::string& value);
  static SetPropertyResult SetVerticalAlignment(BaseObject* node,
                                                const std::string& value);
  static SetPropertyResult SetMargin(BaseObject* node,
                                     const std::string& value);
  static SetPropertyResult SetTransform(BaseObject* node,
                                        const std::string& value);
  static SetPropertyResult SetId(BaseObject* node,
                                 const std::string& value);
  static SetPropertyResult SetBackground(BaseObject* node,
                                         const std::string& value);
  static SetPropertyResult SetMinWidth(BaseObject* node,
                                       const std::string& value);
  static SetPropertyResult SetMaxWidth(BaseObject* node,
                                       const std::string& value);
  static SetPropertyResult SetMinHeight(BaseObject* node,
                                        const std::string& value);
  static SetPropertyResult SetMaxHeight(BaseObject* node,
                                        const std::string& value);
  static SetPropertyResult SetIgnoreClip(BaseObject* node,
                                         const std::string& value);
#endif  // GLINT_ENABLE_XML

  // Sets an animated transition on the node. Takes an animation timeline
  // (which is a set of animation segments, or keyframes).
  // Transitions should be 'triggered' to start playing.
  // Automatic transitions (ALPHA, MOVE etc) are triggered automatically
  // when corresponding underlying value changes. The transition makes
  // a snapshot of "before" and "after" values and segments can refer
  // to those values.
  // If a transition is playing, another call to TriggerTransition
  // automatically cancels the playing one and initiates the "before"
  // values into momentous values affected by the cancelled transition -
  // so the animation is "continuous".
  // The node takes ownership of the AnimationTimline.
  void SetTransition(AnimatedTransition type, AnimationTimeline* timeline);
  void TriggerTransition(AnimatedTransition type);

  // Computes minimal necessary size (in node's local
  // coordinate system) that is needed to render the node's content and its
  // children reasonably. "Reasonably" means without catastrophic loss of
  // fidelity or crashes. The calculation should take into account constraint
  // - and scale accordingly. If the node can not fit into constraint, it
  // is normally clipped. The node implements OnComputeRequiredSize virtual
  // method which is called by this method. The OnComputeRequiredSize can
  // return a bigger size then constraint - however, this method will never
  // return a size exceeding constraint - remembering the fact that node
  // should be clipped.
  Size ComputeRequiredSize(Size constraint);

  // Called by parent container node to set the final size
  // and position of the child. The size of 'bounds' is no less then
  // required_size_ - but it can be more. It's up to the node to decide how
  // it will occupy the given (aka 'reserved') bounds.
  void SetLayoutBounds(const Rectangle& bounds);

  // Recursively computes the drawing bounds - inlcuding both layout and
  // transform. The operation is equivalent to drawing, and therefore it
  // uses DrawStack to accumulate transforms and clips - however, it does not
  // draw anything - but accumulates drawing bounds in 'bounds' out parameter.
  // 'dirty_rectangles' is a collection of rectangles that cover the actually
  // changed pixels. While 'bounds' gives the overall bounds of the drawing
  // (so it can be used to allocate a window or underlying bitmap), the
  // 'dirty_rectangles' is a subset of the area in 'bounds' that has to be
  // incrementally redrawn.
  bool ComputeDrawingBounds(DrawStack* stack,
                            Rectangle* bounds,
                            Array<Rectangle>* dirty_rectangles);

  // Draw method. 'stack' has clips and transforms accumulated from
  // the 'screen' up to the node, not including the node's own
  // transform and clip.
  bool Draw(DrawStack* stack);

  // Records a snapshot of the subtree rooted at this Node.  The results
  // are reflected in stack.  On entry and exit, stack contains the
  // snapshot contexts describing all ancestors.
  bool Snapshot(SnapshotStack* stack);

  // Performs Node-specific action when this Node is visited during
  // a snapshot.  On entry, stack contains the snapshot contexts
  // describing all ancestors and this Node.
  virtual bool OnSnapshot(SnapshotStack* stack);

  // Returns transform that maps parent coordinates into local
  // coordinates of this node.
  void GetTransformToLocal(Transform* transform) const;

  // Returns transform that maps local coordinates into parent
  // coordinates of this node,
  void GetTransformToParent(Transform* transform) const;

  // Returns transform that maps local coordinates into specified Ascendant's
  // coordinates. If there is no proper tree connection from this node to the
  // ascendant, 'false' is returned.   Ascendant 'NULL' means map to the
  // screen coordinates.
  bool GetTransformToAscendant(const Node* ascendant,
                               Transform* transform) const;

  // Aggregates and returns a complete transform that maps local
  // coordinates into specified descendant's coordinates. If the treeness
  // is incorrect, 'false' is returned. Descendant should not be NULL.
  bool GetTransformToDescendant(const Node* descendant,
                                Transform* transform) const;

  // Minimum required size to reasonably show the content of the node.
  // Cached result of ComputeRequiredSize.
  Size required_size() const { return required_size_; }

  // Computed size of the node's content, in local coordinate system - at least
  // equal to required_size but may be larger if the parent layout
  // container decides to reserve a larger size then required.
  Size final_size() const { return final_size_; }

  // Returns a clip rectangle, in local coordinates. Usually this clip
  // is computed by layout when layout constraints do not allow entire node to
  // be displayed.
  void GetClipRect(Rectangle* clip) const;

  // Does a recursive pass similar to rendering, using transforms, clips and
  // alpha to collect all the nodes that intersect the given point.
  bool HitTestDeep(DrawStack* stack,
                   Vector point,
                   HitTestChain* chain);

  // layout-computed offset relative to the parent node
  Point local_offset() { return local_offset_; }

  // Effective offset is used in actual rendering/hittesting. Combines
  // layout-computed and animated offsets relative to parent.
  Point effective_offset() const;

  // Effective alpha is used in actual rendering/hittesting. Combines
  // user-set and animated alpha values.
  int effective_alpha() const;

  // The limited use of 'protected'. These methods are never intended for
  // calling by anybody outside the node itself, they are essentially
  // callbacks that specializations of the node supply to do their specific
  // processing.
 protected:
  // Computed the minimum size necessary to render the content of the node
  // in a reasonable way (without clipping it for example), given a
  // constraint. The computed size may be bigger then constraint, indicating
  // that node can not handle rendering into smaller size. In this case, the
  // node's rendering will likely be clipped anyways by some parent container,
  // but at least the node will not be asked to render into smaller size.
  // In case of clip induced by required size being too large, the
  // Vertical/HorizontalAlignment properties define which portion of the node
  // will be visible in the clipped space.
  virtual Size OnComputeRequiredSize(Size constraint);

  // This method is called from SetLayoutBounds. The 'reserved' is the size
  // that parent layout node 'reserves' in its layout for the child. The callee
  // may use all reserved space or part of it. It may not use more then
  // reserved because it may cause undeterministic behavior (for exampe, it
  // may be clipped at reserved size).
  virtual Size OnSetLayoutBounds(Size reserved);

  // Computes the drawing bounds in local coordinate system. Does not recurse
  // or include children.
  virtual void OnComputeDrawingBounds(Rectangle* bounds);

  // Draws the content of the node itself (not calling any children).
  virtual bool OnDraw(DrawStack* stack);

  // Indicates whether or not the default layout includes children in
  // calulation of sizes. Usually, layout containers override this and return
  // 'false' to indicate that they will measure/position their children
  // themselves.
  virtual bool LayoutChildren() { return true; }

  // Called just before the node is disconnected from the tree.
  virtual void OnDisconnect();

  // Detects if the given point in local coordinates hits the geometry of the
  // node. Default implementation detects all points in final_size rect as
  // successful hits.
  virtual bool OnHitTestContent(Point local_point);

  // Returns true iff this Node is to appear in a snapshot.
  virtual bool RequiresSnapshot() { return false; }

  // Returns an unqualified tag name for this Node in the snapshot document
  virtual std::string TagName();

#ifdef DEBUG
 protected:
  const char* type_name_;
#endif

 private:
  Size ComputeRequiredSizeForChildren(Size constraint);
  Size SetLayoutBoundsForChildren(Size reserved);
  void set_clip_at_bounds(bool clip) { clip_at_bounds_ = clip; }
  void DrawBackground(DrawStack* stack);
  bool RemoveChildHelper(Node* child, bool destruct);
  bool RemoveChildrenHelper(bool destruct);
  void TickAnimation(real64 current_time);
  void RemoveAnimation(AnimationTimeline* timeline);

  Node* first_child_;
  Node* prev_sibling_;
  Node* next_sibling_;
  Node* parent_;
  RootUI* parent_ui_;
  std::string id_;

  Array<MessageHandler> message_handlers_;
  AnimationTimeline* transitions_[LAST_TRANSITION];

  Transform transform_;
  Rectangle margin_;
  Size min_size_;
  Size max_size_;
  int alpha_;
  Color background_;

  // Copy of the last return from ComputeRequiredSize, clipped by constraints.
  Size required_size_;
  // Copy of the last return from OnComputeRequiredSize, not clipped by
  // constraints.
  Size internal_required_size_;
  // The offset of the node inside parent-provided bounds_ (additive to
  // transform, computed as part of layout).
  Point local_offset_;
  // Final layout size - copy of the last from OnSetLayoutBounds.
  Size final_size_;
  // Layout-reserved size (no margins, naked layout slot clipped by max_size).
  Size layout_reserved_size_;
  // Optimization - store previous constraint so we can avoid re-layout if
  // not invalidated and the constraint is the same.
  Size previous_constraint_;
  // Optimization - store previous layout bounds set by the parent so we
  // can bail early from the SetLayoutBounds recursive call if the bounds are
  // the same.
  Rectangle previous_layout_bounds_;
  // Drawing bounds, including children - in global coordinates.
  Rectangle computed_drawing_bounds_;
  // Area to draw after changes, in local coordinates.
  Rectangle draw_area_;

  VerticalAlignment vertical_alignment_;
  HorizontalAlignment horizontal_alignment_;

  bool clip_at_bounds_;
  bool layout_invalidated_;
  bool full_redraw_;
  bool is_root_;
  bool ignore_clip_;
  bool has_active_animations_;
  bool previously_rendered_;
  DISALLOW_EVIL_CONSTRUCTORS(Node);
};

}  // namespace glint

#endif  // GLINT_INCLUDE_NODE_H__
