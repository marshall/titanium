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

// Balloon collection is a queue of balloons with notifications currently
// displayed on the main screen. The notifications expire with time and
// disappear, at which moment they are removed from the collection.

#ifdef OFFICIAL_BUILD
  // The notification API has not been finalized for official builds.
#else
#include <string>
#include <vector>
#include <assert.h>

#include "gears/notifier/balloons.h"
#include "gears/base/common/security_model.h"
#include "gears/base/common/string_utils.h"
#include "gears/base/common/timed_call.h"
#include "gears/notifier/system.h"
#include "third_party/glint/include/animation_timeline.h"
#include "third_party/glint/include/button.h"
#include "third_party/glint/include/color.h"
#include "third_party/glint/include/column.h"
#include "third_party/glint/include/current_time.h"
#include "third_party/glint/include/image_node.h"
#include "third_party/glint/include/interpolation.h"
#include "third_party/glint/include/message.h"
#include "third_party/glint/include/nine_grid.h"
#include "third_party/glint/include/node.h"
#include "third_party/glint/include/platform.h"
#include "third_party/glint/include/point.h"
#include "third_party/glint/include/rectangle.h"
#include "third_party/glint/include/root_ui.h"
#include "third_party/glint/include/row.h"
#include "third_party/glint/include/simple_text.h"
#include "third_party/glint/include/size.h"

static const char *kTitleId = "title_text";
static const char *kSubtitleId = "subtitle_text";
static const char *kDescriptionId = "description_text";
static const char *kBalloonContainer = "balloon_container";
static const char *kActionsContainer = "actions_container";
static const char *kIconId = "icon";
static const char *kCloseButton = "close_button";

#ifdef OS_MACOSX
static const char *kFontName = "Helvetica";
static const int kTitleFontSize = 12;
static const int kSubtitleFontSize = 10;
static const int kDescriptionFontSize = 10;
#else
static const char *kFontName = "Verdana";
static const int kTitleFontSize = 10;
static const int kSubtitleFontSize = 9;
static const int kDescriptionFontSize = 9;
#endif

const double kAlphaTransitionDuration = 1.0;
const double kAlphaTransitionDurationShort = 0.3;
const double kAlphaTransitionDurationForRestore = 0.2;
const double kMoveTransitionDuration = 0.325;
const double kExpirationTime = 7.0;
const double kExpirationTimeSlice = 0.25;

const int kOneHourSeconds = 60 * 60;

const int kUserMessageChangeFont = glint::GL_MSG_USER + 1;

// Portion of the screen allotted for notifications.
// When notification balloons extend over this threshold, no new notifications
// are shown until some notifications are expired/closed.
const double kNotificationFillFactor = 0.7;

enum kContextMenuCommands {
  SEPARATOR,
  SHOW_PREFERENCES,
  DISABLE_NOTIFICATIONS_FOR_1_HOUR,
};

// TODO(dimich): make this internationalized. Pull the strings from resource.
static const System::MenuItem kDefaultContextMenuItems[] = {
  { "Don't show notifications for 1 hour",
    DISABLE_NOTIFICATIONS_FOR_1_HOUR,
    true,
    false },
#ifdef OS_MACOSX
  { "-", SEPARATOR, false, false },
  { "Preferences...", SHOW_PREFERENCES, true, false },
#endif  // OS_MACOSX
};
static const int kContextMenuItemOffset = 100;
static const char16 *kSnoozeAction = STRING16(L"snooze");
static const int kDefaultSnoozeSeconds = 5 * 60;    // 5m

// Container for balloons. Implements several static methods that provide
// balloon metrics scaled properly in case system has "large fonts" support.
// Also implements a few non-static members to perform layout on its child
// nodes that are assumed to be individual baloons.
class BalloonContainer : public glint::Node {
 public:
  BalloonContainer();

  // Refresh the work area and balloon placement.
  void OnDisplaySettingsChanged();

  static int min_balloon_width();
  static int max_balloon_width();
  static int min_balloon_height();
  static int max_balloon_height();

  static glint::Size work_area_size();

  // Scale the size to count in the system font factor.
  static int ScaleSize(int size);

  // Refresh the cached values for work area and drawing metrics (i.e scale
  // factor when 'large ). This is done automatically first time and the
  // application should call this method to re-acquire metrics after
  // resolution and settings change.
  //
  // Return true if and only if a metric changed.
  static bool RefreshSystemMetrics();

 protected:
  // Instructs base class to delegate children layout to this class.
  virtual bool LayoutChildren() { return false; }
  virtual glint::Size OnComputeRequiredSize(glint::Size constraint);
  virtual glint::Size OnSetLayoutBounds(glint::Size reserved);

