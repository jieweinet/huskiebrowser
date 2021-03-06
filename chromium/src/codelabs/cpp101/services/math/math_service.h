// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CODELABS_CPP101_SERVICES_MATH_MATH_SERVICE_H_
#define CODELABS_CPP101_SERVICES_MATH_MATH_SERVICE_H_

#include "base/macros.h"
#include "codelabs/cpp101/services/math/public/mojom/math_service.mojom.h"
#include "mojo/public/cpp/bindings/receiver.h"

namespace math {

class MathService : public mojom::MathService {
 public:
  explicit MathService(mojo::PendingReceiver<mojom::MathService> receiver);
  ~MathService() override;
  MathService(const MathService&) = delete;
  MathService& operator=(const MathService&) = delete;

 private:
  // mojom::MathService:
  void Divide(int32_t dividend,
              int32_t divisor,
              DivideCallback callback) override;

  mojo::Receiver<mojom::MathService> receiver_;
};

}  // namespace math

#endif  // CODELABS_CPP101_SERVICES_MATH_MATH_SERVICE_H_