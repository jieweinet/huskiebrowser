// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_ARC_TEST_FAKE_ADBD_MONITOR_INSTANCE_H_
#define COMPONENTS_ARC_TEST_FAKE_ADBD_MONITOR_INSTANCE_H_

#include "components/arc/mojom/adbd.mojom.h"
#include "components/arc/session/arc_bridge_service.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "mojo/public/cpp/bindings/remote.h"

namespace arc {

class FakeAdbdMonitorInstance : public mojom::AdbdMonitorInstance {
 public:
  FakeAdbdMonitorInstance();
  ~FakeAdbdMonitorInstance() override;

  FakeAdbdMonitorInstance(const FakeAdbdMonitorInstance&) = delete;
  FakeAdbdMonitorInstance& operator=(const FakeAdbdMonitorInstance&) = delete;

  // mojom::AdbdMonitorInstance overrides:
  void Init(mojo::PendingRemote<mojom::AdbdMonitorHost> host_remote,
            InitCallback callback) override;

 private:
  mojo::Remote<mojom::AdbdMonitorHost> host_remote_;
};

}  // namespace arc

#endif  // COMPONENTS_ARC_TEST_FAKE_ADBD_MONITOR_INSTANCE_H_
