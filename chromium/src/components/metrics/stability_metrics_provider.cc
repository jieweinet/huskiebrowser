// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/metrics/stability_metrics_provider.h"

#include <string>

#include "base/logging.h"
#include "base/metrics/histogram_macros.h"
#include "build/build_config.h"
#include "components/metrics/metrics_pref_names.h"
#include "components/metrics/stability_metrics_helper.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/scoped_user_pref_update.h"
#include "third_party/metrics_proto/system_profile.pb.h"

#if defined(OS_ANDROID)
#include "base/android/build_info.h"
#endif
#if defined(OS_WIN)
#include "components/metrics/system_session_analyzer/system_session_analyzer_win.h"
#endif

namespace metrics {

namespace {

#if defined(OS_ANDROID)
bool HasGmsCoreVersionChanged(PrefService* local_state) {
  std::string previous_version =
      local_state->GetString(prefs::kStabilityGmsCoreVersion);
  std::string current_version =
      base::android::BuildInfo::GetInstance()->gms_version_code();

  // If the last version is empty, treat it as consistent.
  if (previous_version.empty())
    return false;

  return previous_version != current_version;
}

void UpdateGmsCoreVersionPref(PrefService* local_state) {
  std::string current_version =
      base::android::BuildInfo::GetInstance()->gms_version_code();
  local_state->SetString(prefs::kStabilityGmsCoreVersion, current_version);
}
#endif

}  // namespace

StabilityMetricsProvider::StabilityMetricsProvider(PrefService* local_state)
    : local_state_(local_state) {}

StabilityMetricsProvider::~StabilityMetricsProvider() = default;

// static
void StabilityMetricsProvider::RegisterPrefs(PrefRegistrySimple* registry) {
  registry->RegisterIntegerPref(prefs::kStabilityCrashCount, 0);
  registry->RegisterIntegerPref(prefs::kStabilityIncompleteSessionEndCount, 0);
  registry->RegisterBooleanPref(prefs::kStabilitySessionEndCompleted, true);
  registry->RegisterIntegerPref(prefs::kStabilityLaunchCount, 0);
  registry->RegisterIntegerPref(prefs::kStabilityFileMetricsUnsentFilesCount,
                                0);
  registry->RegisterIntegerPref(prefs::kStabilityFileMetricsUnsentSamplesCount,
                                0);

#if defined(OS_ANDROID)
  registry->RegisterStringPref(prefs::kStabilityGmsCoreVersion, "");
  registry->RegisterIntegerPref(prefs::kStabilityCrashCountDueToGmsCoreUpdate,
                                0);
#endif
#if defined(OS_WIN)
  registry->RegisterIntegerPref(prefs::kStabilitySystemCrashCount, 0);
#endif
}

void StabilityMetricsProvider::Init() {
#if defined(OS_ANDROID)
  // This method has to be called after HasGmsCoreVersionChanged() to avoid
  // overwriting thie result.
  UpdateGmsCoreVersionPref(local_state_);
#endif
}

void StabilityMetricsProvider::ClearSavedStabilityMetrics() {
  local_state_->SetInteger(prefs::kStabilityCrashCount, 0);
  local_state_->SetInteger(prefs::kStabilityIncompleteSessionEndCount, 0);
  local_state_->SetInteger(prefs::kStabilityLaunchCount, 0);
  local_state_->SetBoolean(prefs::kStabilitySessionEndCompleted, true);

  // The 0 is a valid value for the below prefs, clears pref instead
  // of setting to default value.
  local_state_->ClearPref(prefs::kStabilityFileMetricsUnsentFilesCount);
  local_state_->ClearPref(prefs::kStabilityFileMetricsUnsentSamplesCount);

#if defined(OS_WIN)
  local_state_->SetInteger(prefs::kStabilitySystemCrashCount, 0);
#endif
}

void StabilityMetricsProvider::ProvideStabilityMetrics(
    SystemProfileProto* system_profile) {
  SystemProfileProto::Stability* stability =
      system_profile->mutable_stability();

  int pref_value = 0;

  if (GetAndClearPrefValue(prefs::kStabilityLaunchCount, &pref_value))
    stability->set_launch_count(pref_value);

  if (GetAndClearPrefValue(prefs::kStabilityCrashCount, &pref_value))
    stability->set_crash_count(pref_value);

#if defined(OS_ANDROID)
  if (GetAndClearPrefValue(prefs::kStabilityCrashCountDueToGmsCoreUpdate,
                           &pref_value)) {
    stability->set_crash_count_due_to_gms_core_update(pref_value);
  }
#endif

  if (GetAndClearPrefValue(prefs::kStabilityIncompleteSessionEndCount,
                           &pref_value))
    stability->set_incomplete_shutdown_count(pref_value);


  if (local_state_->HasPrefPath(prefs::kStabilityFileMetricsUnsentFilesCount)) {
    UMA_STABILITY_HISTOGRAM_COUNTS_100(
        "Stability.Internals.FileMetricsProvider.BrowserMetrics."
        "UnsentFilesCount",
        local_state_->GetInteger(prefs::kStabilityFileMetricsUnsentFilesCount));
    local_state_->ClearPref(prefs::kStabilityFileMetricsUnsentFilesCount);
  }

  if (local_state_->HasPrefPath(
          prefs::kStabilityFileMetricsUnsentSamplesCount)) {
    UMA_STABILITY_HISTOGRAM_CUSTOM_COUNTS(
        "Stability.Internals.FileMetricsProvider.BrowserMetrics."
        "UnsentSamplesCount",
        local_state_->GetInteger(
            prefs::kStabilityFileMetricsUnsentSamplesCount),
        0, 1000000, 50);
    local_state_->ClearPref(prefs::kStabilityFileMetricsUnsentSamplesCount);
  }

#if defined(OS_WIN)
  if (GetAndClearPrefValue(prefs::kStabilitySystemCrashCount, &pref_value)) {
    UMA_STABILITY_HISTOGRAM_COUNTS_100("Stability.Internals.SystemCrashCount",
                                       pref_value);
  }
#endif
}


void StabilityMetricsProvider::CheckLastSessionEndCompleted() {
  if (!local_state_->GetBoolean(prefs::kStabilitySessionEndCompleted)) {
    IncrementPrefValue(prefs::kStabilityIncompleteSessionEndCount);
    StabilityMetricsHelper::RecordStabilityEvent(
        StabilityEventType::kIncompleteShutdown);
    // This is marked false when we get a WM_ENDSESSION.
    MarkSessionEndCompleted(true);
  }
}

void StabilityMetricsProvider::MarkSessionEndCompleted(bool end_completed) {
  local_state_->SetBoolean(prefs::kStabilitySessionEndCompleted, end_completed);
}

void StabilityMetricsProvider::LogCrash(base::Time last_live_timestamp) {
#if defined(OS_ANDROID)
  // On Android, if there is an update for GMS Core when Chrome is running,
  // Chrome will be killed, counting as a crash. This is expected and should not
  // be counted in stability crash counts. Thus these crashes are added to a
  // specific bucket for crashes caused by GMS Core updates.
  if (HasGmsCoreVersionChanged(local_state_)) {
    IncrementPrefValue(prefs::kStabilityCrashCountDueToGmsCoreUpdate);
    return;
  }
#endif
  IncrementPrefValue(prefs::kStabilityCrashCount);
  StabilityMetricsHelper::RecordStabilityEvent(
      StabilityEventType::kBrowserCrash);

#if defined(OS_WIN)
  MaybeLogSystemCrash(last_live_timestamp);
#endif
}

void StabilityMetricsProvider::LogLaunch() {
  IncrementPrefValue(prefs::kStabilityLaunchCount);
  StabilityMetricsHelper::RecordStabilityEvent(StabilityEventType::kLaunch);
}

#if defined(OS_WIN)
bool StabilityMetricsProvider::IsUncleanSystemSession(
    base::Time last_live_timestamp) {
  DCHECK_NE(base::Time(), last_live_timestamp);
  // There's a non-null last live timestamp, see if this occurred in
  // a Windows system session that ended uncleanly. The expectation is that
  // |last_live_timestamp| will have occurred in the immediately previous system
  // session, but if the system has been restarted many times since Chrome last
  // ran, that's not necessarily true. Log traversal can be expensive, so we
  // limit the analyzer to reaching back three previous system sessions to bound
  // the cost of the traversal.
  SystemSessionAnalyzer analyzer(3);

  SystemSessionAnalyzer::Status status =
      analyzer.IsSessionUnclean(last_live_timestamp);

  return status == SystemSessionAnalyzer::UNCLEAN;
}

void StabilityMetricsProvider::MaybeLogSystemCrash(
    base::Time last_live_timestamp) {
  if (last_live_timestamp != base::Time() &&
      IsUncleanSystemSession(last_live_timestamp)) {
    IncrementPrefValue(prefs::kStabilitySystemCrashCount);
  }
}
#endif

void StabilityMetricsProvider::IncrementPrefValue(const char* path) {
  int value = local_state_->GetInteger(path);
  local_state_->SetInteger(path, value + 1);
}

int StabilityMetricsProvider::GetAndClearPrefValue(const char* path,
                                                   int* value) {
  *value = local_state_->GetInteger(path);
  if (*value != 0)
    local_state_->SetInteger(path, 0);
  return *value;
}

}  // namespace metrics
