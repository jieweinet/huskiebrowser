// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/web_applications/web_app_file_handler_registration.h"

#include <iterator>
#include <set>
#include <string>

#include "base/callback_helpers.h"
#include "base/command_line.h"
#include "base/files/file_util.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/post_task.h"
#include "base/task/thread_pool.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/web_applications/web_app_constants.h"
#include "chrome/browser/web_applications/web_app_handler_registration_utils_win.h"
#include "chrome/installer/util/shell_util.h"
#include "components/services/app_service/public/cpp/file_handler.h"

namespace web_app {

bool ShouldRegisterFileHandlersWithOs() {
  return true;
}

bool FileHandlingIconsSupportedByOs() {
  // TODO(crbug/1218234): implement and flip this to true.
  return false;
}

void RegisterFileHandlersWithOsTask(
    const AppId& app_id,
    const std::wstring& app_name,
    const base::FilePath& profile_path,
    const std::set<std::wstring>& file_extensions,
    const std::wstring& app_name_extension) {
  const base::FilePath web_app_path =
      GetOsIntegrationResourcesDirectoryForApp(profile_path, app_id, GURL());
  absl::optional<base::FilePath> app_specific_launcher_path =
      CreateAppLauncherFile(app_name, app_name_extension, web_app_path);
  if (!app_specific_launcher_path.has_value())
    return;

  const base::CommandLine app_specific_launcher_command = GetAppLauncherCommand(
      app_id, app_specific_launcher_path.value(), profile_path);

  std::wstring user_visible_app_name(app_name);
  user_visible_app_name.append(app_name_extension);

  const base::FilePath icon_path =
      internals::GetIconFilePath(web_app_path, base::AsString16(app_name));

  bool result = ShellUtil::AddFileAssociations(
      GetProgIdForApp(profile_path, app_id), app_specific_launcher_command,
      user_visible_app_name, L"", icon_path, icon_path, file_extensions);
  if (!result)
    RecordRegistration(RegistrationResult::kFailToAddFileAssociation);
  else
    RecordRegistration(RegistrationResult::kSuccess);
}

void RegisterFileHandlersWithOs(const AppId& app_id,
                                const std::string& app_name,
                                Profile* profile,
                                const apps::FileHandlers& file_handlers) {
  DCHECK(!file_handlers.empty());

  const std::wstring app_name_extension =
      GetAppNameExtensionForNextInstall(app_id, profile->GetPath());

  const std::set<std::string> file_extensions =
      apps::GetFileExtensionsFromFileHandlers(file_handlers);
  std::set<std::wstring> file_extensions_wide;
  for (const auto& file_extension : file_extensions) {
    // The file extensions in apps::FileHandler include a '.' prefix, which must
    // be removed.
    file_extensions_wide.insert(base::UTF8ToWide(file_extension.substr(1)));
  }

  base::ThreadPool::PostTaskAndReply(
      FROM_HERE,
      {base::MayBlock(), base::TaskShutdownBehavior::SKIP_ON_SHUTDOWN},
      base::BindOnce(&RegisterFileHandlersWithOsTask, app_id,
                     base::UTF8ToWide(app_name), profile->GetPath(),
                     file_extensions_wide, app_name_extension),
      base::BindOnce(&CheckAndUpdateExternalInstallations, profile->GetPath(),
                     app_id, base::DoNothing()));
}

void DeleteAppLauncher(const base::FilePath& launcher_path) {
  // Need to delete the app launcher file, since extension uninstall by default
  // doesn't remove the web application directory.
  base::DeleteFile(launcher_path);
}

void UnregisterFileHandlersWithOs(const AppId& app_id,
                                  Profile* profile,
                                  ResultCallback callback) {
  // The app-specific-launcher file name must be calculated before cleaning up
  // the registry, since the app-specific-launcher path is retrieved from the
  // registry.
  const std::wstring prog_id = GetProgIdForApp(profile->GetPath(), app_id);
  const base::FilePath app_specific_launcher_path =
      ShellUtil::GetApplicationPathForProgId(prog_id);
  // This needs to be done synchronously. If it's done via a task, protocol
  // unregistration will delete HKCU\Classes\<progid> before the task runs.
  // Information in that key is needed to unregister file associations.
  ShellUtil::DeleteFileAssociations(prog_id);

  base::ThreadPool::PostTaskAndReply(
      FROM_HERE,
      {base::MayBlock(), base::TaskShutdownBehavior::CONTINUE_ON_SHUTDOWN},
      base::BindOnce(&DeleteAppLauncher, app_specific_launcher_path),
      base::BindOnce(&CheckAndUpdateExternalInstallations, profile->GetPath(),
                     app_id, std::move(callback)));
}

}  // namespace web_app
