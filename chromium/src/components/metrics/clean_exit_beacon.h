// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_METRICS_CLEAN_EXIT_BEACON_H_
#define COMPONENTS_METRICS_CLEAN_EXIT_BEACON_H_

#include <string>

#include "base/files/file_path.h"
#include "base/macros.h"
#include "base/time/time.h"
#include "build/build_config.h"
#include "components/version_info/channel.h"

class PrefRegistrySimple;
class PrefService;

namespace metrics {

// Reads and updates a beacon used to detect whether the previous browser
// process exited cleanly.
class CleanExitBeacon {
 public:
  // Instantiates a CleanExitBeacon whose value is stored in |local_state|'s
  // kStabilityExitedCleanly pref. |local_state| must be fully initialized.
  //
  // On Windows, |backup_registry_key| stores a backup of the beacon to verify
  // that the pref's value corresponds to the registry's. |backup_registry_key|
  // is ignored on other platforms, but iOS has a similar verification
  // mechanism embedded inside CleanExitBeacon.
  //
  // |user_data_dir| is the path to the client's user data directory. If empty,
  // a separate file will not be used for Variations Safe Mode prefs.
  //
  // TODO(crbug/1241702): Remove |channel| at the end of the Extended Variations
  // Safe Mode experiment. |channel| is used to enable the experiment on only
  // certain channels.
  CleanExitBeacon(const std::wstring& backup_registry_key,
                  const base::FilePath& user_data_dir,
                  PrefService* local_state,
                  version_info::Channel channel);

  virtual ~CleanExitBeacon() = default;

  // Not copyable or movable.
  CleanExitBeacon(const CleanExitBeacon&) = delete;
  CleanExitBeacon& operator=(const CleanExitBeacon&) = delete;

  // Initializes the CleanExitBeacon. This includes the following tasks:
  // 1. Determining if the last session exited cleanly,
  // 2. Incrementing the crash streak, if necessary, and
  // 3. Emitting some metrics.
  void Initialize();

  // Returns the original value of the beacon.
  bool exited_cleanly() const { return did_previous_session_exit_cleanly_; }

  // Returns the original value of the last live timestamp.
  base::Time browser_last_live_timestamp() const {
    return initial_browser_last_live_timestamp_;
  }

  // Sets the beacon value to |exited_cleanly| and updates the last live
  // timestamp. If |write_synchronously| is true, then the beacon value is
  // written to disk synchronously. If false, a write is scheduled, and for
  // clients in the Extended Variations Safe Mode experiment, a synchronous
  // write is done, too.
  //
  // Note: |write_synchronously| should be true only for some clients in the
  // Extended Variations Safe Mode experiment.
  void WriteBeaconValue(bool exited_cleanly, bool write_synchronously = false);

  // Updates the last live timestamp.
  void UpdateLastLiveTimestamp();

  // Registers local state prefs used by this class.
  static void RegisterPrefs(PrefRegistrySimple* registry);

  // Updates pref and NSUserDefault value for stability beacon, as either one
  // can effect the value of exited_cleanly depending on the value of
  // ShouldUseUserDefaultsBeacon().
  static void SetStabilityExitedCleanlyForTesting(PrefService* local_state,
                                                  bool exited_cleanly);

  // Resets pref and NSUserDefault value for stability beacon.
  static void ResetStabilityExitedCleanlyForTesting(PrefService* local_state);

  // CHECKs that Chrome exited cleanly.
  static void EnsureCleanShutdown(PrefService* local_state);

#if defined(OS_IOS)
  // Checks user default value of kUseUserDefaultsForExitedCleanlyBeacon.
  // Because variations are not initialized early in startup, pair a user
  // defaults value with the variations config.
  static bool ShouldUseUserDefaultsBeacon();

  // Syncs feature kUseUserDefaultsForExitedCleanlyBeacon to NSUserDefaults
  // kUserDefaultsFeatureFlagForExitedCleanlyBeacon.
  static void SyncUseUserDefaultsBeacon();
#endif  // defined(OS_IOS)

  // Prevents a test browser from performing two clean shutdown steps. First, it
  // prevents the beacon value from being updated after this function is called.
  // This prevents the the test browser from signaling that Chrome is shutting
  // down cleanly. Second, it makes EnsureCleanShutdown() a no-op.
  static void SkipCleanShutdownStepsForTesting();

 private:
  // Writes |exited_cleanly| and the crash streak to the file located at
  // |beacon_file_path_|.
  void WriteBeaconFile(bool exited_cleanly) const;

#if defined(OS_IOS)
  // Checks if the NSUserDefault clean exit beacon value is set.
  static bool HasUserDefaultsBeacon();

  // Gets the NSUserDefault clean exit beacon value.
  static bool GetUserDefaultsBeacon();

  // Sets the user default clean exit beacon value.
  static void SetUserDefaultsBeacon(bool clean);

  // Clears the user default clean exit beacon value, used for testing.
  static void ResetUserDefaultsBeacon();
#endif  // defined(OS_IOS)

  // Indicates whether the CleanExitBeacon has been initialized.
  bool initialized_ = false;

  // Stores a backup of the beacon. Windows only.
  const std::wstring backup_registry_key_;

  // Path to the client's user data directory. May be empty.
  const base::FilePath user_data_dir_;

  PrefService* const local_state_;

  // This is the value of the last live timestamp from local state at the time
  // of construction. It is a timestamp from the previous browser session when
  // the browser was known to be alive.
  const base::Time initial_browser_last_live_timestamp_;

  // The client's channel, e.g. Canary. Used to help determine whether the
  // client should participate in the Extended Variations Safe Mode experiment.
  // TODO(crbug/1241702): Remove at the end of the experiment.
  const version_info::Channel channel_;

  bool did_previous_session_exit_cleanly_ = false;

  // Where the clean exit beacon and the variations crash streak may be stored
  // for some clients in the Extended Variations Safe Mode experiment.
  base::FilePath beacon_file_path_;
};

}  // namespace metrics

#endif  // COMPONENTS_METRICS_CLEAN_EXIT_BEACON_H_
