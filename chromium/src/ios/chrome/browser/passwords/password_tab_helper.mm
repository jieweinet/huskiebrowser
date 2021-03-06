// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/passwords/password_tab_helper.h"

#include "base/check.h"
#include "base/memory/ptr_util.h"
#import "ios/chrome/browser/passwords/password_controller.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

PasswordTabHelper::~PasswordTabHelper() = default;

// static
void PasswordTabHelper::CreateForWebState(web::WebState* web_state) {
  DCHECK(web_state);
  if (!FromWebState(web_state)) {
    web_state->SetUserData(UserDataKey(),
                           base::WrapUnique(new PasswordTabHelper(web_state)));
  }
}

void PasswordTabHelper::SetBaseViewController(
    UIViewController* baseViewController) {
  controller_.baseViewController = baseViewController;
}

void PasswordTabHelper::SetPasswordControllerDelegate(
    id<PasswordControllerDelegate> delegate) {
  controller_.delegate = delegate;
}

void PasswordTabHelper::SetDispatcher(CommandDispatcher* dispatcher) {
  controller_.dispatcher = dispatcher;
}

id<FormSuggestionProvider> PasswordTabHelper::GetSuggestionProvider() {
  return controller_.suggestionProvider;
}

password_manager::PasswordGenerationFrameHelper*
PasswordTabHelper::GetGenerationHelper() {
  return controller_.passwordManagerDriver->GetPasswordGenerationHelper();
}

password_manager::PasswordManager* PasswordTabHelper::GetPasswordManager() {
  return controller_.passwordManager;
}

password_manager::PasswordManagerClient*
PasswordTabHelper::GetPasswordManagerClient() {
  return controller_.passwordManagerClient;
}

id<PasswordGenerationProvider>
PasswordTabHelper::GetPasswordGenerationProvider() {
  return controller_.generationProvider;
}

PasswordTabHelper::PasswordTabHelper(web::WebState* web_state)
    : controller_([[PasswordController alloc] initWithWebState:web_state]) {
  web_state->AddObserver(this);
}

void PasswordTabHelper::WebStateDestroyed(web::WebState* web_state) {
  web_state->RemoveObserver(this);
  controller_ = nil;
}

WEB_STATE_USER_DATA_KEY_IMPL(PasswordTabHelper)
