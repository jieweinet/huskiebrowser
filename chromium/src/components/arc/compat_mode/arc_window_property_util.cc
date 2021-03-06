// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/arc/compat_mode/arc_window_property_util.h"

#include <string>

#include "ash/public/cpp/window_properties.h"
#include "base/stl_util.h"
#include "ui/aura/window.h"
#include "ui/views/widget/widget.h"

namespace arc {

absl::optional<std::string> GetAppId(const aura::Window* window) {
  const std::string* app_id = window->GetProperty(ash::kAppIDKey);
  return base::OptionalFromPtr(app_id);
}

absl::optional<std::string> GetAppId(const views::Widget* widget) {
  return GetAppId(widget->GetNativeWindow());
}

}  // namespace arc
