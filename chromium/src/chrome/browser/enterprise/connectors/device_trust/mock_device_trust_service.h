// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ENTERPRISE_CONNECTORS_DEVICE_TRUST_MOCK_DEVICE_TRUST_SERVICE_H_
#define CHROME_BROWSER_ENTERPRISE_CONNECTORS_DEVICE_TRUST_MOCK_DEVICE_TRUST_SERVICE_H_

#include "chrome/browser/enterprise/connectors/device_trust/device_trust_service.h"
#include "testing/gmock/include/gmock/gmock.h"

namespace enterprise_connectors {
namespace test {

class MockDeviceTrustService : public DeviceTrustService {
 public:
  MockDeviceTrustService();
  ~MockDeviceTrustService() override;

  MOCK_METHOD(bool, IsEnabled, (), (const, override));
  MOCK_METHOD(void,
              BuildChallengeResponse,
              (const std::string&, AttestationCallback),
              (override));
  MOCK_METHOD(base::CallbackListSubscription,
              RegisterTrustedUrlPatternsChangedCallback,
              (TrustedUrlPatternsChangedCallback),
              (override));
};

}  // namespace test
}  // namespace enterprise_connectors

#endif  // CHROME_BROWSER_ENTERPRISE_CONNECTORS_DEVICE_TRUST_MOCK_DEVICE_TRUST_SERVICE_H_
