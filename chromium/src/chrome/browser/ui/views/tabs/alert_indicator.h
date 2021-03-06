// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_VIEWS_TABS_ALERT_INDICATOR_H_
#define CHROME_BROWSER_UI_VIEWS_TABS_ALERT_INDICATOR_H_

#include <memory>

#include "chrome/browser/ui/tabs/tab_utils.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "ui/base/metadata/metadata_header_macros.h"
#include "ui/views/controls/image_view.h"

class Tab;

namespace gfx {
class Animation;
class AnimationDelegate;
}  // namespace gfx

// This is an ImageView subclass that serves as an indicator of various states,
// primarily media playing/recording, but also device connectivity.  It is meant
// to only be used as a child view of Tab.
class AlertIndicator : public views::ImageView {
 public:
  METADATA_HEADER(AlertIndicator);
  explicit AlertIndicator(Tab* parent_tab);
  AlertIndicator(const AlertIndicator&) = delete;
  AlertIndicator& operator=(const AlertIndicator&) = delete;
  ~AlertIndicator() override;

  // views::ImageView:
  void OnPaint(gfx::Canvas* canvas) override;

  // Returns the current TabAlertState except, while the indicator image is
  // fading out, returns the prior TabAlertState.
  absl::optional<TabAlertState> showing_alert_state() const {
    return showing_alert_state_;
  }

  // Calls ResetImages() and starts fade animations as appropriate.
  void TransitionToAlertState(absl::optional<TabAlertState> next_state);

  // Called when the parent tab's button color changes.  Determines whether
  // ResetImages() needs to be called.
  void OnParentTabButtonColorChanged();

 protected:
  View* GetTooltipHandlerForPoint(const gfx::Point& point) override;

 private:
  friend class AlertIndicatorTest;
  friend class TabTest;
  class FadeAnimationDelegate;

  // Resets the images to display on the button to reflect |state| and the
  // parent tab's button color.  Should be called when either of these changes.
  void ResetImage(TabAlertState state);

  Tab* const parent_tab_;
  absl::optional<TabAlertState> alert_state_;
  std::unique_ptr<gfx::AnimationDelegate> fade_animation_delegate_;
  std::unique_ptr<gfx::Animation> fade_animation_;
  absl::optional<TabAlertState> showing_alert_state_;
};

#endif  // CHROME_BROWSER_UI_VIEWS_TABS_ALERT_INDICATOR_H_
