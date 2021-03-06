// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/web_apps/web_app_integration_browsertest_base.h"
#include "content/public/test/browser_test.h"

namespace web_app {

namespace {

using WebAppIntegrationBrowserTestMacWinLinux = WebAppIntegrationBrowserTest;

IN_PROC_BROWSER_TEST_F(
    WebAppIntegrationBrowserTestMacWinLinux,
    WebAppIntegration_InstOmniboxSiteA_WindowCreated_InListWinSiteA_UninstallFromMenuSiteA_NotInListSiteA_NavSiteA_InstIconShown_LaunchIconNotShown) {
  // Test contents are generated by script. Please do not modify!
  // See `chrome/test/webapps/README.md` for more info.
  // Sheriffs: Disabling this test is supported.
  helper_.BeforeStateChangeAction();
  helper_.InstallOmniboxIcon("SiteA");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckWindowCreated();
  helper_.AfterStateCheckAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListWindowed("SiteA");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.UninstallFromMenu("SiteA");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppNotInList("SiteA");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.NavigateBrowser("SiteA");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckInstallIconShown();
  helper_.AfterStateCheckAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckLaunchIconNotShown();
  helper_.AfterStateCheckAction();
}

IN_PROC_BROWSER_TEST_F(
    WebAppIntegrationBrowserTestMacWinLinux,
    WebAppIntegration_InstMenuOptionSiteA_WindowCreated_InListWinSiteA_UninstallFromMenuSiteA_NotInListSiteA_NavSiteA_InstIconShown_LaunchIconNotShown) {
  // Test contents are generated by script. Please do not modify!
  // See `chrome/test/webapps/README.md` for more info.
  // Sheriffs: Disabling this test is supported.
  helper_.BeforeStateChangeAction();
  helper_.InstallMenuOption("SiteA");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckWindowCreated();
  helper_.AfterStateCheckAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListWindowed("SiteA");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.UninstallFromMenu("SiteA");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppNotInList("SiteA");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.NavigateBrowser("SiteA");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckInstallIconShown();
  helper_.AfterStateCheckAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckLaunchIconNotShown();
  helper_.AfterStateCheckAction();
}

IN_PROC_BROWSER_TEST_F(
    WebAppIntegrationBrowserTestMacWinLinux,
    WebAppIntegration_InstCrtShctWindowedSiteA_WindowCreated_InListWinSiteA_UninstallFromMenuSiteA_NotInListSiteA_NavSiteA_InstIconShown_LaunchIconNotShown) {
  // Test contents are generated by script. Please do not modify!
  // See `chrome/test/webapps/README.md` for more info.
  // Sheriffs: Disabling this test is supported.
  helper_.BeforeStateChangeAction();
  helper_.InstallCreateShortcutWindowed("SiteA");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckWindowCreated();
  helper_.AfterStateCheckAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListWindowed("SiteA");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.UninstallFromMenu("SiteA");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppNotInList("SiteA");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.NavigateBrowser("SiteA");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckInstallIconShown();
  helper_.AfterStateCheckAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckLaunchIconNotShown();
  helper_.AfterStateCheckAction();
}

IN_PROC_BROWSER_TEST_F(
    WebAppIntegrationBrowserTestMacWinLinux,
    WebAppIntegration_InstCrtShctWindowedSiteC_InListWinSiteC_UninstallFromMenuSiteC_NotInListSiteA) {
  // Test contents are generated by script. Please do not modify!
  // See `chrome/test/webapps/README.md` for more info.
  // Sheriffs: Disabling this test is supported.
  helper_.BeforeStateChangeAction();
  helper_.InstallCreateShortcutWindowed("SiteC");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListWindowed("SiteC");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.UninstallFromMenu("SiteC");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppNotInList("SiteA");
  helper_.AfterStateCheckAction();
}

}  // namespace
}  // namespace web_app
