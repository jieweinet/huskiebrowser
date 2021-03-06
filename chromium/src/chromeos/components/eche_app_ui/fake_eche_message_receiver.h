// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_COMPONENTS_ECHE_APP_UI_FAKE_ECHE_MESSAGE_RECEIVER_H_
#define CHROMEOS_COMPONENTS_ECHE_APP_UI_FAKE_ECHE_MESSAGE_RECEIVER_H_

#include "chromeos/components/eche_app_ui/eche_message_receiver.h"

namespace chromeos {
namespace eche_app {

class FakeEcheMessageReceiver : public EcheMessageReceiver {
 public:
  FakeEcheMessageReceiver();
  ~FakeEcheMessageReceiver() override;

  void FakeGetAppsAccessStateResponse(proto::Result result,
                                      proto::AppsAccessState status);
  void FakeSendAppsSetupResponse(proto::Result result,
                                 proto::AppsAccessState status);
  void FakeStatusChange(proto::StatusChangeType status_change_type);

 private:
  using EcheMessageReceiver::NotifyGetAppsAccessStateResponse;
  using EcheMessageReceiver::NotifySendAppsSetupResponse;
  using EcheMessageReceiver::NotifyStatusChange;
};

}  // namespace eche_app
}  // namespace chromeos

#endif  // CHROMEOS_COMPONENTS_ECHE_APP_UI_FAKE_ECHE_MESSAGE_RECEIVER_H_
