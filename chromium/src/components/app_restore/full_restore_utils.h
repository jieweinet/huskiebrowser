// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_APP_RESTORE_FULL_RESTORE_UTILS_H_
#define COMPONENTS_APP_RESTORE_FULL_RESTORE_UTILS_H_

#include <memory>

#include "base/component_export.h"
#include "ui/base/class_property.h"
#include "ui/views/widget/widget.h"

namespace app_restore {
struct AppLaunchInfo;
struct WindowInfo;
}  // namespace app_restore

namespace aura {
class Window;
}

namespace base {
class FilePath;
}

namespace full_restore {

// Saves the app launch parameters to the full restore file.
COMPONENT_EXPORT(APP_RESTORE)
void SaveAppLaunchInfo(
    const base::FilePath& profile_path,
    std::unique_ptr<app_restore::AppLaunchInfo> app_launch_info);

// Saves the window information to the full restore file.
COMPONENT_EXPORT(APP_RESTORE)
void SaveWindowInfo(const app_restore::WindowInfo& window_info);

// Fetches the restore window id from the restore data for the given |app_id|.
// |app_id| should be a Chrome app id.
COMPONENT_EXPORT(APP_RESTORE)
int32_t FetchRestoreWindowId(const std::string& app_id);

// Returns the restore window id for the ARC app's |task_id|.
COMPONENT_EXPORT(APP_RESTORE)
int32_t GetArcRestoreWindowIdForTaskId(int32_t task_id);

// Returns the restore window id for the ARC app's |session_id|.
COMPONENT_EXPORT(APP_RESTORE)
int32_t GetArcRestoreWindowIdForSessionId(int32_t session_id);

// Sets the current active profile path.
COMPONENT_EXPORT(APP_RESTORE)
void SetActiveProfilePath(const base::FilePath& profile_path);

// Returns true if there are app type browsers from the full restore file.
// Otherwise, returns false.
COMPONENT_EXPORT(APP_RESTORE)
bool HasAppTypeBrowser(const base::FilePath& profile_path);

// Returns true if there are normal browsers from the full restore file.
// Otherwise, returns false.
COMPONENT_EXPORT(APP_RESTORE)
bool HasBrowser(const base::FilePath& profile_path);

// Returns true if there is a window info for |restore_window_id| from the full
// restore file. Otherwise, returns false. This interface can't be used for Arc
// app windows.
COMPONENT_EXPORT(APP_RESTORE)
bool HasWindowInfo(int32_t restore_window_id);

COMPONENT_EXPORT(APP_RESTORE)
void AddChromeBrowserLaunchInfoForTesting(const base::FilePath& profile_path);

// Returns the full restore app id that's associated with |window|. Note this
// might be different with the window's kAppIdKey property value. kAppIdKey
// is only non-empty for Chrome apps and ARC apps, but empty for other apps (for
// example, browsers, PWAs, SWAs). This function however guarantees to return a
// valid app id value for a window that can be restored by full restore.
COMPONENT_EXPORT(APP_RESTORE)
std::string GetAppId(aura::Window* window);

}  // namespace full_restore

#endif  // COMPONENTS_APP_RESTORE_FULL_RESTORE_UTILS_H_
