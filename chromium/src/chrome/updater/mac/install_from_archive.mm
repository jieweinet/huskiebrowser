// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/updater/mac/install_from_archive.h"

#import <Cocoa/Cocoa.h>

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "base/bind.h"
#include "base/callback.h"
#include "base/files/file_enumerator.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/mac/scoped_nsobject.h"
#include "base/path_service.h"
#include "base/process/launch.h"
#include "base/process/process.h"
#include "base/strings/strcat.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/strings/sys_string_conversions.h"
#include "chrome/updater/mac/mac_util.h"
#include "chrome/updater/util.h"

namespace updater {
namespace {

constexpr int kPermissionsMask = base::FILE_PERMISSION_USER_MASK |
                                 base::FILE_PERMISSION_GROUP_MASK |
                                 base::FILE_PERMISSION_READ_BY_OTHERS |
                                 base::FILE_PERMISSION_EXECUTE_BY_OTHERS;

bool RunHDIUtil(const std::vector<std::string>& args,
                std::string* command_output) {
  base::FilePath hdiutil_path("/usr/bin/hdiutil");
  if (!base::PathExists(hdiutil_path)) {
    VLOG(1) << "hdiutil path (" << hdiutil_path << ") does not exist.";
    return false;
  }

  base::CommandLine command(hdiutil_path);
  for (const auto& arg : args)
    command.AppendArg(arg);

  std::string output;
  bool result = base::GetAppOutputAndError(command, &output);
  if (!result)
    VLOG(1) << "hdiutil failed.";

  if (command_output)
    *command_output = output;

  return result;
}

bool MountDMG(const base::FilePath& dmg_path, std::string* mount_point) {
  if (!base::PathExists(dmg_path)) {
    VLOG(1) << "The DMG file path (" << dmg_path << ") does not exist.";
    return false;
  }

  std::string command_output;
  std::vector<std::string> args{"attach", dmg_path.value(), "-plist",
                                "-nobrowse", "-readonly"};
  if (!RunHDIUtil(args, &command_output)) {
    VLOG(1) << "Mounting DMG (" << dmg_path
            << ") failed. Output: " << command_output;
    return false;
  }
  @autoreleasepool {
    NSString* output = base::SysUTF8ToNSString(command_output);
    NSDictionary* plist = [output propertyList];
    // Look for the mountpoint.
    NSArray* system_entities = [plist objectForKey:@"system-entities"];
    NSString* dmg_mount_point = nil;
    for (NSDictionary* entry in system_entities) {
      NSString* entry_mount_point = entry[@"mount-point"];
      if ([entry_mount_point length]) {
        dmg_mount_point = [entry_mount_point stringByStandardizingPath];
        break;
      }
    }
    if (mount_point)
      *mount_point = base::SysNSStringToUTF8(dmg_mount_point);
  }
  return true;
}

bool UnmountDMG(const base::FilePath& mounted_dmg_path) {
  if (!base::PathExists(mounted_dmg_path)) {
    VLOG(1) << "The mounted DMG path (" << mounted_dmg_path
            << ") does not exist.";
    return false;
  }

  std::vector<std::string> args{"detach", mounted_dmg_path.value(), "-force"};
  if (!RunHDIUtil(args, nullptr)) {
    VLOG(1) << "Unmounting DMG (" << mounted_dmg_path << ") failed.";
    return false;
  }
  return true;
}

bool IsInstallScriptExecutable(const base::FilePath& script_path) {
  int permissions = 0;
  if (!base::GetPosixFilePermissions(script_path, &permissions))
    return false;

  constexpr int kExecutableMask = base::FILE_PERMISSION_EXECUTE_BY_USER;
  return (permissions & kExecutableMask) == kExecutableMask;
}

struct ExecutableFile {
  // The file basename to execute.
  std::string name;