 private:
  enum BalloonPlacementEnum {
    HORIZONTALLY_FROM_BOTTOM_LEFT,
    HORIZONTALLY_FROM_BOTTOM_RIGHT,
    VERTICALLY_FROM_TOP_RIGHT,
    VERTICALLY_FROM_BOTTOM_RIGHT
  };

  // Minimum and maximum size of balloon
  const static int kBalloonMinWidth = 300;
  const static int kBalloonMaxWidth = 300;
  const static int kBalloonMinHeight = 70;
  const static int kBalloonMaxHeight = 120;

  static glint::Point GetOrigin();
  // Compute the position for the next balloon. Modifies current origin.
  static glint::Point NextPosition(glint::Size balloon_size,
                                   glint::Point *origin);

  static BalloonPlacementEnum placement_;
  static glint::Rectangle work_area_;
  static double font_scale_factor_;
  static bool initialized_;
  DISALLOW_EVIL_CONSTRUCTORS(BalloonContainer);
};

#ifdef WIN32
BalloonContainer::BalloonPlacementEnum BalloonContainer::placement_ =
    BalloonContainer::VERTICALLY_FROM_BOTTOM_RIGHT;
#else
BalloonContainer::BalloonPlacementEnum BalloonContainer::placement_ =
    BalloonContainer::VERTICALLY_FROM_TOP_RIGHT;
#endif  // WIN32

glint::Rectangle BalloonContainer::work_area_;
double BalloonContainer::font_scale_factor_ = 1.0;
bool BalloonContainer::initialized_ = false;

class DisplayChangedHandler : public glint::MessageHandler {
 public:
  DisplayChangedHandler(BalloonContainer *balloon_container)
      : balloon_container_(balloon_container) {
    assert(balloon_container);
  }

  glint::MessageResultCode HandleMessage(const glint::Message &message) {
    switch (message.code) {
      case glint::GL_MSG_DISPLAY_SETTINGS_CHANGED:
        balloon_container_->OnDisplaySettingsChanged();
        break;
      default:
        // We don't care about other messages.
        break;
    }
    return glint::MESSAGE_CONTINUE;
  }

 private:
  BalloonContainer *balloon_container_;
  DISALLOW_EVIL_CONSTRUCTORS(DisplayChangedHandler);
};

BalloonContainer::BalloonContainer() {
  if (!initialized_) {
    initialized_ = true;
    RefreshSystemMetrics();
  }
  AddHandler(new DisplayChangedHandler(this));
}

glint::Size BalloonContainer::work_area_size() {
  return work_area_.size();
}

// Scale the size to count in the system font factor
int BalloonContainer::ScaleSize(int size) {
  return static_cast<int>(size * font_scale_factor_);
}

int BalloonContainer::min_balloon_width() {
  return ScaleSize(kBalloonMinWidth);
}

int BalloonContainer::max_balloon_width() {
  return ScaleSize(kBalloonMaxWidth);
}

int BalloonContainer::min_balloon_height() {
  return ScaleSize(kBalloonMinHeight);
}

int BalloonContainer::max_balloon_height() {
  return ScaleSize(kBalloonMaxHeight);
}

bool BalloonContainer::RefreshSystemMetrics() {
  bool changed = false;

  glint::Rectangle new_work_area;
  new_work_area.Set(work_area_);
  System::GetMainScreenWorkArea(&new_work_area);
  if (!work_area_.IsEqual(new_work_area)) {
    work_area_.Set(new_work_area);
    changed = true;
  }

  double new_font_scale_factor = font_scale_factor_;
  new_font_scale_factor = System::GetSystemFontScaleFactor();
  if (font_scale_factor_ != new_font_scale_factor) {
    font_scale_factor_ = new_font_scale_factor;
    changed = true;
  }
  return changed;
}

glint::Point BalloonContainer::GetOrigin() {
  glint::Point origin;

  switch (placement_) {
    case HORIZONTALLY_FROM_BOTTOM_LEFT:
      origin.x = work_area_.left();
      origin.y = work_area_.bottom();
      break;
    case HORIZONTALLY_FROM_BOTTOM_RIGHT:
      origin.x = work_area_.right();
      origin.y = work_area_.bottom();
      break;
    case VERTICALLY_FROM_TOP_RIGHT:
      origin.x = work_area_.right();
      origin.y = work_area_.top();
      break;
    case VERTICALLY_FROM_BOTTOM_RIGHT:
      origin.x = work_area_.right();
      origin.y = work_area_.bottom();
      break;
    default:
      assert(false);
      break;
  }
  return origin;
}

