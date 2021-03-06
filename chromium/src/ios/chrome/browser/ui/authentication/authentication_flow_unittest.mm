// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/ui/authentication/authentication_flow.h"

#include <memory>

#include "base/bind.h"
#include "base/memory/ptr_util.h"
#import "base/test/ios/wait_util.h"
#import "base/test/metrics/histogram_tester.h"
#include "components/pref_registry/pref_registry_syncable.h"
#import "components/signin/public/base/signin_metrics.h"
#include "components/sync_preferences/pref_service_mock_factory.h"
#include "components/sync_preferences/pref_service_syncable.h"
#include "ios/chrome/browser/browser_state/test_chrome_browser_state.h"
#include "ios/chrome/browser/main/test_browser.h"
#include "ios/chrome/browser/prefs/browser_prefs.h"
#include "ios/chrome/browser/signin/authentication_service_factory.h"
#import "ios/chrome/browser/signin/authentication_service_fake.h"
#import "ios/chrome/browser/signin/chrome_account_manager_service.h"
#import "ios/chrome/browser/signin/chrome_account_manager_service_factory.h"
#import "ios/chrome/browser/ui/authentication/authentication_flow_performer.h"
#include "ios/public/provider/chrome/browser/signin/fake_chrome_identity.h"
#include "ios/public/provider/chrome/browser/signin/fake_chrome_identity_service.h"
#include "ios/web/public/test/web_task_environment.h"
#import "testing/gtest_mac.h"
#import "testing/platform_test.h"
#import "third_party/ocmock/gtest_support.h"
#import "third_party/ocmock/ocmock_extensions.h"
#include "ui/base/l10n/l10n_util.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

namespace {

class AuthenticationFlowTest : public PlatformTest {
 protected:
  void SetUp() override {
    PlatformTest::SetUp();

    TestChromeBrowserState::Builder builder;
    builder.AddTestingFactory(
        AuthenticationServiceFactory::GetInstance(),
        base::BindRepeating(
            &AuthenticationServiceFake::CreateAuthenticationService));
    builder.SetPrefService(CreatePrefService());
    browser_state_ = builder.Build();
    WebStateList* web_state_list = nullptr;
    browser_ =
        std::make_unique<TestBrowser>(browser_state_.get(), web_state_list);

    ios::FakeChromeIdentityService* identityService =
        ios::FakeChromeIdentityService::GetInstanceFromChromeProvider();
    identityService->AddIdentities(@[ @"identity1", @"identity2" ]);

    ChromeAccountManagerService* account_manager_service =
        ChromeAccountManagerServiceFactory::GetForBrowserState(
            browser_state_.get());
    NSArray<ChromeIdentity*>* identities =
        account_manager_service->GetAllIdentities();
    identity1_ = identities[0];
    identity2_ = identities[1];
    managed_identity_ = [FakeChromeIdentity identityWithEmail:@"managed@foo.com"
                                                       gaiaID:@"managed"
                                                         name:@"managed"];
    identityService->AddIdentity(managed_identity_);

    sign_in_completion_ = ^(BOOL success) {
      finished_ = true;
      signed_in_success_ = success;
    };
    finished_ = false;
    signed_in_success_ = false;
  }

  std::unique_ptr<sync_preferences::PrefServiceSyncable> CreatePrefService() {
    sync_preferences::PrefServiceMockFactory factory;
    scoped_refptr<user_prefs::PrefRegistrySyncable> registry(
        new user_prefs::PrefRegistrySyncable);
    std::unique_ptr<sync_preferences::PrefServiceSyncable> prefs =
        factory.CreateSyncable(registry.get());
    RegisterBrowserStatePrefs(registry.get());
    return prefs;
  }

  AuthenticationFlowPerformer* GetAuthenticationFlowPerformer() {
    return static_cast<AuthenticationFlowPerformer*>(performer_);
  }

  // Creates a new AuthenticationFlow with default values for fields that are
  // not directly useful.
  void CreateAuthenticationFlow(ShouldClearData shouldClearData,
                                PostSignInAction postSignInAction,
                                ChromeIdentity* identity) {
    view_controller_ = [OCMockObject niceMockForClass:[UIViewController class]];
    authentication_flow_ =
        [[AuthenticationFlow alloc] initWithBrowser:browser_.get()
                                           identity:identity
                                    shouldClearData:shouldClearData
                                   postSignInAction:postSignInAction
                           presentingViewController:view_controller_];
    performer_ =
        [OCMockObject mockForClass:[AuthenticationFlowPerformer class]];
    [authentication_flow_
        setPerformerForTesting:GetAuthenticationFlowPerformer()];
  }

