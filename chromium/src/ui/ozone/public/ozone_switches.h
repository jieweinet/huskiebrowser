// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_OZONE_PUBLIC_OZONE_SWITCHES_H_
#define UI_OZONE_PUBLIC_OZONE_SWITCHES_H_

#include "base/compiler_specific.h"
#include "base/component_export.h"

namespace switches {

COMPONENT_EXPORT(OZONE_BASE) extern const char kOzonePlatform[];

COMPONENT_EXPORT(OZONE_BASE) extern const char kOzonePlatformHint[];

COMPONENT_EXPORT(OZONE_BASE) extern const char kOzoneDumpFile[];

COMPONENT_EXPORT(OZONE_BASE) extern const char kEnableWaylandIme[];

COMPONENT_EXPORT(OZONE_BASE) extern const char kDisableWaylandIme[];

COMPONENT_EXPORT(OZONE_BASE) extern const char kUseWaylandExplicitGrab[];

COMPONENT_EXPORT(OZONE_BASE) extern const char kDisableExplicitDmaFences[];

COMPONENT_EXPORT(OZONE_BASE)
extern const char kDisableRunningAsSystemCompositor[];

COMPONENT_EXPORT(OZONE_BASE) extern const char kDisableBufferBWCompression[];

COMPONENT_EXPORT(OZONE_BASE) extern const char kOzoneOverrideScreenSize[];

}  // namespace switches

#endif  // UI_OZONE_PUBLIC_OZONE_SWITCHES_H_
