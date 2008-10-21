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

#include "glint/include/node.h"
#include "glint/crossplatform/core_util.h"
#include "glint/include/animation_timeline.h"
#include "glint/include/bitmap.h"
#include "glint/include/draw_stack.h"
#include "glint/include/snapshot_stack.h"
#include "glint/include/root_ui.h"
#include "glint/include/xml_parser.h"

namespace glint {

#ifdef DEBUG
static const char* kDefaultNodeName = "Node";
#endif

Node::Node()
    : first_child_(NULL),
      prev_sibling_(NULL),
      next_sibling_(NULL),
      parent_(NULL),
      parent_ui_(NULL),
      alpha_(colors::kOpaqueAlpha),
      background_(colors::kTransparent),
      vertical_alignment_(Y_FILL),
      horizontal_alignment_(X_FILL),
      clip_at_bounds_(false),
      layout_invalidated_(false),
      full_redraw_(false),
      is_root_(false),
      ignore_clip_(false),
      has_active_animations_(false),
      previously_rendered_(false) {
#ifdef DEBUG
  type_name_ = kDefaultNodeName;
#endif

  required_size_ = internal_required_size_ = final_size_ = min_size_ = Size();
  max_size_ = Size::HugeSize();
  for (int i = 0; i < LAST_TRANSITION; ++i) {
    transitions_[i] = NULL;
  }
}

Node::~Node() {
  RemoveChildren();
  RootUI* root_ui = GetRootUI();
  for (int i = 0; i < LAST_TRANSITION; ++i) {
    AnimationTimeline* timeline = transitions_[i];
    if (root_ui) {
      root_ui->RemovePendingAnimation(timeline);
    }
    delete timeline;
  }
}

void Node::SetParentUI(RootUI* ui) {
  if (parent_ || ui == GetRootUI())
    return;
  is_root_ = (ui != NULL);
  parent_ui_ = ui;
}

bool Node::AddChild(Node* new_child) {
  if (!new_child)
    return false;

  ASSERT(new_child->parent_ == NULL);
  ASSERT(new_child->next_sibling_ == NULL);
  ASSERT(new_child->prev_sibling_ == NULL);
  // if there is at least one child, we should have last too
  ASSERT(!first_child_ || first_child_->prev_sibling_);

  if (!first_child_) {
    first_child_ = new_child->prev_sibling_ = new_child;
  } else {
    new_child->prev_sibling_ = first_child_->prev_sibling_;
    first_child_->prev_sibling_->next_sibling_ = new_child;  // last->next
    first_child_->prev_sibling_ = new_child;  // last
  }

  new_child->next_sibling_ = NULL;
  new_child->parent_ = this;

  Invalidate();
  return true;
}

bool Node::RemoveChildHelper(Node* child, bool destruct) {
  if (!child || child->parent_ != this)
    return false;

  child->OnDisconnect();

  if (child->next_sibling_) {
    child->next_sibling_->prev_sibling_ = child->prev_sibling_;
  } else {
    // last child - change the last child pointer which is a
    // prev_sibling of the first child
    ASSERT(first_child_->prev_sibling_ == child);
    first_child_->prev_sibling_ = child->prev_sibling_;
  }

  // update prev_sibling, unless it's the prev_sibling pointer
  // of the first child, which is re-purposed to keep a pointer
  // to the last_child, since we don't have a separate pointer for that.
  if (child->prev_sibling_ && child->prev_sibling_->next_sibling_) {
    child->prev_sibling_->next_sibling_ = child->next_sibling_;
  }

  if (first_child_ == child) {
    first_child_ = child->next_sibling_;
  }

  if (destruct) {
    delete child;
  } else {
    child->parent_ = NULL;
    child->prev_sibling_ = NULL;
    child->next_sibling_ = NULL;
  }

  Invalidate();
  return true;
}

bool Node::RemoveNode(Node* child) {
  return RemoveChildHelper(child, true);  // with destruction of the child
}

bool Node::DisconnectNode(Node* child) {
  return RemoveChildHelper(child, false);  // no destruction of the child
}

bool Node::RemoveChildrenHelper(bool destruct) {
  Node* child = first_child();
  while (child) {
    Node* next = child->next_sibling();
    child->OnDisconnect();
    if (destruct) {
      delete child;
    } else {
      child->parent_ = NULL;
      child->next_sibling_ = NULL;
      child->prev_sibling_ = NULL;
    }
    child = next;
  }
  first_child_ = NULL;

  Invalidate();
  return true;
}


bool Node::RemoveChildren() {
  return RemoveChildrenHelper(true);
}

bool Node::DisconnectChildren() {
  return RemoveChildrenHelper(false);
}

void Node::OnDisconnect() {
  RootUI* root_ui = GetRootUI();
  if (!root_ui)
    return;

  if (root_ui->capture_node() == this) {
    root_ui->EndMouseCapture();
  }

  if (root_ui->focus_node() == this) {
    root_ui->SetFocus(NULL);
  }
}

// Mark the path to the top of the tree with 'layout_invalidated' to
// be able to recurse through affected subtrees (since in terms of layout,
// a subtree is in need of recalc if it has an invalidated descendant).
// Mark only the actually invalidated node as "directly_invalidated" because
// they will also receive mandatory re-paint (think about button changing
// its appearance on mouse-over). Without this bit, the node will be repainted
// only if it changed size or position (since then result of composition with
// underlying/overlaying nodes may change).
void Node::Invalidate() {
  if (layout_invalidated_)
    return;

  full_redraw_ = true;

  // Propagate the invalidate up the tree.
  // Avoid setting full_redraw_ on ascendants.
  for (Node* p = this; p; p = p->parent_) {
    p->layout_invalidated_ = true;
    if (p->is_root()) {
      RootUI* pUI = p->GetRootUI();
      if (pUI) {
        pUI->Invalidate();
      }
    }
  }
}

void Node::GetTransformToLocal(Transform* transform) const {
  ASSERT(transform);
  transform->Reset();
  Point effective_offset = this->effective_offset();
  int dx = -effective_offset.x;
  int dy = -effective_offset.y;
  transform->AddOffset(Vector(dx, dy));
  Transform inverted;
  inverted.Set(transform_);
  inverted.Invert();
  transform->AddPostTransform(inverted);
}

void Node::GetTransformToParent(Transform* transform) const {
  ASSERT(transform);
  transform->Reset();
  transform->AddPostTransform(transform_);
  Point effective_offset = this->effective_offset();
  int dx = effective_offset.x;
  int dy = effective_offset.y;
  transform->AddOffset(Vector(dx, dy));
}

MessageResultCode Node::HandleMessage(const Message& message) {
  for (int i = 0; i < message_handlers_.length(); ++i) {
    MessageResultCode res = message_handlers_[i]->HandleMessage(message);
    if (res == MESSAGE_HANDLED)
      return res;
  }
  if (has_active_animations_ && message.code == GL_MSG_IDLE) {
    TickAnimation(message.time_stamp);
  }
  return MESSAGE_CONTINUE;
}

bool Node::AddHandler(MessageHandler* handler) {
  if (!handler)
    return false;

  message_handlers_.Add(handler);
  return true;
}

Size Node::ComputeRequiredSize(Size constraint) {
  // If nothing changed and the node nor its subtree is invalidated,
  // simply return the previously-computed result.
  if (!layout_invalidated_ &&
      (constraint == previous_constraint_) &&
      previously_rendered_) {
    return required_size_;
  }
  previous_constraint_ = constraint;
  // Cause SetLayoutBounds to run even if the parent is setting the same bounds
  // and we are not directly invalidated - this ensures both phases of layout
  // (measurememnt and positioning) to be executed if measurement went through.
  // Some nodes (HtmlNode) rely on both phases to finalize layout.
  layout_invalidated_ = true;

  Size original_constraint = constraint;

  // First, strip the margins.
  constraint.width -= margin_.left() + margin_.right();
  constraint.height -= margin_.top() + margin_.bottom();

  // Make sure maximum size is honored by clipping constraint
  constraint.Min(max_size_);
  // Make sure the minimum size is honored (even if the node will be clipped).
  // Note that min_size always wins over max_size.
  constraint.Max(min_size_);
  // Make sure no negative sizes are used.
  constraint.Max(Size());

  // Invoke the virtual method to compute minimum required size of content.
  internal_required_size_ = OnComputeRequiredSize(constraint);

  if (LayoutChildren() && first_child_)
    internal_required_size_.Max(ComputeRequiredSizeForChildren(constraint));

  // At least make the element to be the min, even if it does not want it.
  internal_required_size_.Max(min_size_);
  required_size_ = internal_required_size_;

  // Enforce max size
  required_size_.Min(max_size_);
  // Enforce min size. Again, min size wins over max size.
  required_size_.Max(min_size_);

  // Add the margins back.
  required_size_.width += margin_.left() + margin_.right();
  required_size_.height += margin_.top() + margin_.bottom();

  // Make sure we don't violate constraint (at least this is what parent sees).
  required_size_.Min(original_constraint);

  return required_size_;
}

void Node::SetLayoutBounds(const Rectangle& bounds) {
  // Don't do positioning in the subtree if nothing changes, layout-wise.
  if (!layout_invalidated_ &&
      bounds.IsEqual(previous_layout_bounds_) &&
      previously_rendered_) {
    return;
  }
  previous_layout_bounds_.Set(bounds);

  Rectangle inner_bounds(bounds.left() + margin_.left(),
                         bounds.top() + margin_.top(),
                         bounds.right() - margin_.right(),
                         bounds.bottom() - margin_.bottom());

  // exclude negative sizes
  if (inner_bounds.left() > inner_bounds.right()) {
    inner_bounds.set_right(inner_bounds.left());
  }
  if (inner_bounds.top() > inner_bounds.bottom()) {
    inner_bounds.set_bottom(inner_bounds.top());
  }

  Size local_size = inner_bounds.size();
  HorizontalAlignment alignment_x = horizontal_alignment_;
  VerticalAlignment alignment_y = vertical_alignment_;

  Size child_space = local_size;

  // cache the margin-less layout-reserved size.
  layout_reserved_size_ = child_space;

  // if we are not filling, only use the required size, not the
  // whole space reserved by layout parent
  if (alignment_x != X_FILL) {
    child_space.width = required_size_.width - margin_.left() - margin_.right();
  }
  if (alignment_y != Y_FILL) {
    child_space.height = required_size_.height -
                         margin_.top() -
                         margin_.bottom();
  }

  child_space.Min(max_size_);
  // min size always wins over max size, so apply it after
  child_space.Max(min_size_);

  // if the internal required size (minimum required by element's content)
  // is larger then (potentially constrained) reserved space -
  // give the element what it wants and clip it later. Also
  // make sure the alignment changes to top-left (because top left is
  // the most reasonable portion of a clipped node to see)
  set_clip_at_bounds(false);

  // internal_required_size_ is without margins
  if (internal_required_size_.width > child_space.width) {
    child_space.width = internal_required_size_.width;
    set_clip_at_bounds(true);
    alignment_x = X_LEFT;
  }
  if (internal_required_size_.height > child_space.height) {
    child_space.height = internal_required_size_.height;
    set_clip_at_bounds(true);
    alignment_y = Y_TOP;
  }

  // Invoke the virtual. Give the child a space reserved for it.
  // Get back the size actually consumed by the child.
  Size content_size = OnSetLayoutBounds(child_space);

  // Fix up the final computed size. Don't give more then layout has reserved.
  content_size.Min(child_space);
  // Also, make sure the final size is at least the min as specified by
  content_size.Max(min_size_);

  if (LayoutChildren() && first_child_)
    content_size.Max(SetLayoutBoundsForChildren(child_space));

  Point new_local_offset;
  // set up local offset, taking into account margin,
  // and how 2 reserved space rectangle and actually occupied
  // rectangle overlap due to alignments.
  new_local_offset.x = inner_bounds.left();
  if (alignment_x == X_RIGHT) {
    new_local_offset.x += local_size.width - content_size.width;
  } else if (alignment_x != X_LEFT) {  // center or fill
    new_local_offset.x += (local_size.width - content_size.width) / 2;
  }

  new_local_offset.y = inner_bounds.top();
  if (alignment_y == Y_BOTTOM) {
    new_local_offset.y += local_size.height - content_size.height;
  } else if (alignment_y != Y_TOP) {  // center or fill
    new_local_offset.y += (local_size.height - content_size.height) / 2;
  }

  // If the node changes size or position,
  // invalidate both previously occupied rectangle and the new one.
  bool size_changed = (final_size_ != content_size);
  bool position_changed = (new_local_offset != local_offset_);

  if (size_changed) {
    final_size_ = content_size;
  }

  if (position_changed) {
    // Node moves if it was on the screen before.
    // TODO(dimich): add "reveal" and "discard" as triggers
    if (previously_rendered_) {
      TriggerTransition(MOVE_TRANSITION);  // make a snapshot of old offset
    }
    local_offset_ = new_local_offset;    // now, update the offset
  }

  // If the node went through layout and the final_size has changed,
  // the node has to be re-rendered. If the node was not directly invalidated
  // (by calling Node::Invalidate()) or didn't change size as result of its
  // other nodes changing around, and it still has to be re-rendered, then
  // either there is a bug or the node has to call InvalidateDrawing correctly
  // from its OnSetLayoutBounds.
  if (size_changed || position_changed) {
    full_redraw_ = true;
  }
  layout_invalidated_ = false;
}

// TODO(dimich): in case of non-top-left alignment and clip_to_bounds
// activated, we should add the offset that will move clip in local space.
void Node::GetClipRect(Rectangle* clip) const {
  ASSERT(clip);
  clip->SetHuge();
  // Node can be clipped by layout if it is bigger then space
  // reserved by layout for it, or max_size clips it.
  // Clip is in local coordinates of the node.
  if (clip_at_bounds() && !ignore_clip_) {
    Size visible_size = layout_reserved_size_;
    visible_size.Min(max_size_);
    visible_size.Max(min_size_);
    clip->Set(Point(), visible_size);
  }
}

bool Node::Draw(DrawStack* stack) {
  if (!stack)
    return false;

  full_redraw_ = false;

  Rectangle clip;
  GetClipRect(&clip);

  Transform to_parent;
  GetTransformToParent(&to_parent);

  if (!stack->Push(to_parent,
                   clip,
                   ignore_clip_,
                   effective_alpha()))
    return false;

  bool success = true;

  // Check if the combined drawing bounds of this node and its children
  // are inside the clip. If not - skip the subtree altogether.
  DrawContext* draw_context = stack->Top();
  Rectangle drawing_bounds;
  drawing_bounds.Set(computed_drawing_bounds_);
  // Clip is in global coordinates.
  drawing_bounds.Intersect(draw_context->clip);
  if (!drawing_bounds.IsEmpty()) {
    DrawBackground(stack);

    // Draw itself, then children
    success = OnDraw(stack);
    if (success) {
      for (Node* child = first_child();
           child && success;
           child = child->next_sibling()) {
        success = child->Draw(stack);
      }
    }
    previously_rendered_ = true;
  }

  stack->Pop();
  return success;
}

void Node::DrawBackground(DrawStack* stack) {
  if (final_size_.width <= 0 ||
      final_size_.height <= 0 ||
      background_.alpha() == colors::kTransparentAlpha) {
    return;
  }

  Bitmap* target = stack->target();
  DrawContext* draw_context = stack->Top();

  bool translate_only = draw_context->transform_to_global.IsOffsetOnly();

  // Optimize for translate only - no need to do bilinear interpolation then.
  if (translate_only) {
    if (draw_context->clip.IsEmpty())
      return;

    Vector offset = draw_context->transform_to_global.GetOffset();
    Point left_top(static_cast<int>(offset.x),
                   static_cast<int>(offset.y));

    Rectangle background_rect(left_top.x,
                              left_top.y,
                              left_top.x + final_size_.width,
                              left_top.y + final_size_.height);

    background_rect.Intersect(draw_context->clip);

    // Premultiply the background color and alpha with context's alpha
    target->DrawRectangle(background_rect, background_, draw_context->alpha);
  } else {
    // Rotation etc.
    Bitmap local_bitmap(Point(), final_size_);
    local_bitmap.Fill(background_);
    Rectangle source_clip;
    source_clip.SetHuge();
    target->Compose(local_bitmap,
                    source_clip,
                    draw_context->transform_to_global,
                    draw_context->clip,
                    draw_context->alpha);
  }
}

// Recursively computes the drawing bounds - including both layout and
// transform.
bool Node::ComputeDrawingBounds(DrawStack* stack,
                                Rectangle* bounds,
                                Array<Rectangle>* dirty_rectangles) {
  if (!stack || !bounds || !dirty_rectangles)
    return false;

  // Prepare drawing context
  Rectangle clip;
  GetClipRect(&clip);

  Transform to_parent;
  GetTransformToParent(&to_parent);

  if (!stack->Push(to_parent,
                   clip,
                   ignore_clip_,
                   effective_alpha()))
    return false;
  DrawContext* context = stack->Top();

  // Now, see if previous bounds of this node should be re-rendered
  // If so, invalidate both previous drawing bounds (here) and new ones (below).
  if (full_redraw_ && !computed_drawing_bounds_.IsEmpty()) {
    Rectangle* invalid_rect = new Rectangle();
    invalid_rect->Set(computed_drawing_bounds_);
    dirty_rectangles->Add(invalid_rect);
  }

  // Compute local bounds - background and content.
  Rectangle local_bounds;
  OnComputeDrawingBounds(&local_bounds);
  if (background_.alpha() != colors::kTransparentAlpha) {
    Rectangle background_area(Point(), final_size_);
    local_bounds.Union(background_area);
  }
  local_bounds.Intersect(clip);

  // Transform local bounds to global space.
  context->transform_to_global.TransformRectangle(local_bounds,
                                                  &computed_drawing_bounds_);
  computed_drawing_bounds_.Intersect(context->clip);

  // Now, recurse into children
  Node* child = first_child();
  while (child) {
    if (!child->ComputeDrawingBounds(stack,
                                     &computed_drawing_bounds_,
                                     dirty_rectangles))
      return false;
    child = child->next_sibling();
  }

  // Invalidate the new bounds.
  if (full_redraw_ && !computed_drawing_bounds_.IsEmpty()) {
    Rectangle* invalid_rect = new Rectangle();
    invalid_rect->Set(computed_drawing_bounds_);
    dirty_rectangles->Add(invalid_rect);
  }

  // Now see if the node needs to repaint if not full_redraw.
  if (!draw_area_.IsEmpty()) {
    if (!full_redraw_) {
      Rectangle* invalid_rect = new Rectangle();
      context->transform_to_global.TransformRectangle(draw_area_, invalid_rect);
      dirty_rectangles->Add(invalid_rect);
    }
    draw_area_.Reset();
  }
  // Union our global contribution into accumulating global bounds.
  bounds->Union(computed_drawing_bounds_);

  stack->Pop();
  return true;
}

std::string Node::TagName() {
  return std::string("unknown");
}

bool Node::Snapshot(SnapshotStack* stack) {
  if (!stack)
    return false;

  bool success;

  Transform to_parent;
  GetTransformToParent(&to_parent);

  if (RequiresSnapshot()) {
    // Add new snapshot context for current Node
    stack->Push(to_parent, TagName());
    // Perform actions for this Node
    success = OnSnapshot(stack);
  } else {
    // Add new snapshot context for current Node.
    // In this case, no new element is created for this Node.
    stack->Push(to_parent);
    success = true;
  }
  if (success) {
    // Recurse subtree
    for (Node* child = first_child();
         child && success;
         child = child->next_sibling()) {
      success = child->Snapshot(stack);
    }
  }
  // Remove the context we added earlier
  stack->Pop();
  return success;
}

bool Node::OnSnapshot(SnapshotStack* stack) {
  // Adds four attributes to describe the coordinates and dimensions of this
  // Node.

  SnapshotContext* snapshot_context = stack->Top();
  ASSERT(snapshot_context);

  Transform transform;
  transform.Set(snapshot_context->transform());
  Vector offset = transform.GetOffset();
  snapshot_context->AddAttribute("x", static_cast<int>(offset.x));
  snapshot_context->AddAttribute("y", static_cast<int>(offset.y));

  Rectangle bounds(Point(), final_size_);
  Rectangle global_bounds;
  transform.Invert();
  transform.TransformRectangle(bounds, &global_bounds);

  snapshot_context->AddAttribute("w", bounds.size().width);
  snapshot_context->AddAttribute("h", bounds.size().height);

  snapshot_context->AddAttribute("alpha", effective_alpha());
  return true;
}

bool Node::HitTestDeep(DrawStack* stack,
                       Vector point,
                       HitTestChain* chain) {
  ASSERT(stack && chain);
  if (!stack || !chain)
    return false;

  // Prepare drawing context
  Rectangle clip;
  GetClipRect(&clip);

  Transform to_parent;
  GetTransformToParent(&to_parent);
  if (!stack->Push(to_parent, clip, ignore_clip(), effective_alpha()))
    return false;
  DrawContext* context = stack->Top();

  // Nothing will be rendered by this subtree - no hit test either.
  // Note: this also skips the "ignore_clip" nodes below in the tree.
  if (context->clip.IsEmpty() || context->alpha == 0) {
    stack->Pop();
    return true;
  }

  Vector local_point = context->transform_to_local.TransformVector(point);
  if (OnHitTestContent(Point(Round(local_point.x), Round(local_point.y)))) {
    HitTestResult* result = new HitTestResult();
    if (!result)
      return false;
    result->node = this;
    result->local_point = local_point;
    chain->Add(result);
  }

  Node* child = first_child();
  while (child) {
    if (!child->HitTestDeep(stack, point, chain))
      return false;
    child = child->next_sibling();
  }

  stack->Pop();
  return true;
}

Size Node::ComputeRequiredSizeForChildren(Size constraint) {
  Size result;  // content has zero size in the base class

  Node* child = first_child();
  while (child) {
    Size s = child->ComputeRequiredSize(constraint);
    result.Max(s);
    child = child->next_sibling();
  }

  return result;
}

// Base Node - size of content is (0,0)
Size Node::OnComputeRequiredSize(Size constraint) {
  return Size();
}

Size Node::SetLayoutBoundsForChildren(Size reserved) {
  // this is the simplest way - just agree with the layout parent,
  // and pass the same size to all children.
  Node* child = first_child();
  while (child) {
    Rectangle child_bounds(Point(), reserved);
    child->SetLayoutBounds(child_bounds);
    child = child->next_sibling();
  }
  return reserved;
}

// Base Node - size of content is (0,0)
Size Node::OnSetLayoutBounds(Size reserved) {
  if (background_.alpha() != colors::kTransparentAlpha)
    return reserved;
  else
    return Size();
}

bool Node::OnDraw(DrawStack* stack) {
  // draw nothing - this is empty node
  return true;
}

void Node::OnComputeDrawingBounds(Rectangle* bounds) {
  ASSERT(bounds);
  // Base Node has no content.
  bounds->Reset();
}

bool Node::OnHitTestContent(Point local_point) {
  Rectangle hit_area;
  OnComputeDrawingBounds(&hit_area);
  Rectangle bounds(Point(), final_size_);
  hit_area.Union(bounds);

  Rectangle clip;
  GetClipRect(&clip);
  hit_area.Intersect(clip);

  if (hit_area.IsEmpty())
    return false;

  return local_point.x >= hit_area.left() &&
         local_point.y >= hit_area.top() &&
         local_point.x <  hit_area.right()  &&
         local_point.y <  hit_area.bottom();
}

bool Node::GetTransformToAscendant(const Node* ascendant,
                                   Transform* transform) const {
  if (!transform)
    return false;

  transform->Reset();
  const Node* node = this;

  while (node != ascendant) {
    Transform to_parent;
    node->GetTransformToParent(&to_parent);
    transform->AddPostTransform(to_parent);
    node = node->parent();
    if (!node && ascendant)
      return false;
  }
  return true;
}

bool Node::GetTransformToDescendant(const Node* descendant,
                                    Transform* transform) const {
  if (!descendant || !transform)
    return false;

  if (!descendant->GetTransformToAscendant(this, transform))
    return false;

  transform->Invert();
  return true;
}

void Node::SetTransition(AnimatedTransition type,
                         AnimationTimeline* timeline) {
  if (type < 0 || type >= LAST_TRANSITION)
    return;
  RemoveAnimation(transitions_[type]);
  transitions_[type] = timeline;
}

void Node::TriggerTransition(AnimatedTransition type) {
  if (type < 0 || type >= LAST_TRANSITION)
    return;

  AnimationTimeline* timeline = transitions_[type];
  if (timeline) {
    timeline->Initialize(this);
    has_active_animations_ = true;
  }
}

static Node* FindNodeByIdHelper(Node* node, const std::string& id) {
  if (!node)
    return NULL;

  if (node->id() == id)
    return node;

  for (Node* child = node->first_child();
       child;
       child = child->next_sibling()) {
    Node* node = FindNodeByIdHelper(child, id);
    if (node)
      return node;
  }
  return NULL;
}

Node* Node::FindNodeById(const std::string& id) {
  return FindNodeByIdHelper(this, id);
}

// XML Support. A bunch of static property parsers and setters.
#ifdef GLINT_ENABLE_XML

SetPropertyResult Node::SetAlpha(BaseObject* node,
                                 const std::string& value) {
  ASSERT(node);
  int alpha = 255;
  if (!XmlParser::StringToInt(value, &alpha))
    return PROPERTY_HAS_INCORRECT_FORMAT;
  if (alpha < 0 || alpha > 255)
    return PROPERTY_BAD_VALUE;
  static_cast<Node*>(node)->alpha_ = alpha;
  return PROPERTY_OK;
}

SetPropertyResult Node::SetChild(BaseObject* node,
                                 BaseObject* value) {
  ASSERT(node);
  if (static_cast<Node*>(node)->AddChild(static_cast<Node*>(value)))
    return PROPERTY_OK;
  else
    return PROPERTY_NOT_SET;
}

struct AlignmentXmlNames {
  const char* name;
  int alignment;
};

static AlignmentXmlNames horizontal_names[] = {
    { "left",   X_LEFT },
    { "right",  X_RIGHT },
    { "center", X_CENTER },
    { "fill",   X_FILL },
};

static AlignmentXmlNames vertical_names[] = {
    { "top",    Y_TOP },
    { "bottom", Y_BOTTOM },
    { "center", Y_CENTER },
    { "fill",   Y_FILL },
};

SetPropertyResult Node::SetHorizontalAlignment(BaseObject* node,
                                               const std::string& value) {
  ASSERT(node);
  for (size_t i = 0;
       i < sizeof(horizontal_names) / sizeof(horizontal_names[0]);
       ++i) {
    if (value.compare(horizontal_names[i].name) == 0) {
      HorizontalAlignment alignment = static_cast<HorizontalAlignment>(
          horizontal_names[i].alignment);
      static_cast<Node*>(node)->horizontal_alignment_ = alignment;
      return PROPERTY_OK;
    }
  }
  return PROPERTY_BAD_VALUE;
}

SetPropertyResult Node::SetVerticalAlignment(BaseObject* node,
                                             const std::string& value) {
  ASSERT(node);
  for (size_t i = 0;
       i < sizeof(vertical_names) / sizeof(vertical_names[0]);
       ++i) {
    if (value.compare(vertical_names[i].name) == 0) {
      VerticalAlignment alignment = static_cast<VerticalAlignment>(
          vertical_names[i].alignment);
      static_cast<Node*>(node)->vertical_alignment_ = alignment;
      return PROPERTY_OK;
    }
  }
  return PROPERTY_BAD_VALUE;
}

SetPropertyResult Node::SetMargin(BaseObject* node,
                                  const std::string& value) {
  ASSERT(node);
  Rectangle margin;
  if (!XmlParser::StringToRect(value, &margin))
    return PROPERTY_HAS_INCORRECT_FORMAT;
  static_cast<Node*>(node)->margin_.Set(margin);
  return PROPERTY_OK;
}

SetPropertyResult Node::SetTransform(BaseObject* node,
                                     const std::string& value) {
  ASSERT(node);
  Transform transform;
  if (!XmlParser::StringToTransform(value, &transform))
    return PROPERTY_HAS_INCORRECT_FORMAT;
  static_cast<Node*>(node)->transform_.Set(transform);
  return PROPERTY_OK;
}

SetPropertyResult Node::SetId(BaseObject* node,
                              const std::string& value) {
  ASSERT(node);
  if (value.empty())
    return PROPERTY_BAD_VALUE;
  static_cast<Node*>(node)->id_ = value;
  return PROPERTY_OK;
}

SetPropertyResult Node::SetBackground(BaseObject* node,
                                      const std::string& value) {
  ASSERT(node);
  Color color(0x00000000);
  if (!XmlParser::StringToColor(value, &color))
    return PROPERTY_HAS_INCORRECT_FORMAT;

  static_cast<Node*>(node)->background_ = color;
  return PROPERTY_OK;
}

SetPropertyResult Node::SetSizeFromString(const std::string& value,
                                          int* size) {
  ASSERT(size);
  int local_size;
  if (!XmlParser::StringToInt(value, &local_size))
    return PROPERTY_HAS_INCORRECT_FORMAT;
  if (local_size < 0)
    return PROPERTY_BAD_VALUE;
  *size = local_size;
  return PROPERTY_OK;
}

SetPropertyResult Node::SetMinWidth(BaseObject* node,
                                    const std::string& value) {
  ASSERT(node);
  return SetSizeFromString(value,
                           &(static_cast<Node*>(node)->min_size_.width));
}

SetPropertyResult Node::SetMaxWidth(BaseObject* node,
                                    const std::string& value) {
  ASSERT(node);
  return SetSizeFromString(value,
                           &(static_cast<Node*>(node)->max_size_.width));
}

SetPropertyResult Node::SetMinHeight(BaseObject* node,
                                     const std::string& value) {
  ASSERT(node);
  return SetSizeFromString(value,
                           &(static_cast<Node*>(node)->min_size_.height));
}

SetPropertyResult Node::SetMaxHeight(BaseObject* node,
                                     const std::string& value) {
  ASSERT(node);
  return SetSizeFromString(value,
                           &(static_cast<Node*>(node)->max_size_.height));
}

SetPropertyResult Node::SetIgnoreClip(BaseObject* node,
                                      const std::string& value) {
  ASSERT(node);
  bool ignore_clip;
  if (!XmlParser::StringToBool(value, &ignore_clip))
    return PROPERTY_HAS_INCORRECT_FORMAT;
  static_cast<Node*>(node)->ignore_clip_ = ignore_clip;
  return PROPERTY_OK;
}

#endif  // GLINT_ENABLE_XML

void Node::RemoveAnimation(AnimationTimeline* timeline) {
  if (!timeline || timeline->owner() != this)
    return;
  if (timeline->state() == AnimationTimeline::PLAYING) {
    timeline->Cancel(false);
  }

  RootUI* root_ui = GetRootUI();
  if (root_ui) {
    root_ui->RemovePendingAnimation(timeline);
  }
  delete timeline;
}

void Node::TickAnimation(real64 current_time) {
  bool has_active = false;
  for (int i = 0; i < LAST_TRANSITION; ++i) {
    AnimationTimeline* animation = transitions_[i];
    // If animation is IDLE or INITIALIZED and waiting in pending list
    // on RootUI - skip it
    if (!animation)
      continue;

    if (animation->state() == AnimationTimeline::PLAYING) {
      animation->Advance(current_time);
    }

    has_active = has_active ||
                 animation->state() != AnimationTimeline::IDLE;
  }
  has_active_animations_ = has_active;
}

void Node::InvalidateDrawing(const Rectangle& rectangle) {
  draw_area_.Union(rectangle);
  RootUI* root_ui = GetRootUI();
  if (!root_ui)
    return;
  root_ui->Invalidate();
}

Point Node::effective_offset() const {
  Point animated_offset = local_offset_;
  for (int i = 0; i < LAST_TRANSITION; ++i) {
    AnimationTimeline* animation = transitions_[i];
    // If animation is IDLE or INITIALIZED and waiting in pending list
    // on RootUI - skip it
    if (animation &&
        animation->state() == AnimationTimeline::PLAYING) {
      Point offset = animation->animated_offset();
      animated_offset.x += offset.x;
      animated_offset.y += offset.y;
    }
  }

  return animated_offset;
}

int Node::effective_alpha() const {
  int animated_alpha = 255;
  bool alpha_is_animated = false;

  for (int i = 0; i < LAST_TRANSITION; ++i) {
    AnimationTimeline* animation = transitions_[i];
    // If animation is IDLE or INITIALIZED and waiting in pending list
    // on RootUI - skip it
    if (animation &&
        animation->state() == AnimationTimeline::PLAYING) {
      int alpha = animation->animated_alpha();
      if (alpha >= 0 && alpha <= 255) {
        alpha_is_animated = true;
        animated_alpha = (animated_alpha * (alpha + 1)) >> 8;
      }
    }
  }

  return alpha_is_animated ? animated_alpha : alpha_;
}

}  // namespace glint
