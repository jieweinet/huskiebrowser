// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/enterprise/connectors/device_trust/signals/decorators/common/common_signals_decorator.h"

#include "base/bind.h"
#include "base/callback.h"
#include "chrome/browser/enterprise/signals/signals_utils.h"
#include "components/policy/core/common/cloud/cloud_policy_util.h"
#include "components/version_info/version_info.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

namespace enterprise_connectors {

CommonSignalsDecorator::CommonSignalsDecorator(PrefService* local_state,
                                               PrefService* profile_prefs)
    : local_state_(local_state), profile_prefs_(profile_prefs) {
  DCHECK(profile_prefs_);
  DCHECK(local_state_);
}

CommonSignalsDecorator::~CommonSignalsDecorator() = default;

void CommonSignalsDecorator::Decorate(SignalsType& signals,
                                      base::OnceClosure done_closure) {
  signals.set_os(policy::GetOSPlatform());
  signals.set_os_version(policy::GetOSVersion());
  signals.set_display_name(policy::GetDeviceName());
  signals.set_browser_version(version_info::GetVersionNumber());

  // Get signals from policy values.
  signals.set_built_in_dns_client_enabled(
      enterprise_signals::utils::GetBuiltInDnsClientEnabled(local_state_));
  signals.set_safe_browsing_protection_level(static_cast<int32_t>(
      enterprise_signals::utils::GetSafeBrowsingProtectionLevel(
          profile_prefs_)));

  absl::optional<bool> third_party_blocking_enabled =
      enterprise_signals::utils::GetThirdPartyBlockingEnabled(local_state_);
  if (third_party_blocking_enabled.has_value()) {
    signals.set_third_party_blocking_enabled(
        third_party_blocking_enabled.value());
  }

  absl::optional<bool> chrome_cleanup_enabled =
      enterprise_signals::utils::GetChromeCleanupEnabled(local_state_);
  if (chrome_cleanup_enabled.has_value()) {
    signals.set_chrome_cleanup_enabled(chrome_cleanup_enabled.value());
  }

  absl::optional<safe_browsing::PasswordProtectionTrigger>
      password_protection_warning_trigger =
          enterprise_signals::utils::GetPasswordProtectionWarningTrigger(
              profile_prefs_);
  if (password_protection_warning_trigger.has_value()) {
    signals.set_password_protection_warning_trigger(
        static_cast<int32_t>(password_protection_warning_trigger.value()));
  }

  auto callback =
      base::BindOnce(&CommonSignalsDecorator::OnHardwareInfoRetrieved,
                     weak_ptr_factory_.GetWeakPtr(), std::ref(signals),
                     std::move(done_closure));

  base::SysInfo::GetHardwareInfo(std::move(callback));
}

void CommonSignalsDecorator::OnHardwareInfoRetrieved(
    SignalsType& signals,
    base::OnceClosure done_closure,
    base::SysInfo::HardwareInfo hardware_info) {
  // TODO(b/178421844): Look into adding caching support for these signals, as
  // they will never change throughout the browser's lifetime.
  signals.set_device_model(hardware_info.model);
  signals.set_device_manufacturer(hardware_info.manufacturer);

  std::move(done_closure).Run();
}

}  // namespace enterprise_connectors