glint::Point BalloonContainer::NextPosition(glint::Size balloon_size,
                                            glint::Point *origin) {
  assert(origin);
  glint::Point result;

  switch (placement_) {
    case HORIZONTALLY_FROM_BOTTOM_LEFT:
      result.x = origin->x;
      result.y = origin->y - balloon_size.height;
      origin->x += balloon_size.width;
      break;
    case HORIZONTALLY_FROM_BOTTOM_RIGHT:
      origin->x -= balloon_size.width;
      result.x = origin->x;
      result.y = origin->y - balloon_size.height;
      break;
    case VERTICALLY_FROM_TOP_RIGHT:
      result.x = origin->x - balloon_size.width;
      result.y = origin->y;
      origin->y += balloon_size.height;
      break;
    case VERTICALLY_FROM_BOTTOM_RIGHT:
      origin->y -= balloon_size.height;
      result.x = origin->x - balloon_size.width;
      result.y = origin->y;
      break;
    default:
      assert(false);
      break;
  }
  return result;
}

glint::Size BalloonContainer::OnComputeRequiredSize(glint::Size constraint) {
  glint::Rectangle screen_bounds;
  System::GetMainScreenWorkArea(&screen_bounds);
  if (screen_bounds.IsEmpty())
    return glint::Size();

  glint::Size container_size = screen_bounds.size();
  // Let children compute their size
  for (glint::Node *child = first_child(); child;
       child = child->next_sibling()) {
    // Re-set min-max sizes on the balloons. They can change if resolution or
    // font size changes dynamically. Also broadcast 'font change' message
    // so the text nodes in the balloon can update their font size.
    glint::Message message;
    message.code = static_cast<glint::GlintMessages>(kUserMessageChangeFont);
    message.user_data = this;
    glint::RootUI::BroadcastMessage(child, message);

    child->set_min_width(min_balloon_width());
    child->set_min_height(min_balloon_height());
    child->set_max_width(max_balloon_width());
    child->set_max_height(max_balloon_height());

    glint::Size child_size = child->ComputeRequiredSize(container_size);
  }
  return container_size;
}

// The parent container reserved a space for us - at least the amount returned
// from OnComputeRequiredSize. Now we should iterate over the children and give
// them new layout bounds.
glint::Size BalloonContainer::OnSetLayoutBounds(glint::Size reserved) {
  glint::Point origin = GetOrigin();

  for (glint::Node *child = first_child(); child;
       child = child->next_sibling()) {
    glint::Point position = NextPosition(child->required_size(), &origin);
    glint::Rectangle child_location(position.x,
                                    position.y,
                                    position.x + child->required_size().width,
                                    position.y + child->required_size().height);
    // Position the child.
    child->SetLayoutBounds(child_location);
  }
  return reserved;
}

void BalloonContainer::OnDisplaySettingsChanged() {
  if (RefreshSystemMetrics()) {
    Invalidate();
  }
}

class ChangeFontHandler : public glint::MessageHandler {
 public:
  ChangeFontHandler(glint::SimpleText *owner, int base_font_size)
    : owner_(owner),
      base_font_size_(base_font_size) {
    assert(owner);
    assert(base_font_size > 0);
  }

  glint::MessageResultCode HandleMessage(const glint::Message &message) {
    switch (message.code) {
      case kUserMessageChangeFont: {
          BalloonContainer *container =
              reinterpret_cast<BalloonContainer*>(message.user_data);
          assert(container);
          owner_->set_font_size(container->ScaleSize(base_font_size_));
        }
        break;
      default:
        // We don't care about other messages.
        break;
    }
    return glint::MESSAGE_CONTINUE;
  }

 private:
  glint::SimpleText *owner_;
  int base_font_size_;
  DISALLOW_EVIL_CONSTRUCTORS(ChangeFontHandler);
};

class TimerHandler : public glint::MessageHandler {
 public:
  TimerHandler(BalloonCollection *collection)
    : collection_(collection) {
    assert(collection);
    time_stamp_ = glint::CurrentTime::Seconds();
  }

  glint::MessageResultCode HandleMessage(const glint::Message &message) {
    if (message.code == glint::GL_MSG_IDLE) {
      double current_time = glint::CurrentTime::Seconds();
      if (current_time - time_stamp_ > kExpirationTimeSlice) {
        collection_->OnTimer(current_time);
        time_stamp_ = current_time;
      }
    }
    return glint::MESSAGE_CONTINUE;
  }
 private:
  BalloonCollection *collection_;
  double time_stamp_;
  DISALLOW_EVIL_CONSTRUCTORS(TimerHandler);
};

class AnimationCompletedHandler : public glint::MessageHandler {
 public:
  AnimationCompletedHandler(Balloon *balloon) : balloon_(balloon) {
    assert(balloon);
  }

  glint::MessageResultCode HandleMessage(const glint::Message &message) {
    if (message.code == glint::GL_MSG_ANIMATION_COMPLETED) {
      balloon_->OnAnimationCompleted();
    }
    return glint::MESSAGE_CONTINUE;
  }

 private:
  Balloon *balloon_;
  DISALLOW_EVIL_CONSTRUCTORS(AnimationCompletedHandler);
};

