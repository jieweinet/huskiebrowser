// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/sync/test/integration/two_client_web_apps_integration_test_base.h"
#include "content/public/test/browser_test.h"

namespace web_app {

// This test is a part of the web app integration test suite, which is
// documented in //chrome/browser/ui/views/web_apps/README.md. For information
// about diagnosing, debugging and/or disabling tests, please look to the
// README file.

namespace {

using TwoClientWebAppsIntegrationTestMacWinLinux =
    TwoClientWebAppsIntegrationTestBase;

IN_PROC_BROWSER_TEST_F(
    TwoClientWebAppsIntegrationTestMacWinLinux,
    WebAppIntegration_InstCrtShctWindowedSiteA_WindowCreated_InListWinSiteA_SwitchProfileClientClient2_InListNotLclyInstSiteA_InstLocallySiteA_InListWinSiteA_NavSiteA_InstIconNotShown_LaunchIconShown) {
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
  helper_.SwitchProfileClients("Client2");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListNotLocallyInstalled("SiteA");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.InstallLocally("SiteA");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListWindowed("SiteA");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.NavigateBrowser("SiteA");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckInstallIconNotShown();
  helper_.AfterStateCheckAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckLaunchIconShown();
  helper_.AfterStateCheckAction();
}

IN_PROC_BROWSER_TEST_F(
    TwoClientWebAppsIntegrationTestMacWinLinux,
    WebAppIntegration_InstCrtShctWindowedSiteA_WindowCreated_InListWinSiteA_SwitchProfileClientClient2_InListNotLclyInstSiteA_InstLocallySiteA_InListWinSiteA_LaunchFromListSiteA_WindowCreated) {
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
  helper_.SwitchProfileClients("Client2");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListNotLocallyInstalled("SiteA");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.InstallLocally("SiteA");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListWindowed("SiteA");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.LaunchFromChromeApps("SiteA");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckWindowCreated();
  helper_.AfterStateCheckAction();
}

IN_PROC_BROWSER_TEST_F(
    TwoClientWebAppsIntegrationTestMacWinLinux,
    WebAppIntegration_InstCrtShctWindowedSiteA_WindowCreated_InListWinSiteA_SwitchProfileClientClient2_InListNotLclyInstSiteA_LaunchFromListSiteA_TabCreated) {
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
  helper_.SwitchProfileClients("Client2");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListNotLocallyInstalled("SiteA");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.LaunchFromChromeApps("SiteA");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckTabCreated();
  helper_.AfterStateCheckAction();
}

IN_PROC_BROWSER_TEST_F(
    TwoClientWebAppsIntegrationTestMacWinLinux,
    WebAppIntegration_InstCrtShctWindowedSiteA_WindowCreated_InListWinSiteA_SwitchProfileClientClient2_InListNotLclyInstSiteA_NavSiteA_InstIconShown_LaunchIconNotShown) {
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
  helper_.SwitchProfileClients("Client2");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListNotLocallyInstalled("SiteA");
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
    TwoClientWebAppsIntegrationTestMacWinLinux,
    WebAppIntegration_InstCrtShctWindowedSiteA_WindowCreated_InListWinSiteA_SwitchProfileClientClient2_InListNotLclyInstSiteA_TurnSyncOff) {
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
  helper_.SwitchProfileClients("Client2");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListNotLocallyInstalled("SiteA");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.SyncTurnOff();
  helper_.AfterStateChangeAction();
}

IN_PROC_BROWSER_TEST_F(
    TwoClientWebAppsIntegrationTestMacWinLinux,
    WebAppIntegration_InstOmniboxSiteA_WindowCreated_InListWinSiteA_SwitchProfileClientClient2_InListNotLclyInstSiteA_InstLocallySiteA_InListWinSiteA_NavSiteA_InstIconNotShown_LaunchIconShown) {
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
  helper_.SwitchProfileClients("Client2");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListNotLocallyInstalled("SiteA");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.InstallLocally("SiteA");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListWindowed("SiteA");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.NavigateBrowser("SiteA");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckInstallIconNotShown();
  helper_.AfterStateCheckAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckLaunchIconShown();
  helper_.AfterStateCheckAction();
}

IN_PROC_BROWSER_TEST_F(
    TwoClientWebAppsIntegrationTestMacWinLinux,
    WebAppIntegration_InstOmniboxSiteA_WindowCreated_InListWinSiteA_SwitchProfileClientClient2_InListNotLclyInstSiteA_InstLocallySiteA_InListWinSiteA_LaunchFromListSiteA_WindowCreated) {
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
  helper_.SwitchProfileClients("Client2");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListNotLocallyInstalled("SiteA");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.InstallLocally("SiteA");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListWindowed("SiteA");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.LaunchFromChromeApps("SiteA");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckWindowCreated();
  helper_.AfterStateCheckAction();
}

IN_PROC_BROWSER_TEST_F(
    TwoClientWebAppsIntegrationTestMacWinLinux,
    WebAppIntegration_InstOmniboxSiteA_WindowCreated_InListWinSiteA_SwitchProfileClientClient2_InListNotLclyInstSiteA_LaunchFromListSiteA_TabCreated) {
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
  helper_.SwitchProfileClients("Client2");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListNotLocallyInstalled("SiteA");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.LaunchFromChromeApps("SiteA");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckTabCreated();
  helper_.AfterStateCheckAction();
}

IN_PROC_BROWSER_TEST_F(
    TwoClientWebAppsIntegrationTestMacWinLinux,
    WebAppIntegration_InstOmniboxSiteA_WindowCreated_InListWinSiteA_SwitchProfileClientClient2_InListNotLclyInstSiteA_NavSiteA_InstIconShown_LaunchIconNotShown) {
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
  helper_.SwitchProfileClients("Client2");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListNotLocallyInstalled("SiteA");
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
    TwoClientWebAppsIntegrationTestMacWinLinux,
    WebAppIntegration_InstOmniboxSiteA_WindowCreated_InListWinSiteA_SwitchProfileClientClient2_InListNotLclyInstSiteA_TurnSyncOff) {
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
  helper_.SwitchProfileClients("Client2");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListNotLocallyInstalled("SiteA");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.SyncTurnOff();
  helper_.AfterStateChangeAction();
}

IN_PROC_BROWSER_TEST_F(
    TwoClientWebAppsIntegrationTestMacWinLinux,
    WebAppIntegration_InstMenuOptionSiteA_WindowCreated_InListWinSiteA_SwitchProfileClientClient2_InListNotLclyInstSiteA_InstLocallySiteA_InListWinSiteA_NavSiteA_InstIconNotShown_LaunchIconShown) {
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
  helper_.SwitchProfileClients("Client2");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListNotLocallyInstalled("SiteA");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.InstallLocally("SiteA");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListWindowed("SiteA");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.NavigateBrowser("SiteA");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckInstallIconNotShown();
  helper_.AfterStateCheckAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckLaunchIconShown();
  helper_.AfterStateCheckAction();
}

IN_PROC_BROWSER_TEST_F(
    TwoClientWebAppsIntegrationTestMacWinLinux,
    WebAppIntegration_InstMenuOptionSiteA_WindowCreated_InListWinSiteA_SwitchProfileClientClient2_InListNotLclyInstSiteA_InstLocallySiteA_InListWinSiteA_LaunchFromListSiteA_WindowCreated) {
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
  helper_.SwitchProfileClients("Client2");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListNotLocallyInstalled("SiteA");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.InstallLocally("SiteA");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListWindowed("SiteA");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.LaunchFromChromeApps("SiteA");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckWindowCreated();
  helper_.AfterStateCheckAction();
}

IN_PROC_BROWSER_TEST_F(
    TwoClientWebAppsIntegrationTestMacWinLinux,
    WebAppIntegration_InstMenuOptionSiteA_WindowCreated_InListWinSiteA_SwitchProfileClientClient2_InListNotLclyInstSiteA_LaunchFromListSiteA_TabCreated) {
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
  helper_.SwitchProfileClients("Client2");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListNotLocallyInstalled("SiteA");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.LaunchFromChromeApps("SiteA");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckTabCreated();
  helper_.AfterStateCheckAction();
}

IN_PROC_BROWSER_TEST_F(
    TwoClientWebAppsIntegrationTestMacWinLinux,
    WebAppIntegration_InstMenuOptionSiteA_WindowCreated_InListWinSiteA_SwitchProfileClientClient2_InListNotLclyInstSiteA_NavSiteA_InstIconShown_LaunchIconNotShown) {
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
  helper_.SwitchProfileClients("Client2");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListNotLocallyInstalled("SiteA");
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
    TwoClientWebAppsIntegrationTestMacWinLinux,
    WebAppIntegration_InstMenuOptionSiteA_WindowCreated_InListWinSiteA_SwitchProfileClientClient2_InListNotLclyInstSiteA_TurnSyncOff) {
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
  helper_.SwitchProfileClients("Client2");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListNotLocallyInstalled("SiteA");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.SyncTurnOff();
  helper_.AfterStateChangeAction();
}
IN_PROC_BROWSER_TEST_F(
    TwoClientWebAppsIntegrationTestMacWinLinux,
    WebAppIntegration_InstCrtShctTabbedSiteA_InListTabbedSiteA_SwitchProfileClientClient2_InListNotLclyInstSiteA_InstLocallySiteA_InListTabbedSiteA_NavSiteA_InstIconShown_LaunchIconNotShown) {
  // Test contents are generated by script. Please do not modify!
  // See `chrome/test/webapps/README.md` for more info.
  // Sheriffs: Disabling this test is supported.
  helper_.BeforeStateChangeAction();
  helper_.InstallCreateShortcutTabbed("SiteA");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListTabbed("SiteA");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.SwitchProfileClients("Client2");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListNotLocallyInstalled("SiteA");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.InstallLocally("SiteA");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListTabbed("SiteA");
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
    TwoClientWebAppsIntegrationTestMacWinLinux,
    WebAppIntegration_InstCrtShctTabbedSiteA_InListTabbedSiteA_SwitchProfileClientClient2_InListNotLclyInstSiteA_InstLocallySiteA_InListTabbedSiteA_LaunchFromListSiteA_TabCreated) {
  // Test contents are generated by script. Please do not modify!
  // See `chrome/test/webapps/README.md` for more info.
  // Sheriffs: Disabling this test is supported.
  helper_.BeforeStateChangeAction();
  helper_.InstallCreateShortcutTabbed("SiteA");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListTabbed("SiteA");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.SwitchProfileClients("Client2");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListNotLocallyInstalled("SiteA");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.InstallLocally("SiteA");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListTabbed("SiteA");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.LaunchFromChromeApps("SiteA");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckTabCreated();
  helper_.AfterStateCheckAction();
}

IN_PROC_BROWSER_TEST_F(
    TwoClientWebAppsIntegrationTestMacWinLinux,
    WebAppIntegration_InstCrtShctTabbedSiteA_InListTabbedSiteA_SwitchProfileClientClient2_InListNotLclyInstSiteA_LaunchFromListSiteA_TabCreated) {
  // Test contents are generated by script. Please do not modify!
  // See `chrome/test/webapps/README.md` for more info.
  // Sheriffs: Disabling this test is supported.
  helper_.BeforeStateChangeAction();
  helper_.InstallCreateShortcutTabbed("SiteA");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListTabbed("SiteA");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.SwitchProfileClients("Client2");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListNotLocallyInstalled("SiteA");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.LaunchFromChromeApps("SiteA");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckTabCreated();
  helper_.AfterStateCheckAction();
}

IN_PROC_BROWSER_TEST_F(
    TwoClientWebAppsIntegrationTestMacWinLinux,
    WebAppIntegration_InstCrtShctTabbedSiteA_InListTabbedSiteA_SwitchProfileClientClient2_InListNotLclyInstSiteA_TurnSyncOff) {
  // Test contents are generated by script. Please do not modify!
  // See `chrome/test/webapps/README.md` for more info.
  // Sheriffs: Disabling this test is supported.
  helper_.BeforeStateChangeAction();
  helper_.InstallCreateShortcutTabbed("SiteA");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListTabbed("SiteA");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.SwitchProfileClients("Client2");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListNotLocallyInstalled("SiteA");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.SyncTurnOff();
  helper_.AfterStateChangeAction();
}

IN_PROC_BROWSER_TEST_F(
    TwoClientWebAppsIntegrationTestMacWinLinux,
    WebAppIntegration_InstCrtShctWindowedSiteC_InListWinSiteC_SwitchProfileClientClient2_InListNotLclyInstSiteC_InstLocallySiteA) {
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
  helper_.SwitchProfileClients("Client2");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListNotLocallyInstalled("SiteC");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.InstallLocally("SiteA");
  helper_.AfterStateChangeAction();
}

IN_PROC_BROWSER_TEST_F(
    TwoClientWebAppsIntegrationTestMacWinLinux,
    WebAppIntegration_InstCrtShctWindowedSiteC_InListWinSiteC_SwitchProfileClientClient2_InListNotLclyInstSiteC_InstLocallySiteC_InListWinSiteC_NavSiteC_InstIconNotShown_LaunchIconShown) {
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
  helper_.SwitchProfileClients("Client2");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListNotLocallyInstalled("SiteC");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.InstallLocally("SiteC");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListWindowed("SiteC");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.NavigateBrowser("SiteC");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckInstallIconNotShown();
  helper_.AfterStateCheckAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckLaunchIconShown();
  helper_.AfterStateCheckAction();
}

IN_PROC_BROWSER_TEST_F(
    TwoClientWebAppsIntegrationTestMacWinLinux,
    WebAppIntegration_InstCrtShctTabbedSiteC_InListTabbedSiteC_SwitchProfileClientClient2_InListNotLclyInstSiteC_InstLocallySiteA) {
  // Test contents are generated by script. Please do not modify!
  // See `chrome/test/webapps/README.md` for more info.
  // Sheriffs: Disabling this test is supported.
  helper_.BeforeStateChangeAction();
  helper_.InstallCreateShortcutTabbed("SiteC");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListTabbed("SiteC");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.SwitchProfileClients("Client2");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListNotLocallyInstalled("SiteC");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.InstallLocally("SiteA");
  helper_.AfterStateChangeAction();
}

IN_PROC_BROWSER_TEST_F(
    TwoClientWebAppsIntegrationTestMacWinLinux,
    WebAppIntegration_InstCrtShctTabbedSiteC_InListTabbedSiteC_SwitchProfileClientClient2_InListNotLclyInstSiteC_InstLocallySiteC_InListTabbedSiteC_NavSiteC_LaunchIconNotShown) {
  // Test contents are generated by script. Please do not modify!
  // See `chrome/test/webapps/README.md` for more info.
  // Sheriffs: Disabling this test is supported.
  helper_.BeforeStateChangeAction();
  helper_.InstallCreateShortcutTabbed("SiteC");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListTabbed("SiteC");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.SwitchProfileClients("Client2");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListNotLocallyInstalled("SiteC");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.InstallLocally("SiteC");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListTabbed("SiteC");
  helper_.AfterStateCheckAction();

  helper_.BeforeStateChangeAction();
  helper_.NavigateBrowser("SiteC");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckLaunchIconNotShown();
  helper_.AfterStateCheckAction();
}

IN_PROC_BROWSER_TEST_F(
    TwoClientWebAppsIntegrationTestMacWinLinux,
    WebAppIntegration_TurnSyncOff_InstCrtShctWindowedSiteA_TurnSyncOn_SwitchProfileClientClient2_InListNotLclyInstSiteA) {
  // Test contents are generated by script. Please do not modify!
  // See `chrome/test/webapps/README.md` for more info.
  // Sheriffs: Disabling this test is supported.
  helper_.BeforeStateChangeAction();
  helper_.SyncTurnOff();
  helper_.AfterStateChangeAction();

  helper_.BeforeStateChangeAction();
  helper_.InstallCreateShortcutWindowed("SiteA");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateChangeAction();
  helper_.SyncTurnOn();
  helper_.AfterStateChangeAction();

  helper_.BeforeStateChangeAction();
  helper_.SwitchProfileClients("Client2");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListNotLocallyInstalled("SiteA");
  helper_.AfterStateCheckAction();
}

IN_PROC_BROWSER_TEST_F(
    TwoClientWebAppsIntegrationTestMacWinLinux,
    WebAppIntegration_TurnSyncOff_InstOmniboxSiteA_TurnSyncOn_SwitchProfileClientClient2_InListNotLclyInstSiteA) {
  // Test contents are generated by script. Please do not modify!
  // See `chrome/test/webapps/README.md` for more info.
  // Sheriffs: Disabling this test is supported.
  helper_.BeforeStateChangeAction();
  helper_.SyncTurnOff();
  helper_.AfterStateChangeAction();

  helper_.BeforeStateChangeAction();
  helper_.InstallOmniboxIcon("SiteA");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateChangeAction();
  helper_.SyncTurnOn();
  helper_.AfterStateChangeAction();

  helper_.BeforeStateChangeAction();
  helper_.SwitchProfileClients("Client2");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListNotLocallyInstalled("SiteA");
  helper_.AfterStateCheckAction();
}

IN_PROC_BROWSER_TEST_F(
    TwoClientWebAppsIntegrationTestMacWinLinux,
    WebAppIntegration_TurnSyncOff_InstMenuOptionSiteA_TurnSyncOn_SwitchProfileClientClient2_InListNotLclyInstSiteA) {
  // Test contents are generated by script. Please do not modify!
  // See `chrome/test/webapps/README.md` for more info.
  // Sheriffs: Disabling this test is supported.
  helper_.BeforeStateChangeAction();
  helper_.SyncTurnOff();
  helper_.AfterStateChangeAction();

  helper_.BeforeStateChangeAction();
  helper_.InstallMenuOption("SiteA");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateChangeAction();
  helper_.SyncTurnOn();
  helper_.AfterStateChangeAction();

  helper_.BeforeStateChangeAction();
  helper_.SwitchProfileClients("Client2");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListNotLocallyInstalled("SiteA");
  helper_.AfterStateCheckAction();
}

IN_PROC_BROWSER_TEST_F(
    TwoClientWebAppsIntegrationTestMacWinLinux,
    WebAppIntegration_TurnSyncOff_InstCrtShctTabbedSiteA_TurnSyncOn_SwitchProfileClientClient2_InListNotLclyInstSiteA) {
  // Test contents are generated by script. Please do not modify!
  // See `chrome/test/webapps/README.md` for more info.
  // Sheriffs: Disabling this test is supported.
  helper_.BeforeStateChangeAction();
  helper_.SyncTurnOff();
  helper_.AfterStateChangeAction();

  helper_.BeforeStateChangeAction();
  helper_.InstallCreateShortcutTabbed("SiteA");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateChangeAction();
  helper_.SyncTurnOn();
  helper_.AfterStateChangeAction();

  helper_.BeforeStateChangeAction();
  helper_.SwitchProfileClients("Client2");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListNotLocallyInstalled("SiteA");
  helper_.AfterStateCheckAction();
}

IN_PROC_BROWSER_TEST_F(
    TwoClientWebAppsIntegrationTestMacWinLinux,
    WebAppIntegration_TurnSyncOff_InstCrtShctWindowedSiteC_TurnSyncOn_SwitchProfileClientClient2_InListNotLclyInstSiteC) {
  // Test contents are generated by script. Please do not modify!
  // See `chrome/test/webapps/README.md` for more info.
  // Sheriffs: Disabling this test is supported.
  helper_.BeforeStateChangeAction();
  helper_.SyncTurnOff();
  helper_.AfterStateChangeAction();

  helper_.BeforeStateChangeAction();
  helper_.InstallCreateShortcutWindowed("SiteC");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateChangeAction();
  helper_.SyncTurnOn();
  helper_.AfterStateChangeAction();

  helper_.BeforeStateChangeAction();
  helper_.SwitchProfileClients("Client2");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListNotLocallyInstalled("SiteC");
  helper_.AfterStateCheckAction();
}

IN_PROC_BROWSER_TEST_F(
    TwoClientWebAppsIntegrationTestMacWinLinux,
    WebAppIntegration_TurnSyncOff_InstCrtShctTabbedSiteC_TurnSyncOn_SwitchProfileClientClient2_InListNotLclyInstSiteC) {
  // Test contents are generated by script. Please do not modify!
  // See `chrome/test/webapps/README.md` for more info.
  // Sheriffs: Disabling this test is supported.
  helper_.BeforeStateChangeAction();
  helper_.SyncTurnOff();
  helper_.AfterStateChangeAction();

  helper_.BeforeStateChangeAction();
  helper_.InstallCreateShortcutTabbed("SiteC");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateChangeAction();
  helper_.SyncTurnOn();
  helper_.AfterStateChangeAction();

  helper_.BeforeStateChangeAction();
  helper_.SwitchProfileClients("Client2");
  helper_.AfterStateChangeAction();

  helper_.BeforeStateCheckAction();
  helper_.CheckAppInListNotLocallyInstalled("SiteC");
  helper_.AfterStateCheckAction();
}

}  // namespace
}  // namespace web_app