// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_ARC_TEST_FAKE_POLICY_INSTANCE_H_
#define COMPONENTS_ARC_TEST_FAKE_POLICY_INSTANCE_H_

#include <string>

#include "components/arc/mojom/policy.mojom.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "mojo/public/cpp/bindings/remote.h"

namespace arc {

class FakePolicyInstance : public mojom::PolicyInstance {
 public:
  FakePolicyInstance();

  FakePolicyInstance(const FakePolicyInstance&) = delete;
  FakePolicyInstance& operator=(const FakePolicyInstance&) = delete;

  ~FakePolicyInstance() override;

  // mojom::PolicyInstance
  void InitDeprecated(
      mojo::PendingRemote<mojom::PolicyHost> host_remote) override;
  void Init(mojo::PendingRemote<mojom::PolicyHost> host_remote,
            InitCallback callback) override;
  void OnPolicyUpdated() override;
  void OnCommandReceived(const std::string& command,
                         OnCommandReceivedCallback callback) override;

  void CallGetPolicies(mojom::PolicyHost::GetPoliciesCallback callback);

  const std::string& command_payload() { return command_payload_; }

 private:
  mojo::Remote<mojom::PolicyHost> host_remote_;

  std::string command_payload_;
};

}  // namespace arc

#endif  // COMPONENTS_ARC_TEST_FAKE_POLICY_INSTANCE_H_
