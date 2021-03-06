// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/run_loop.h"
#include "chrome/browser/ash/policy/core/browser_policy_connector_ash.h"
#include "chrome/browser/ash/policy/core/device_cloud_policy_manager_ash.h"
#include "chrome/browser/ash/policy/core/device_cloud_policy_store_ash.h"
#include "chrome/browser/ash/policy/core/device_policy_cros_browser_test.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/browser_process_platform_part_chromeos.h"
#include "components/policy/core/common/cloud/mock_cloud_policy_store.h"
#include "components/policy/core/common/cloud/test/policy_builder.h"
#include "components/policy/proto/device_management_backend.pb.h"
#include "content/public/test/browser_test.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using testing::InvokeWithoutArgs;

namespace policy {

const char kCustomDisplayDomain[] = "acme.corp";
const char kMachineName[] = "machine_name";
const char kCustomManager[] = "user@acme.corp";

void WaitUntilPolicyLoaded() {
  BrowserPolicyConnectorAsh* connector =
      g_browser_process->platform_part()->browser_policy_connector_ash();
  DeviceCloudPolicyStoreAsh* store =
      connector->GetDeviceCloudPolicyManager()->device_store();
  if (!store->has_policy()) {
    MockCloudPolicyStoreObserver observer;
    base::RunLoop loop;
    store->AddObserver(&observer);
    EXPECT_CALL(observer, OnStoreLoaded(store))
        .Times(1)
        .WillOnce(InvokeWithoutArgs(&loop, &base::RunLoop::Quit));
    loop.Run();
    store->RemoveObserver(&observer);
  }
}

class BrowserPolicyConnectorAshTest : public DevicePolicyCrosBrowserTest {
 public:
  BrowserPolicyConnectorAshTest() {
    device_state_.set_skip_initial_policy_setup(true);
  }
  ~BrowserPolicyConnectorAshTest() override = default;
};

// Test that GetEnterpriseEnrollmentDomain and GetEnterpriseDisplayDomain work
// as expected.
IN_PROC_BROWSER_TEST_F(BrowserPolicyConnectorAshTest, EnterpriseDomains) {
  BrowserPolicyConnectorAsh* connector =
      g_browser_process->platform_part()->browser_policy_connector_ash();
  EXPECT_EQ(PolicyBuilder::kFakeDomain,
            connector->GetEnterpriseEnrollmentDomain());
  // Custom display domain not set at this point and policy not loaded yet so
  // display domain defaults to enrollment domain.
  EXPECT_EQ(PolicyBuilder::kFakeDomain,
            connector->GetEnterpriseDisplayDomain());
  // If no manager set, EnterpriseDomainManager is equal to
  // EnterpriseDisplayDomain
  EXPECT_EQ(connector->GetEnterpriseDisplayDomain(),
            connector->GetEnterpriseDomainManager());
  device_policy()->policy_data().set_display_domain(kCustomDisplayDomain);
  device_policy()->policy_data().set_managed_by(kCustomManager);
  RefreshDevicePolicy();
  WaitUntilPolicyLoaded();
  // At this point custom display domain is set and policy is loaded so expect
  // to see the custom display domain.
  EXPECT_EQ(kCustomDisplayDomain, connector->GetEnterpriseDisplayDomain());
  EXPECT_EQ(kCustomManager, connector->GetEnterpriseDomainManager());
  // Make sure that enrollment domain stays the same.
  EXPECT_EQ(PolicyBuilder::kFakeDomain,
            connector->GetEnterpriseEnrollmentDomain());
}

IN_PROC_BROWSER_TEST_F(BrowserPolicyConnectorAshTest, MarketSegment) {
  BrowserPolicyConnectorAsh* connector =
      g_browser_process->platform_part()->browser_policy_connector_ash();
  EXPECT_EQ(MarketSegment::UNKNOWN, connector->GetEnterpriseMarketSegment());

  device_policy()->policy_data().set_market_segment(
      enterprise_management::PolicyData::ENROLLED_EDUCATION);
  RefreshDevicePolicy();
  WaitUntilPolicyLoaded();
  EXPECT_EQ(MarketSegment::EDUCATION, connector->GetEnterpriseMarketSegment());
}

// Test that GetEnterpriseEnrollmentDomain and GetEnterpriseDisplayDomain work
// as expected.
IN_PROC_BROWSER_TEST_F(BrowserPolicyConnectorAshTest, MachineName) {
  BrowserPolicyConnectorAsh* connector =
      g_browser_process->platform_part()->browser_policy_connector_ash();
  EXPECT_EQ(std::string(), connector->GetMachineName());
  device_policy()->policy_data().set_machine_name(kMachineName);
  RefreshDevicePolicy();
  WaitUntilPolicyLoaded();
  // At this point custom display domain is set and policy is loaded so expect
  // to see the custom display domain.
  EXPECT_EQ(kMachineName, connector->GetMachineName());
}

}  // namespace policy
