// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_VIEWS_AUTOFILL_SAVE_ADDRESS_PROFILE_VIEW_H_
#define CHROME_BROWSER_UI_VIEWS_AUTOFILL_SAVE_ADDRESS_PROFILE_VIEW_H_

#include "chrome/browser/ui/autofill/autofill_bubble_base.h"
#include "chrome/browser/ui/views/location_bar/location_bar_bubble_delegate_view.h"

namespace content {
class WebContents;
}

namespace views {
class ImageButton;
class ImageView;
class View;
}  // namespace views

namespace autofill {
class SaveUpdateAddressProfileBubbleController;

// This is the bubble views that is part of the flow for when the user submits a
// form with an address profile that Autofill has not previously saved.
class SaveAddressProfileView : public AutofillBubbleBase,
                               public LocationBarBubbleDelegateView {
 public:
  SaveAddressProfileView(views::View* anchor_view,
                         content::WebContents* web_contents,
                         SaveUpdateAddressProfileBubbleController* controller);

  SaveAddressProfileView(const SaveAddressProfileView&) = delete;
  SaveAddressProfileView& operator=(const SaveAddressProfileView&) = delete;
  ~SaveAddressProfileView() override;

  // views::WidgetDelegate:
  bool ShouldShowCloseButton() const override;
  void WindowClosing() override;

  void Show(DisplayReason reason);

  // AutofillBubbleBase:
  void Hide() override;

  // View:
  void AddedToWidget() override;
  void OnThemeChanged() override;

 private:
  // Sets the proper margins for icons (and other views) in the UI to make sure
  // all icons are vertically centered with corresponding text.
  void AlignIcons();

  SaveUpdateAddressProfileBubbleController* controller_;

  // The following are used for UI elements alignment upon changes in theme.
  views::View* address_components_view_;
  std::vector<views::ImageView*> address_section_icons_;
  views::ImageButton* edit_button_;
};

}  // namespace autofill

#endif  // CHROME_BROWSER_UI_VIEWS_AUTOFILL_SAVE_ADDRESS_PROFILE_VIEW_H_