  // Checks if the AuthenticationFlow operation has completed, and whether it
  // was successful.
  void CheckSignInCompletion(bool expected_signed_in) {
    base::test::ios::WaitUntilCondition(^bool {
      return finished_;
    });
    EXPECT_TRUE(finished_);
    EXPECT_EQ(expected_signed_in, signed_in_success_);
    [performer_ verify];
  }

  web::WebTaskEnvironment task_environment_;
  AuthenticationFlow* authentication_flow_ = nullptr;
  std::unique_ptr<TestChromeBrowserState> browser_state_;
  std::unique_ptr<Browser> browser_;
  ChromeIdentity* identity1_ = nullptr;
  ChromeIdentity* identity2_ = nullptr;
  ChromeIdentity* managed_identity_ = nullptr;
  OCMockObject* performer_ = nil;
  signin_ui::CompletionCallback sign_in_completion_;
  UIViewController* view_controller_;
  // Used to verify histogram logging.
  base::HistogramTester histogram_tester_;

  // State of the flow
  bool finished_;
  bool signed_in_success_;
};

// Tests a Sign In of a normal account on the same profile, merging user data
// and showing the sync settings.
TEST_F(AuthenticationFlowTest, TestSignInSimple) {
  CreateAuthenticationFlow(SHOULD_CLEAR_DATA_MERGE_DATA,
                           POST_SIGNIN_ACTION_COMMIT_SYNC, identity1_);

  [[[performer_ expect] andDo:^(NSInvocation*) {
    [authentication_flow_ didFetchManagedStatus:nil];
  }] fetchManagedStatus:browser_state_.get()
             forIdentity:identity1_];

  [[[performer_ expect] andReturnBool:NO]
      shouldHandleMergeCaseForIdentity:identity1_
                          browserState:browser_state_.get()];

  [[performer_ expect] signInIdentity:identity1_
                     withHostedDomain:nil
                       toBrowserState:browser_state_.get()];

  [[performer_ expect] commitSyncForBrowserState:browser_state_.get()];

  [authentication_flow_ startSignInWithCompletion:sign_in_completion_];

  CheckSignInCompletion(/*expected_signed_in=*/true);
  histogram_tester_.ExpectUniqueSample(
      "Signin.AccountType.SigninConsent",
      signin_metrics::SigninAccountType::kRegular, 1);
  histogram_tester_.ExpectUniqueSample(
      "Signin.AccountType.SyncConsent",
      signin_metrics::SigninAccountType::kRegular, 1);
}

// Tests that starting sync while the user is already signed in only.
TEST_F(AuthenticationFlowTest, TestAlreadySignedIn) {
  CreateAuthenticationFlow(SHOULD_CLEAR_DATA_MERGE_DATA,
                           POST_SIGNIN_ACTION_COMMIT_SYNC, identity1_);

  [[[performer_ expect] andDo:^(NSInvocation*) {
    [authentication_flow_ didFetchManagedStatus:nil];
  }] fetchManagedStatus:browser_state_.get() forIdentity:identity1_];

  [[[performer_ expect] andReturnBool:NO]
      shouldHandleMergeCaseForIdentity:identity1_
                          browserState:browser_state_.get()];

  [[performer_ expect] signInIdentity:identity1_
                     withHostedDomain:nil
                       toBrowserState:browser_state_.get()];

  [[performer_ expect] commitSyncForBrowserState:browser_state_.get()];

  AuthenticationServiceFactory::GetForBrowserState(browser_state_.get())
      ->SignIn(identity1_);
  [authentication_flow_ startSignInWithCompletion:sign_in_completion_];

  CheckSignInCompletion(/*expected_signed_in=*/true);
  histogram_tester_.ExpectUniqueSample(
      "Signin.AccountType.SigninConsent",
      signin_metrics::SigninAccountType::kRegular, 1);
  histogram_tester_.ExpectUniqueSample(
      "Signin.AccountType.SyncConsent",
      signin_metrics::SigninAccountType::kRegular, 1);
}

// Tests a Sign In&Sync of a different account, requiring a sign out of the
// already signed in account, and asking the user whether data should be cleared
// or merged.
TEST_F(AuthenticationFlowTest, TestSignOutUserChoice) {
  CreateAuthenticationFlow(SHOULD_CLEAR_DATA_USER_CHOICE,
                           POST_SIGNIN_ACTION_COMMIT_SYNC, identity1_);

  [[[performer_ expect] andDo:^(NSInvocation*) {
    [authentication_flow_ didFetchManagedStatus:nil];
  }] fetchManagedStatus:browser_state_.get() forIdentity:identity1_];

  [[[performer_ expect] andReturnBool:YES]
      shouldHandleMergeCaseForIdentity:identity1_
                          browserState:browser_state_.get()];

  [[[performer_ expect] andDo:^(NSInvocation*) {
    [authentication_flow_
        didChooseClearDataPolicy:SHOULD_CLEAR_DATA_CLEAR_DATA];
  }] promptMergeCaseForIdentity:identity1_
                        browser:browser_.get()
                 viewController:view_controller_];

  [[[performer_ expect] andDo:^(NSInvocation*) {
    [authentication_flow_ didSignOut];
  }] signOutBrowserState:browser_state_.get()];

  [[[performer_ expect] andDo:^(NSInvocation*) {
    [authentication_flow_ didClearData];
  }] clearDataFromBrowser:browser_.get() commandHandler:nil];

  [[performer_ expect] signInIdentity:identity1_
                     withHostedDomain:nil
                       toBrowserState:browser_state_.get()];

  [[performer_ expect] commitSyncForBrowserState:browser_state_.get()];

  AuthenticationServiceFactory::GetForBrowserState(browser_state_.get())
      ->SignIn(identity2_);
  [authentication_flow_ startSignInWithCompletion:sign_in_completion_];

  CheckSignInCompletion(/*expected_signed_in=*/true);
  histogram_tester_.ExpectUniqueSample(
      "Signin.AccountType.SigninConsent",
      signin_metrics::SigninAccountType::kRegular, 1);
  histogram_tester_.ExpectUniqueSample(
      "Signin.AccountType.SyncConsent",
      signin_metrics::SigninAccountType::kRegular, 1);
}

// Tests the cancelling of a Sign In.
TEST_F(AuthenticationFlowTest, TestCancel) {
  CreateAuthenticationFlow(SHOULD_CLEAR_DATA_USER_CHOICE,
                           POST_SIGNIN_ACTION_COMMIT_SYNC, identity1_);

  [[[performer_ expect] andDo:^(NSInvocation*) {
    [authentication_flow_ didFetchManagedStatus:nil];
  }] fetchManagedStatus:browser_state_.get()
             forIdentity:identity1_];

  [[[performer_ expect] andReturnBool:YES]
      shouldHandleMergeCaseForIdentity:identity1_
                          browserState:browser_state_.get()];

  [[[performer_ expect] andDo:^(NSInvocation*) {
    [authentication_flow_ cancelAndDismissAnimated:NO];
  }] promptMergeCaseForIdentity:identity1_
                        browser:browser_.get()
                 viewController:view_controller_];

  [[performer_ expect] cancelAndDismissAnimated:NO];

  [authentication_flow_ startSignInWithCompletion:sign_in_completion_];

  CheckSignInCompletion(/*expected_signed_in=*/false);
  histogram_tester_.ExpectTotalCount("Signin.AccountType.SigninConsent", 0);
  histogram_tester_.ExpectTotalCount("Signin.AccountType.SyncConsent", 0);
}

// Tests the fetch managed status failure case.
TEST_F(AuthenticationFlowTest, TestFailFetchManagedStatus) {
  CreateAuthenticationFlow(SHOULD_CLEAR_DATA_MERGE_DATA,
                           POST_SIGNIN_ACTION_COMMIT_SYNC, identity1_);

  NSError* error = [NSError errorWithDomain:@"foo" code:0 userInfo:nil];
  [[[performer_ expect] andDo:^(NSInvocation*) {
    [authentication_flow_ didFailFetchManagedStatus:error];
  }] fetchManagedStatus:browser_state_.get()
             forIdentity:identity1_];

  [[[performer_ expect] andDo:^(NSInvocation* invocation) {
    __unsafe_unretained ProceduralBlock completionBlock;
    [invocation getArgument:&completionBlock atIndex:3];
    completionBlock();
  }] showAuthenticationError:[OCMArg any]
              withCompletion:[OCMArg any]
              viewController:view_controller_
                     browser:browser_.get()];

  [authentication_flow_ startSignInWithCompletion:sign_in_completion_];

  CheckSignInCompletion(/*expected_signed_in=*/false);
  histogram_tester_.ExpectTotalCount("Signin.AccountType.SigninConsent", 0);
  histogram_tester_.ExpectTotalCount("Signin.AccountType.SyncConsent", 0);
}

// Tests the managed sign in confirmation dialog is shown when signing in to
// a managed identity.
TEST_F(AuthenticationFlowTest, TestShowManagedConfirmation) {
  CreateAuthenticationFlow(SHOULD_CLEAR_DATA_CLEAR_DATA,
                           POST_SIGNIN_ACTION_COMMIT_SYNC, managed_identity_);

  [[[performer_ expect] andDo:^(NSInvocation*) {
    [authentication_flow_ didFetchManagedStatus:@"foo.com"];
  }] fetchManagedStatus:browser_state_.get() forIdentity:managed_identity_];

  [[[performer_ expect] andReturnBool:NO]
      shouldHandleMergeCaseForIdentity:managed_identity_
                          browserState:browser_state_.get()];

  [[[performer_ expect] andDo:^(NSInvocation*) {
    [authentication_flow_ didAcceptManagedConfirmation];
  }] showManagedConfirmationForHostedDomain:@"foo.com"
                             viewController:view_controller_
                                    browser:browser_.get()];

  [[[performer_ expect] andDo:^(NSInvocation*) {
    [authentication_flow_ didClearData];
  }] clearDataFromBrowser:browser_.get() commandHandler:nil];

  [[performer_ expect] signInIdentity:managed_identity_
                     withHostedDomain:@"foo.com"
                       toBrowserState:browser_state_.get()];

  [[performer_ expect] commitSyncForBrowserState:browser_state_.get()];

  [authentication_flow_ startSignInWithCompletion:sign_in_completion_];

  CheckSignInCompletion(/*expected_signed_in=*/true);
  histogram_tester_.ExpectUniqueSample(
      "Signin.AccountType.SigninConsent",
      signin_metrics::SigninAccountType::kManaged, 1);
  histogram_tester_.ExpectUniqueSample(
      "Signin.AccountType.SyncConsent",
      signin_metrics::SigninAccountType::kManaged, 1);
}

// Tests sign-in only with a managed account. The managed account confirmation
// dialog should not be shown.
TEST_F(AuthenticationFlowTest, TestShowNoManagedConfirmationForSigninOnly) {
  CreateAuthenticationFlow(SHOULD_CLEAR_DATA_USER_CHOICE,
                           POST_SIGNIN_ACTION_NONE, managed_identity_);

  [[[performer_ expect] andDo:^(NSInvocation*) {
    [authentication_flow_ didFetchManagedStatus:@"foo.com"];
  }] fetchManagedStatus:browser_state_.get() forIdentity:managed_identity_];

  [[[performer_ expect] andReturnBool:NO]
      shouldHandleMergeCaseForIdentity:managed_identity_
                          browserState:browser_state_.get()];

  [[performer_ expect] signInIdentity:managed_identity_
                     withHostedDomain:@"foo.com"
                       toBrowserState:browser_state_.get()];

  [authentication_flow_ startSignInWithCompletion:sign_in_completion_];

  CheckSignInCompletion(/*expected_signed_in=*/true);
  histogram_tester_.ExpectUniqueSample(
      "Signin.AccountType.SigninConsent",
      signin_metrics::SigninAccountType::kManaged, 1);
  histogram_tester_.ExpectTotalCount("Signin.AccountType.SyncConsent", 0);
}

// Tests sign-in only with a managed account, and then starts sync. The managed
// account confirmation dialog should be shown only in sync.
TEST_F(AuthenticationFlowTest, TestSyncAfterSigninAndSync) {
  CreateAuthenticationFlow(SHOULD_CLEAR_DATA_USER_CHOICE,
                           POST_SIGNIN_ACTION_COMMIT_SYNC, managed_identity_);

  [[[performer_ expect] andDo:^(NSInvocation*) {
    [authentication_flow_ didFetchManagedStatus:@"foo.com"];
  }] fetchManagedStatus:browser_state_.get() forIdentity:managed_identity_];

  [[[performer_ expect] andReturnBool:NO]
      shouldHandleMergeCaseForIdentity:managed_identity_
                          browserState:browser_state_.get()];

  [[performer_ expect] signInIdentity:managed_identity_
                     withHostedDomain:@"foo.com"
                       toBrowserState:browser_state_.get()];
  [[[performer_ expect] andDo:^(NSInvocation*) {
    [authentication_flow_ didAcceptManagedConfirmation];
  }] showManagedConfirmationForHostedDomain:@"foo.com"
                             viewController:view_controller_
                                    browser:browser_.get()];
  [[performer_ expect] commitSyncForBrowserState:browser_state_.get()];

  [authentication_flow_ startSignInWithCompletion:sign_in_completion_];

  CheckSignInCompletion(/*expected_signed_in=*/true);
  histogram_tester_.ExpectUniqueSample(
      "Signin.AccountType.SigninConsent",
      signin_metrics::SigninAccountType::kManaged, 1);
  histogram_tester_.ExpectUniqueSample(
      "Signin.AccountType.SyncConsent",
      signin_metrics::SigninAccountType::kManaged, 1);
}

}  // namespace
