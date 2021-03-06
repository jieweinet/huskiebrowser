// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_AUTOFILL_PAYMENTS_CARD_UNMASK_OTP_INPUT_DIALOG_CONTROLLER_IMPL_H_
#define CHROME_BROWSER_UI_AUTOFILL_PAYMENTS_CARD_UNMASK_OTP_INPUT_DIALOG_CONTROLLER_IMPL_H_

#include <string>

#include "build/build_config.h"
#include "chrome/browser/ui/autofill/payments/card_unmask_otp_input_dialog_controller.h"
#include "chrome/browser/ui/autofill/payments/card_unmask_otp_input_dialog_view.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"

namespace autofill {

enum class OtpUnmaskResult;

class CardUnmaskOtpInputDialogControllerImpl
    : public CardUnmaskOtpInputDialogController,
      public content::WebContentsObserver,
      public content::WebContentsUserData<
          CardUnmaskOtpInputDialogControllerImpl> {
 public:
  CardUnmaskOtpInputDialogControllerImpl(
      const CardUnmaskOtpInputDialogControllerImpl&) = delete;
  CardUnmaskOtpInputDialogControllerImpl& operator=(
      const CardUnmaskOtpInputDialogControllerImpl&) = delete;
  ~CardUnmaskOtpInputDialogControllerImpl() override;

  void ShowDialog(size_t otp_length);

  // CardUnmaskOtpInputDialogController:
  void OnDialogClosed() override;
  std::u16string GetWindowTitle() const override;
  std::u16string GetTextfieldPlaceholderText() const override;
#if defined(OS_ANDROID)
  int GetExpectedOtpLength() const override;
#endif  // OS_ANDROID
  bool IsValidOtp(const std::u16string& otp) const override;
  FooterText GetFooterText(const std::u16string& link_text) const override;
  std::u16string GetNewCodeLinkText() const override;
  std::u16string GetOkButtonLabel() const override;
  std::u16string GetProgressLabel() const override;

#if defined(UNIT_TEST)
  CardUnmaskOtpInputDialogView* GetDialogViewForTesting();
#endif

 private:
  explicit CardUnmaskOtpInputDialogControllerImpl(
      content::WebContents* web_contents);

  friend class content::WebContentsUserData<
      CardUnmaskOtpInputDialogControllerImpl>;

  // Sets the view's state to the invalid state for the corresponding
  // |otp_unmask_result|.
  void ShowInvalidState(OtpUnmaskResult otp_unmask_result);

  size_t otp_length_;
  CardUnmaskOtpInputDialogView* dialog_view_ = nullptr;

  WEB_CONTENTS_USER_DATA_KEY_DECL();
};

}  // namespace autofill

#endif  // CHROME_BROWSER_UI_AUTOFILL_PAYMENTS_CARD_UNMASK_OTP_INPUT_DIALOG_CONTROLLER_IMPL_H_
