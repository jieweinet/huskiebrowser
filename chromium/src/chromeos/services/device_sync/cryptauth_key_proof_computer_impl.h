// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_SERVICES_DEVICE_SYNC_CRYPTAUTH_KEY_PROOF_COMPUTER_IMPL_H_
#define CHROMEOS_SERVICES_DEVICE_SYNC_CRYPTAUTH_KEY_PROOF_COMPUTER_IMPL_H_

#include <memory>
#include <string>

#include "base/macros.h"
#include "chromeos/services/device_sync/cryptauth_key_proof_computer.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

namespace chromeos {

namespace device_sync {

class CryptAuthKey;

class CryptAuthKeyProofComputerImpl : public CryptAuthKeyProofComputer {
 public:
  class Factory {
   public:
    static std::unique_ptr<CryptAuthKeyProofComputer> Create();
    static void SetFactoryForTesting(Factory* test_factory);

   protected:
    virtual ~Factory();
    virtual std::unique_ptr<CryptAuthKeyProofComputer> CreateInstance() = 0;

   private:
    static Factory* test_factory_;
  };

  ~CryptAuthKeyProofComputerImpl() override;

  // CryptAuthKeyProofComputer:
  absl::optional<std::string> ComputeKeyProof(
      const CryptAuthKey& key,
      const std::string& payload,
      const std::string& salt,
      const absl::optional<std::string>& info) override;

 private:
  CryptAuthKeyProofComputerImpl();

  absl::optional<std::string> ComputeSymmetricKeyProof(
      const CryptAuthKey& symmetric_key,
      const std::string& payload,
      const std::string& salt,
      const std::string& info);
  absl::optional<std::string> ComputeAsymmetricKeyProof(
      const CryptAuthKey& asymmetric_key,
      const std::string& payload,
      const std::string& salt);

  DISALLOW_COPY_AND_ASSIGN(CryptAuthKeyProofComputerImpl);
};

}  // namespace device_sync

}  // namespace chromeos

#endif  // CHROMEOS_SERVICES_DEVICE_SYNC_CRYPTAUTH_KEY_PROOF_COMPUTER_IMPL_H_
