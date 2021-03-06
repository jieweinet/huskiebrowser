// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_COMPONENTS_ECHE_APP_UI_FAKE_ECHE_CONNECTOR_H_
#define CHROMEOS_COMPONENTS_ECHE_APP_UI_FAKE_ECHE_CONNECTOR_H_

#include "chromeos/components/eche_app_ui/eche_connector.h"

namespace chromeos {
namespace eche_app {

class FakeEcheConnector : public EcheConnector {
 public:
  FakeEcheConnector();
  ~FakeEcheConnector() override;

  size_t send_apps_setup_request_count() const {
    return send_apps_setup_request_count_;
  }

  size_t get_apps_access_state_request_count() const {
    return get_apps_access_state_request_count_;
  }

  size_t attempt_nearby_connection_count() const {
    return attempt_nearby_connection_count_;
  }

  void SendMessage(const std::string& message) override;
  void Disconnect() override;
  void SendAppsSetupRequest() override;
  void GetAppsAccessStateRequest() override;
  void AttemptNearbyConnection() override;

 private:
  size_t send_apps_setup_request_count_ = 0;
  size_t get_apps_access_state_request_count_ = 0;
  size_t attempt_nearby_connection_count_ = 0;
};

}  // namespace eche_app
}  // namespace chromeos

#endif  // CHROMEOS_COMPONENTS_ECHE_APP_UI_FAKE_ECHE_CONNECTOR_H_