class RemoveWorkItem : public glint::WorkItem {
 public:
  RemoveWorkItem(BalloonCollection *collection,
                 Balloon *balloon)
    : collection_(collection), balloon_(balloon) {
    assert(collection_);
    assert(balloon_);
  }
  virtual void Run() {
    collection_->RemoveFromUI(balloon_);
    delete balloon_;
  }
 private:
  BalloonCollection *collection_;
  Balloon *balloon_;
  DISALLOW_EVIL_CONSTRUCTORS(RemoveWorkItem);
};

class MouseWithinDetector : public glint::MessageHandler {
 public:
  MouseWithinDetector(Balloon *balloon, glint::Node *owner)
      : owner_(owner),
        balloon_(balloon),
        mouse_within_(false) {
    assert(owner && balloon);
  }

  ~MouseWithinDetector() {
    // Make sure we reset notification expiration counter.
    if (mouse_within_) {
      balloon_->OnMouseOut();
    }
  }

  glint::MessageResultCode HandleMessage(const glint::Message &message) {
    bool mouse_within_now = mouse_within_;
    if (message.code == glint::GL_MSG_MOUSELEAVE) {
      // Mouse leaves the application's window - signal "mouse out".
      mouse_within_now = false;
    } else if (message.code == glint::GL_MSG_MOUSEMOVE_BROADCAST) {
      // GL_MSG_MOUSEMOVE_BROADCAST comes to all nodes even if the mouse
      // is outside - we use it to detect the mouse in/out condition.
      glint::Rectangle bounds(glint::Point(), owner_->final_size());
      // GL_MSG_MOUSEMOVE_BROADCAST comes with screen coordinate of the
      // mouse, so transform it first.
      glint::Point mouse = message.GetLocalPosition(owner_);
      mouse_within_now = bounds.Contains(mouse);
    }
    // Detect change in mouse "withinness".
    if (mouse_within_now != mouse_within_) {
      mouse_within_ = mouse_within_now;
      if (mouse_within_) {
        balloon_->OnMouseIn();
      } else {
        balloon_->OnMouseOut();
      }
    }
    return glint::MESSAGE_CONTINUE;
  }

 private:
  glint::Node *owner_;
  Balloon *balloon_;
  bool mouse_within_;
  DISALLOW_EVIL_CONSTRUCTORS(MouseWithinDetector);
};

Balloon::Balloon(const GearsNotification &from, BalloonCollection *collection)
  : root_(NULL),
    collection_(collection),
    state_(OPENING_BALLOON)  {
  assert(collection_);
  notification_.CopyFrom(from);
}

Balloon::~Balloon() {
}

