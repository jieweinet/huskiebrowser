// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_ARC_TEST_FAKE_MEMORY_INSTANCE_H_
#define COMPONENTS_ARC_TEST_FAKE_MEMORY_INSTANCE_H_

#include "base/callback.h"
#include "components/arc/mojom/memory.mojom.h"

namespace arc {

class FakeMemoryInstance : public mojom::MemoryInstance {
 public:
  FakeMemoryInstance();
  FakeMemoryInstance(const FakeMemoryInstance&) = delete;
  FakeMemoryInstance& operator=(const FakeMemoryInstance&) = delete;
  ~FakeMemoryInstance() override;

  void set_drop_caches_result(bool result) { drop_caches_result_ = result; }

  // mojom::MemoryInstance:
  void DropCaches(DropCachesCallback callback) override;

 private:
  bool drop_caches_result_ = true;
};

}  // namespace arc

#endif  // COMPONENTS_ARC_TEST_FAKE_MEMORY_INSTANCE_H_