  // True if and only if installation should fail if the file is missing.
  bool is_required = false;
};

int RunExecutable(const base::FilePath& existence_checker_path,
                  const std::string& ap,
                  const std::string& arguments,
                  const base::FilePath& unpacked_path) {
  if (!base::PathExists(unpacked_path)) {
    VLOG(1) << "File path (" << unpacked_path << ") does not exist.";
    return static_cast<int>(InstallErrors::kMountedDmgPathDoesNotExist);
  }
  for (const auto& executable : std::vector<ExecutableFile>{
           {".preinstall", false},
           {".install", true},
           {".postinstall", false},
       }) {
    base::FilePath executable_file_path = unpacked_path.Append(executable.name);
    if (!base::PathExists(executable_file_path)) {
      if (!executable.is_required)
        continue;
      VLOG(1) << "Executable file path (" << executable_file_path
              << ") does not exist.";
      return static_cast<int>(InstallErrors::kExecutableFilePathDoesNotExist);
    }

    if (!IsInstallScriptExecutable(executable_file_path)) {
      VLOG(1) << "Executable file path (" << executable_file_path
              << ") is not executable";
      return static_cast<int>(InstallErrors::kExecutablePathNotExecutable);
    }

    // TODO(crbug.com/1056818): Improve the way we parse args for CommandLine
    // object.
    base::CommandLine command(executable_file_path);
    command.AppendArgPath(unpacked_path);
    if (!arguments.empty()) {
      base::CommandLine::StringVector argv =
          base::SplitString(arguments, base::kWhitespaceASCII,
                            base::TRIM_WHITESPACE, base::SPLIT_WANT_NONEMPTY);
      argv.insert(argv.begin(), existence_checker_path.value());
      argv.insert(argv.begin(), unpacked_path.value());
      argv.insert(argv.begin(), executable_file_path.value());
      command = base::CommandLine(argv);
    }

    std::string env_path = "/bin:/usr/bin";
    absl::optional<base::FilePath> ksadmin_path =
        GetKSAdminPath(GetUpdaterScope());
    if (ksadmin_path) {
      env_path = base::StrCat({env_path, ":", (*ksadmin_path).value()});
    }

    base::LaunchOptions options;
    options.current_directory = unpacked_path;
    options.clear_environment = true;
    options.environment = {
        {"PATH", env_path},
        {"KS_TICKET_XC_PATH", existence_checker_path.value()},
        {"KS_TICKET_AP", ap},
    };
    int exit_code = 0;
    if (!base::LaunchProcess(command, options).WaitForExit(&exit_code))
      return static_cast<int>(InstallErrors::kExecutableWaitForExitFailed);
    if (exit_code != 0)
      return exit_code;
  }
  return 0;
}

base::FilePath AlterFileExtension(const base::FilePath& path,
                                  const std::string& extension) {
  return path.RemoveExtension().AddExtension(extension);
}

void CopyDMGContents(const base::FilePath& dmg_path,
                     const base::FilePath& destination_path) {
  base::FileEnumerator file_enumerator(
      dmg_path, false,
      base::FileEnumerator::FILES | base::FileEnumerator::DIRECTORIES);

  for (base::FilePath path = file_enumerator.Next(); !path.empty();
       path = file_enumerator.Next()) {
    base::File::Info file_info;
    if (!base::GetFileInfo(path, &file_info)) {
      VLOG(0) << "Couldn't get file info for: " << path.value();
      continue;
    }

    if (base::IsLink(path)) {
      VLOG(0) << "File is symbolic link: " << path.value();
      continue;
    }

    if (file_info.is_directory) {
      if (!base::CopyDirectory(path, destination_path, true)) {
        VLOG(0) << "Couldn't copy directory for: " << path.value() << " to "
                << destination_path.value();
        continue;
      }
    } else {
      if (!base::CopyFile(path, destination_path.Append(path.BaseName()))) {
        VLOG(0) << "Couldn't copy file for: " << path.value() << " to "
                << destination_path.value();
        continue;
      }
    }
  }
}

// Mounts the DMG specified by `dmg_file_path`. The install executable located
// at "/.install" in the mounted volume is executed, and then the DMG is
// un-mounted. Returns an error code if mounting the DMG or executing the
// executable failed.
int InstallFromDMG(const base::FilePath& dmg_file_path,
                   base::OnceCallback<int(const base::FilePath&)> install) {
  std::string mount_point;
  if (!MountDMG(dmg_file_path, &mount_point))
    return static_cast<int>(InstallErrors::kFailMountDmg);

  if (mount_point.empty()) {
    VLOG(1) << "No mount point.";
    return static_cast<int>(InstallErrors::kNoMountPoint);
  }
  const base::FilePath mounted_dmg_path = base::FilePath(mount_point);
  const int result = std::move(install).Run(mounted_dmg_path);

  // After running the executable, before unmount, copy the contents of the DMG
  // into the cache folder. This will allow for differentials.
  CopyDMGContents(mounted_dmg_path, dmg_file_path.DirName());

  if (!UnmountDMG(mounted_dmg_path))
    VLOG(1) << "Could not unmount the DMG: " << mounted_dmg_path;

  // Delete the DMG from the cached folder after we are done.
  if (!base::DeleteFile(dmg_file_path)) {
    VPLOG(1) << "Couldn't remove the DMG.";
  }

  return result;
}

// Unzips the zip using the existing unzip utility in Mac. Path to the zip is
// specified by the `zip_file_path`. The install executable located at
// "/.install" in the contents of the zip is executed, and then the zip is
// deleted. Returns an error code if unzipping the archive or executing the
// executable failed.
int InstallFromZip(const base::FilePath& zip_file_path,
                   base::OnceCallback<int(const base::FilePath&)> install) {
  const base::FilePath dest_path = zip_file_path.DirName();

  if (!UnzipWithExe(zip_file_path, dest_path)) {
    VLOG(1) << "Failed to unzip zip file.";
    return static_cast<int>(InstallErrors::kFailedToExpandZip);
  }

  if (!ConfirmFilePermissions(dest_path, kPermissionsMask)) {
    return static_cast<int>(InstallErrors::kCouldNotConfirmAppPermissions);
  }

  const int result = std::move(install).Run(dest_path);

  // Remove the zip file, keep the expanded.
  base::DeleteFile(zip_file_path);

  return result;
}

// Installs with a path to the app specified by the `app_file_path`. The install
// executable located at "/.install" next to the .app is executed. This function
// is important for the differential installs, as applying the differential
// creates a .app file within the caching folder.
int InstallFromApp(const base::FilePath& app_file_path,
                   base::OnceCallback<int(const base::FilePath&)> install) {
  if (!base::PathExists(app_file_path) ||
      app_file_path.FinalExtension() != ".app") {
    VLOG(1) << "Path to the app does not exist!";
    return static_cast<int>(InstallErrors::kNotSupportedInstallerType);
  }

  // Need to make sure that the app at the path being installed has the correect
  // permissions.
  if (!ConfirmFilePermissions(app_file_path, kPermissionsMask)) {
    return static_cast<int>(InstallErrors::kCouldNotConfirmAppPermissions);
  }

  return std::move(install).Run(app_file_path.DirName());
}
}  // namespace

int InstallFromArchive(const base::FilePath& file_path,
                       const base::FilePath& existence_checker_path,
                       const std::string& ap,
                       const std::string& arguments) {
  const std::map<std::string,
                 int (*)(const base::FilePath&,
                         base::OnceCallback<int(const base::FilePath&)>)>
      handlers = {
          {".dmg", &InstallFromDMG},
          {".zip", &InstallFromZip},
          {".app", &InstallFromApp},
      };
  for (const auto& entry : handlers) {
    base::FilePath new_path = AlterFileExtension(file_path, entry.first);
    if (base::PathExists(new_path)) {
      return entry.second(
          new_path, base::BindOnce(&RunExecutable, existence_checker_path, ap,
                                   arguments));
    }
  }

  VLOG(0) << "Could not find a supported installer to install.";
  return static_cast<int>(InstallErrors::kNotSupportedInstallerType);
}
}  // namespace updater