glint::Node *Balloon::CreateTree() {
#if defined(OS_MACOSX) || defined (LINUX)
  std::string resource_path = System::GetResourcePath();
#else
  // This only works for Win32
  std::string resource_path = "res:/";
#endif
  // Note: we set min/max sizes on this node in BalloonContainer during
  // layout pass (see BalloonContainer::OnComputeRequiredSize)
  glint::Node *root = new glint::Node();
  root->set_alpha(glint::colors::kTransparentAlpha);
  glint::Rectangle margin(0, 0, 5, 7);
  root->set_margin(margin);

  glint::NineGrid *background = new glint::NineGrid();
  background->ReplaceImage(resource_path + "/background.png");
  margin.Set(-5, -2, -5, -7);
  background->set_margin(margin);
  root->AddChild(background);

  glint::Row *main_row = new glint::Row();
  main_row->ReplaceDistribution("natural,*,23");  // 3 columns
  root->AddChild(main_row);

  glint::ImageNode *image = new glint::ImageNode();
  image->set_id(kIconId);
  image->set_vertical_alignment(glint::Y_TOP);
  image->set_horizontal_alignment(glint::X_LEFT);
  margin.Set(12, 10, 8, 0);
  image->set_margin(margin);
  main_row->AddChild(image);

  glint::Column *column = new glint::Column();
  margin.Set(0, 6, 4, 0);
  column->set_margin(margin);
  main_row->AddChild(column);

  glint::SimpleText *text = new glint::SimpleText();
  text->set_id(kTitleId);
  text->set_font_family(kFontName);
  text->set_font_size(kTitleFontSize);
  text->set_bold(true);
  text->set_horizontal_alignment(glint::X_LEFT);
  text->AddHandler(new ChangeFontHandler(text, text->font_size()));
  column->AddChild(text);

  text = new glint::SimpleText();
  text->set_id(kSubtitleId);
  text->set_font_family(kFontName);
  text->set_font_size(kSubtitleFontSize);
  margin.Set(0, 4, 0, 0);
  text->set_margin(margin);
  text->set_horizontal_alignment(glint::X_LEFT);
  text->AddHandler(new ChangeFontHandler(text, text->font_size()));
  column->AddChild(text);

  text = new glint::SimpleText();
  text->set_id(kDescriptionId);
  text->set_font_family(kFontName);
  text->set_font_size(kDescriptionFontSize);
  margin.Set(0, 4, 0, 0);
  text->set_margin(margin);
  text->set_horizontal_alignment(glint::X_LEFT);
  text->AddHandler(new ChangeFontHandler(text, text->font_size()));
  column->AddChild(text);

  glint::Row *actions = new glint::Row();
  actions->set_id(kActionsContainer);
  actions->set_horizontal_alignment(glint::X_RIGHT);
  margin.Set(0, 0, 10, 10);
  actions->set_margin(margin);
  column->AddChild(actions);

  glint::Node *button_container = new glint::Node();
  main_row->AddChild(button_container);

  glint::Button *close_button = new glint::Button();
#ifdef OS_MACOSX
  close_button->ReplaceImage(resource_path + "/close_button_strip_osx.png");
  close_button->set_min_height(30);
  close_button->set_min_width(30);
  margin.Set(-4, -4, 0, 0);  // slightly out of the frame
  close_button->set_margin(margin);
  close_button->set_horizontal_alignment(glint::X_LEFT);
  close_button->set_vertical_alignment(glint::Y_TOP);
  close_button->set_alpha(glint::colors::kTransparentAlpha);
  close_button->set_id(kCloseButton);
  SetAlphaTransition(close_button, 0.25);
  root->AddChild(close_button);
#else
  close_button->ReplaceImage(resource_path + "/close_button_strip.png");
  close_button->set_min_height(22);
  close_button->set_min_width(22);
  margin.Set(0, 3, 3, 0);
  close_button->set_margin(margin);
  close_button->set_horizontal_alignment(glint::X_RIGHT);
  close_button->set_vertical_alignment(glint::Y_TOP);
  button_container->AddChild(close_button);
#endif  // OS_MACOSX
  close_button->SetClickHandler(Balloon::OnCloseButton, this);

  glint::Button *menu_button = new glint::Button();
  menu_button->ReplaceImage(resource_path + "/menu_button_strip.png");
  menu_button->set_min_height(22);
  menu_button->set_min_width(22);
  margin.Set(0, 0, 3, 4);
  menu_button->set_margin(margin);
  menu_button->set_horizontal_alignment(glint::X_RIGHT);
  menu_button->set_vertical_alignment(glint::Y_BOTTOM);
  menu_button->SetClickHandler(Balloon::OnMenuButton, this);
  button_container->AddChild(menu_button);

  root->AddHandler(new AnimationCompletedHandler(this));
  root->AddHandler(new MouseWithinDetector(this, root));
  SetAlphaTransition(root, kAlphaTransitionDuration);
  SetMoveTransition(root);

  return root;
}

void Balloon::OnMenuButton(const std::string &button_id, void *user_info) {
  Balloon *this_ = reinterpret_cast<Balloon*>(user_info);
  assert(this_);
  this_->ShowMenu();
}

void Balloon::ShowMenu() {
  // Add actions to context menu.
  int num = ARRAYSIZE(kDefaultContextMenuItems);
  if (!notification_.actions().empty()) {
    num += notification_.actions().size() + 1;
  }
  std::vector<System::MenuItem> context_menu_items;
  for (size_t i = 0; i < notification_.actions().size(); ++i) {
    System::MenuItem menu_item;
    String16ToUTF8(notification_.actions().at(i).text.c_str(),
                   &menu_item.title);
    menu_item.command_id = kContextMenuItemOffset + static_cast<int>(i);
    menu_item.enabled = true;
    menu_item.checked = false;
    context_menu_items.push_back(menu_item);
  }

  // Add separator to context menu.
  System::MenuItem separator;
  separator.title = "-";
  separator.command_id = -1;
  separator.enabled = false;
  separator.checked = false;
  context_menu_items.push_back(separator);

  // Add default menu items.
  for (size_t i = 0; i < ARRAYSIZE(kDefaultContextMenuItems); ++i) {
    context_menu_items.push_back(kDefaultContextMenuItems[i]);
  }

  // Stop expiration while mouse is over context menu.
  collection_->SuspendExpirationTimer();
  int command = System::ShowContextMenu(&context_menu_items.front(),
                                        context_menu_items.size(),
                                        root_->GetRootUI());
  collection_->RestoreExpirationTimer();
  switch (command) {
    case DISABLE_NOTIFICATIONS_FOR_1_HOUR:
      collection_->Snooze(kOneHourSeconds);
      break;
    case SHOW_PREFERENCES:
      System::ShowNotifierPreferences();
      break;
    default:
      if (command >= kContextMenuItemOffset) {
        PerformAction(
            notification_.actions().at(command - kContextMenuItemOffset).url);
      }
      break;
  }
}

