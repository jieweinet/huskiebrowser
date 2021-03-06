// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMECAST_CAST_CORE_CAST_RUNTIME_HISTOGRAM_FLATTENER_H_
#define CHROMECAST_CAST_CORE_CAST_RUNTIME_HISTOGRAM_FLATTENER_H_

#include <vector>

#include "third_party/cast_core/public/src/proto/metrics/metrics_recorder.pb.h"

namespace chromecast {

// Returns the current uncollected histogram samples.
std::vector<cast::metrics::Histogram> GetHistogramDeltas();

}  // namespace chromecast

#endif  // CHROMECAST_CAST_CORE_CAST_RUNTIME_HISTOGRAM_FLATTENER_H_
