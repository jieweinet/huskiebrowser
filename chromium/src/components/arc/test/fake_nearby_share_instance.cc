// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/arc/test/fake_nearby_share_instance.h"

#include "base/callback_helpers.h"
#include "components/arc/mojom/nearby_share.mojom.h"

namespace arc {

FakeNearbyShareInstance::FakeNearbyShareInstance() = default;

FakeNearbyShareInstance::~FakeNearbyShareInstance() = default;

void FakeNearbyShareInstance::Init(
    mojo::PendingRemote<mojom::NearbyShareHost> host_remote,
    InitCallback callback) {
  ++num_init_called_;
  // For every change in a connection bind latest remote.
  host_remote_.reset();
  host_remote_.Bind(std::move(host_remote));
  std::move(callback).Run();
}

}  // namespace arc