void Balloon::PerformAction(const std::string16 &action) {
  assert(!action.empty());

  static const std::string16 kHttpUrl = STRING16(L"http://");
  static const std::string16 kHttpsUrl = STRING16(L"https://");
  if (StartsWith(action, kHttpUrl) || StartsWith(action, kHttpsUrl)) {
    System::OpenUrlInBrowser(action.c_str());
    return;
  }

  static const std::string16 kActionDelimiters = STRING16(L",");
  std::vector<std::string16> fields;
  if (!Tokenize(action, kActionDelimiters, &fields) || fields.empty()) {
    return;
  }

  if (fields[0] == kSnoozeAction) {
    InitiateClose(true);
    int snooze_seconds = 0;
    if (fields.size() < 2 ||
        !String16ToInt(fields[1].c_str(), &snooze_seconds)) {
      snooze_seconds = kDefaultSnoozeSeconds;
    }
    collection_->OnSnoozeNotification(notification_, snooze_seconds);
  }
}

void Balloon::OnCloseButton(const std::string &button_id, void *user_info) {
  Balloon *this_ = reinterpret_cast<Balloon*>(user_info);
  assert(this_);
  this_->InitiateClose(true);  // true == 'user_initiated'
}

bool Balloon::SetImage(const char *id,
                       int width,
                       int height,
                       const void *decoded_image) {
  assert(id);
  glint::ImageNode *image_node = static_cast<glint::ImageNode*>(
      root_->FindNodeById(id));
  if (!image_node)
    return false;
  image_node->ReplaceBitmap(width, height, decoded_image);
  return true;
}

bool Balloon::SetTextField(const char *id, const std::string16 &text) {
  assert(id);
  glint::SimpleText *text_node = static_cast<glint::SimpleText*>(
      root_->FindNodeById(id));
  if (!text_node)
    return false;
  std::string text_utf8;
  if (!String16ToUTF8(text.c_str(), &text_utf8))
    return false;
  text_node->set_text(text_utf8);
  return true;
}

void Balloon::UpdateUI() {
  if (!notification_.icon_raw_data().empty()) {
    SetImage(kIconId,
             kNotificationIconDimensions,
             kNotificationIconDimensions,
             &(notification_.icon_raw_data()[0]));
  } else {
    SetImage(kIconId, 0, 0, NULL);
  }
  SetTextField(kTitleId, notification_.title());
  SetTextField(kSubtitleId, notification_.subtitle());
  SetTextField(kDescriptionId, notification_.description());
}

bool Balloon::SetAlphaTransition(glint::Node *node,
                                 double transition_duration) {
  if (!node)
    return false;

  glint::AnimationTimeline *timeline = NULL;
  if (transition_duration > 0) {
    timeline = new glint::AnimationTimeline();

    glint::AlphaAnimationSegment *segment = new glint::AlphaAnimationSegment();
    segment->set_type(glint::RELATIVE_TO_START);
    timeline->AddAlphaSegment(segment);

    segment = new glint::AlphaAnimationSegment();
    segment->set_type(glint::RELATIVE_TO_FINAL);
    segment->set_duration(transition_duration);
    timeline->AddAlphaSegment(segment);

    timeline->RequestCompletionMessage(node);
  }
  node->SetTransition(glint::ALPHA_TRANSITION, timeline);
  return true;
}

bool Balloon::SetMoveTransition(glint::Node *node) {
  if (!node)
    return false;

  glint::AnimationTimeline *timeline = new glint::AnimationTimeline();

  glint::TranslationAnimationSegment *segment =
      new glint::TranslationAnimationSegment();
  segment->set_type(glint::RELATIVE_TO_START);
  timeline->AddTranslationSegment(segment);

  segment = new glint::TranslationAnimationSegment();
  segment->set_type(glint::RELATIVE_TO_FINAL);
  segment->set_duration(kMoveTransitionDuration);
  segment->set_interpolation(glint::Interpolation::Smooth);
  timeline->AddTranslationSegment(segment);

  node->SetTransition(glint::MOVE_TRANSITION, timeline);
  return true;
}

bool Balloon::InitiateClose(bool user_initiated) {
  state_ = user_initiated ? USER_CLOSING_BALLOON : AUTO_CLOSING_BALLOON;

  // If closed by user, set shorter transition.
  if (user_initiated) {
    SetAlphaTransition(root_, kAlphaTransitionDurationShort);
  }

  root_->set_alpha(glint::colors::kTransparentAlpha);
  return true;
}

