// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_GCM_DRIVER_GCM_CLIENT_FACTORY_H_
#define COMPONENTS_GCM_DRIVER_GCM_CLIENT_FACTORY_H_

#include <memory>

#include "base/macros.h"

namespace gcm {

class GCMClient;

class GCMClientFactory {
 public:
  GCMClientFactory();

  GCMClientFactory(const GCMClientFactory&) = delete;
  GCMClientFactory& operator=(const GCMClientFactory&) = delete;

  virtual ~GCMClientFactory();

  // Creates a new instance of GCMClient. The testing code could override this
  // to provide a mocked instance.
  virtual std::unique_ptr<GCMClient> BuildInstance();
};

}  // namespace gcm

#endif  // COMPONENTS_GCM_DRIVER_GCM_CLIENT_FACTORY_H_
