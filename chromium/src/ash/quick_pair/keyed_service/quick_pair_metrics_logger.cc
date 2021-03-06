// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/quick_pair/keyed_service/quick_pair_metrics_logger.h"

#include "ash/quick_pair/common/device.h"
#include "ash/quick_pair/common/fast_pair/fast_pair_feature_usage_metrics_logger.h"
#include "ash/quick_pair/common/fast_pair/fast_pair_metrics.h"

namespace ash {
namespace quick_pair {

QuickPairMetricsLogger::QuickPairMetricsLogger(ScannerBroker* scanner_broker,
                                               PairerBroker* pairer_broker,
                                               UIBroker* ui_broker)
    : feature_usage_metrics_logger_(
          std::make_unique<FastPairFeatureUsageMetricsLogger>()) {
  scanner_broker_observation_.Observe(scanner_broker);
  pairer_broker_observation_.Observe(pairer_broker);
  ui_broker_observation_.Observe(ui_broker);
}

QuickPairMetricsLogger::~QuickPairMetricsLogger() = default;

void QuickPairMetricsLogger::OnDevicePaired(scoped_refptr<Device> device) {
  switch (device->protocol) {
    case Protocol::kFastPairInitial:
      RecordFastPairEngagementFlow(
          FastPairEngagementFlowEvent::kPairingSucceeded);
      feature_usage_metrics_logger_->RecordUsage(/*success=*/true);
      break;
    case Protocol::kFastPairRetroactive:
      break;
    case Protocol::kFastPairSubsequent:
      break;
  }
}

void QuickPairMetricsLogger::OnPairFailure(scoped_refptr<Device> device,
                                           PairFailure failure) {
  switch (device->protocol) {
    case Protocol::kFastPairInitial:
      RecordFastPairEngagementFlow(FastPairEngagementFlowEvent::kPairingFailed);
      feature_usage_metrics_logger_->RecordUsage(/*success=*/false);
      break;
    case Protocol::kFastPairRetroactive:
      break;
    case Protocol::kFastPairSubsequent:
      break;
  }
}

void QuickPairMetricsLogger::OnDiscoveryAction(scoped_refptr<Device> device,
                                               DiscoveryAction action) {
  switch (device->protocol) {
    case Protocol::kFastPairInitial:
      switch (action) {
        case DiscoveryAction::kPairToDevice:
          RecordFastPairEngagementFlow(
              FastPairEngagementFlowEvent::kDiscoveryUiConnectPressed);
          break;
        case DiscoveryAction::kDismissedByUser:
        case DiscoveryAction::kDismissed:
          RecordFastPairEngagementFlow(
              FastPairEngagementFlowEvent::kDiscoveryUiDismissed);
          feature_usage_metrics_logger_->RecordUsage(/*success=*/true);
          break;
      }
      break;
    case Protocol::kFastPairRetroactive:
      break;
    case Protocol::kFastPairSubsequent:
      break;
  }
}

void QuickPairMetricsLogger::OnPairingFailureAction(
    scoped_refptr<Device> device,
    PairingFailedAction action) {
  switch (device->protocol) {
    case Protocol::kFastPairInitial:
      switch (action) {
        case PairingFailedAction::kNavigateToSettings:
          RecordFastPairEngagementFlow(
              FastPairEngagementFlowEvent::kErrorUiSettingsPressed);
          break;
        case PairingFailedAction::kDismissedByUser:
        case PairingFailedAction::kDismissed:
          RecordFastPairEngagementFlow(
              FastPairEngagementFlowEvent::kErrorUiDismissed);
          break;
      }
      break;
    case Protocol::kFastPairRetroactive:
      break;
    case Protocol::kFastPairSubsequent:
      break;
  }
}

void QuickPairMetricsLogger::OnDeviceFound(scoped_refptr<Device> device) {
  switch (device->protocol) {
    case Protocol::kFastPairInitial:
      RecordFastPairEngagementFlow(
          FastPairEngagementFlowEvent::kDiscoveryUiShown);
      break;
    case Protocol::kFastPairRetroactive:
      break;
    case Protocol::kFastPairSubsequent:
      break;
  }
}

void QuickPairMetricsLogger::OnAccountKeyWrite(
    scoped_refptr<Device> device,
    absl::optional<AccountKeyFailure> error) {}

void QuickPairMetricsLogger::OnCompanionAppAction(scoped_refptr<Device> device,
                                                  CompanionAppAction action) {}

void QuickPairMetricsLogger::OnAssociateAccountAction(
    scoped_refptr<Device> device,
    AssociateAccountAction action) {}

void QuickPairMetricsLogger::OnDeviceLost(scoped_refptr<Device> device) {}

}  // namespace quick_pair
}  // namespace ash