void Balloon::OnAnimationCompleted() {
  switch (state_) {
    case OPENING_BALLOON:
      state_ = SHOWING_BALLOON;
      break;

    case AUTO_CLOSING_BALLOON:
    case USER_CLOSING_BALLOON: {
      // Need to do this async because 'animation completion' message gets
      // fired from synchronous tree walk and it's bad to remove part of
      // the tree during the walk.
      RemoveWorkItem *remove_work = new RemoveWorkItem(collection_, this);
      glint::platform()->PostWorkItem(NULL, remove_work);
      break;
    }

    case RESTORING_BALLOON:
      state_ = SHOWING_BALLOON;
      SetAlphaTransition(root_, kAlphaTransitionDuration);
      break;

    case SHOWING_BALLOON:
      // We might be doing some other animation while showing the balloon,
      // like the scaling animation when the system font changes.
      break;

    default:
      assert(false);
      break;
  }
}

void Balloon::OnMouseIn() {
  // If closing it automatically, bring it back
  if (state_ == AUTO_CLOSING_BALLOON) {
    // Cancel current animation
    SetAlphaTransition(root_, 0);

    // Restore the alpha to opaque, fast.
    SetAlphaTransition(root_, kAlphaTransitionDurationForRestore);
    root_->set_alpha(glint::colors::kOpaqueAlpha);

    state_ = RESTORING_BALLOON;
  }
  collection_->SuspendExpirationTimer();
  glint::Node *close_button = root_->FindNodeById(kCloseButton);
  if (close_button) {
    close_button->set_alpha(glint::colors::kOpaqueAlpha);
  }
}

void Balloon::OnMouseOut() {
  collection_->RestoreExpirationTimer();
  glint::Node *close_button = root_->FindNodeById(kCloseButton);
  if (close_button) {
    close_button->set_alpha(glint::colors::kTransparentAlpha);
  }
}

bool Balloon::InitializeUI(glint::Node *container) {
  assert(container);
  assert(!root_);
  // Some operations (triggering animations for example) require node to be
  // already connected to a RootUI. So first create tree, hook it up to
  // container and then complete initialization.
  root_ = CreateTree();
  if (!root_)
    return false;

  container->AddChild(root_);
  UpdateUI();

  // Start revealing animation.
  state_ = OPENING_BALLOON;
  root_->set_alpha(glint::colors::kOpaqueAlpha);
  return true;
}

BalloonCollection::BalloonCollection(BalloonCollectionObserver *observer)
  : observer_(observer),
    root_ui_(NULL),
    expiration_suspended_counter_(0),
    last_time_(-1),
    elapsed_time_(0),
    has_space_(true) {
  assert(observer);
  EnsureRoot();
  observer_->OnBalloonSpaceChanged();
}

BalloonCollection::~BalloonCollection() {
  Clear();

  // NULL the root_ui_ and then delete it
  // to avoid unnecessary processing when it is deleted.
  glint::RootUI *root = root_ui_;
  root_ui_ = NULL;
  delete root;

  // Just making sure that no code recreated the root_ui_
  // while it was being destroyed.
  assert(!root_ui_);
}

void BalloonCollection::Clear() {
  for (Balloons::iterator it = balloons_.begin();
       it != balloons_.end();
       ++it) {
    Balloon *removed = *it;
    delete removed;
  }
  balloons_.clear();
}

Balloon *BalloonCollection::FindBalloon(
    const SecurityOrigin &security_origin,
    const std::string16 &id,
    bool and_remove) {
  for (Balloons::iterator it = balloons_.begin();
       it != balloons_.end();
       ++it) {
    if ((*it)->notification().Matches(security_origin, id)) {
      Balloon *result = *it;
      if (and_remove) {
        balloons_.erase(it);
      }
      return result;
    }
  }
  return NULL;
}

bool BalloonCollection::HasSpace() const {
  // Root is not initialized => no balloons yet => means we have space.
  if (!root_ui_)
    return true;

  return BalloonContainer::max_balloon_height() * count() <
         BalloonContainer::work_area_size().height * kNotificationFillFactor;
}

void BalloonCollection::Add(const GearsNotification &notification) {
  Balloon *balloon = FindBalloon(notification.security_origin(),
                                 notification.id(),
                                 false);  // no remove
  assert(!balloon);
  // If someone repeatedly adds the same notification, avoid flood - return.
  if (balloon)
    return;

  Balloon *new_balloon = new Balloon(notification, this);
  AddToUI(new_balloon);
  observer_->OnBalloonSpaceChanged();
}

bool BalloonCollection::Update(const GearsNotification &notification) {
  Balloon *balloon = FindBalloon(notification.security_origin(),
                                 notification.id(),
                                 false);  // no remove
  if (!balloon)
    return false;
  balloon->mutable_notification()->CopyFrom(notification);
  balloon->UpdateUI();
  return true;
}

bool BalloonCollection::Delete(const SecurityOrigin &security_origin,
                               const std::string16 &id) {
  Balloon *balloon = FindBalloon(security_origin,
                                 id,
                                 true);  // remove from list immediately
  if (!balloon)
    return false;

  if (!balloon->InitiateClose(false))  // not user-initiated.
    return false;

  observer_->OnBalloonSpaceChanged();
  return true;
}

void BalloonCollection::ShowAll() {
  if (!root_ui_)
    return;
  glint::Node *root_node = root_ui_->root_node();
  assert(root_node);
  root_node->set_alpha(glint::colors::kOpaqueAlpha);
}

void BalloonCollection::HideAll() {
  if (!root_ui_)
    return;
  glint::Node *root_node = root_ui_->root_node();
  assert(root_node);
  root_node->set_alpha(glint::colors::kTransparentAlpha);
}

void BalloonCollection::EnsureRoot() {
  if (root_ui_)
    return;

  root_ui_ = new glint::RootUI(true);  // topmost window
  glint::Node *container = new BalloonContainer();
  container->set_id(kBalloonContainer);
  container->AddHandler(new TimerHandler(this));

  // Set transitional animation for alpha.
  glint::AnimationTimeline *timeline = new glint::AnimationTimeline();

  glint::AlphaAnimationSegment *segment = new glint::AlphaAnimationSegment();
  segment->set_type(glint::RELATIVE_TO_START);
  timeline->AddAlphaSegment(segment);

  segment = new glint::AlphaAnimationSegment();
  segment->set_type(glint::RELATIVE_TO_FINAL);
  segment->set_duration(kAlphaTransitionDurationShort);
  segment->set_interpolation(glint::Interpolation::Smooth);
  timeline->AddAlphaSegment(segment);

  container->SetTransition(glint::ALPHA_TRANSITION, timeline);

  root_ui_->set_root_node(container);
  root_ui_->Show();
}

bool BalloonCollection::AddToUI(Balloon *balloon) {
  assert(balloon);
  EnsureRoot();

  ResetExpirationTimer();
  balloons_.push_back(balloon);

  glint::Row *container = static_cast<glint::Row*>(
      root_ui_->FindNodeById(kBalloonContainer));
  if (!container)
    return false;
  return balloon->InitializeUI(container);
}

bool BalloonCollection::RemoveFromUI(Balloon *balloon) {
  assert(balloon);
  if (!root_ui_)
    return false;

  ResetExpirationTimer();
  // Ignore return value. The balloon could already be removed from balloons_
  // by Delete method - we only keep balloons for longer if it was a
  // user-initiated 'close' operation or auto-expiration.
  FindBalloon(balloon->notification().security_origin(),
              balloon->notification().id(),
              true);  // remove from list

  glint::Row *container = static_cast<glint::Row*>(
      root_ui_->FindNodeById(kBalloonContainer));
  if (!container)
    return false;

  container->RemoveNode(balloon->root());
  observer_->OnBalloonSpaceChanged();
  return true;
}

void BalloonCollection::SuspendExpirationTimer() {
  ++expiration_suspended_counter_;
}

void BalloonCollection::RestoreExpirationTimer() {
  --expiration_suspended_counter_;
}

void BalloonCollection::ResetExpirationTimer() {
  last_time_ = -1;
}

void BalloonCollection::OnTimer(double current_time) {
  if (balloons_.empty())
    return;

  // Timer was reset - start it.
  if (last_time_ < 0) {
    last_time_ = current_time;
    elapsed_time_ = 0;
    return;
  }

  double delta = current_time - last_time_;
  last_time_ = current_time;

  if (expiration_suspended()) {
    // Set elapsed time to half of expiration time so the balloon does
    // not go away immediately after mouse goes away.
    elapsed_time_ = kExpirationTime / 2;
    return;
  }

  elapsed_time_ += delta;

  if (elapsed_time_ > kExpirationTime) {
    // Expire the bottom-most balloon.
    Balloon *balloon = balloons_.front();
    if (!balloon)
      return;
    balloon->InitiateClose(false);  // 'false' == not initiated by the user.
    ResetExpirationTimer();
  }
}

static void OnSnoozeTimerFired(void *arg) {
  BalloonCollection *collection = reinterpret_cast<BalloonCollection*>(arg);
  collection->ShowAll();
  collection->RestoreExpirationTimer();
}

void BalloonCollection::Snooze(int timeout_seconds) {
  ResetExpirationTimer();
  SuspendExpirationTimer();
  HideAll();
  snooze_timer_.reset(new TimedCall(static_cast<int64>(timeout_seconds) * 1000,
                                    false,  // no repeat
                                    OnSnoozeTimerFired,
                                    this));
}

void BalloonCollection::OnSnoozeNotification(
    const GearsNotification &notification, int snooze_seconds) {
  observer_->OnSnoozeNotification(notification, snooze_seconds);
}

#endif  // OFFICIAL_BULID
