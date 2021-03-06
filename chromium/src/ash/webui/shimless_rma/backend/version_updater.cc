// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/webui/shimless_rma/backend/version_updater.h"

#include "ash/webui/shimless_rma/mojom/shimless_rma.mojom.h"
#include "base/logging.h"
#include "chromeos/dbus/dbus_thread_manager.h"
#include "chromeos/dbus/update_engine/update_engine.pb.h"
#include "chromeos/dbus/update_engine/update_engine_client.h"
#include "chromeos/network/network_state.h"
#include "chromeos/network/network_state_handler.h"
#include "chromeos/network/network_type_pattern.h"

namespace ash {
namespace shimless_rma {

namespace {

void ReportUpdateFailure(const VersionUpdater::StatusCallback& callback,
                         update_engine::Operation operation) {
  callback.Run(operation, /*progress=*/0,
               /*rollback=*/false, /*powerwash=*/false,
               /*version=*/std::string(), /*update_size=*/0);
}

// Returns whether an update is allowed. If not, it calls the callback with
// the appropriate status. |interactive| indicates whether the user is actively
// checking for updates.
bool EnsureCanUpdate(const VersionUpdater::StatusCallback& callback) {
  if (!callback) {
    return false;
  }

  chromeos::NetworkStateHandler* network_state_handler =
      chromeos::NetworkHandler::Get()->network_state_handler();
  const chromeos::NetworkState* network =
      network_state_handler->DefaultNetwork();
  // TODO(gavindodd): Confirm that metered networks should be excluded.
  const bool metered = network_state_handler->default_network_is_metered();
  // Don't allow an update if device is currently offline or connected
  // to a network for which data is metered.
  if (!network || !network->IsConnectedState()) {
    // TODO(gavindodd): Pass a specific error to UI.
    ReportUpdateFailure(callback, update_engine::REPORTING_ERROR_EVENT);
    return false;
  } else if (metered) {
    LOG(ERROR) << "A metered network connection cannot be used during RMA.";
    callback.Run(update_engine::REPORTING_ERROR_EVENT, 0, false, false,
                 std::string(), 0);
    return false;
  }

  return true;
}
}  // namespace

VersionUpdater::VersionUpdater() {
  DBusThreadManager::Get()->GetUpdateEngineClient()->AddObserver(this);
}

VersionUpdater::~VersionUpdater() {
  DBusThreadManager::Get()->GetUpdateEngineClient()->RemoveObserver(this);
}

void VersionUpdater::SetStatusCallback(StatusCallback status_callback) {
  status_callback_ = std::move(status_callback);
}

bool VersionUpdater::UpdateOs() {
  if (!EnsureCanUpdate(status_callback_)) {
    return false;
  }

  UpdateEngineClient* update_engine_client =
      DBusThreadManager::Get()->GetUpdateEngineClient();
  if (!IsIdle()) {
    LOG(ERROR) << "Tried to start update when UpdateEngine not IDLE.";
    return false;
  }
  // RequestUpdateCheck will check if an update is available and install it.
  update_engine_client->RequestUpdateCheck(base::BindOnce(
      &VersionUpdater::OnUpdateProgress, weak_ptr_factory_.GetWeakPtr()));
  return true;
}

bool VersionUpdater::IsIdle() {
  return DBusThreadManager::Get()
             ->GetUpdateEngineClient()
             ->GetLastStatus()
             .current_operation() == update_engine::Operation::IDLE;
}

void VersionUpdater::UpdateStatusChanged(
    const update_engine::StatusResult& status) {
  if (status.current_operation() == update_engine::UPDATED_NEED_REBOOT) {
    // During RMA there are no other critical processes running so we can
    // automatically reboot.
    DBusThreadManager::Get()->GetUpdateEngineClient()->RebootAfterUpdate();
  }
  // TODO(gavindodd): Work out how errors are passed from UpdateEngine when
  // operation == REPORTING_ERROR_EVENT and handle them appropriately.
  status_callback_.Run(status.current_operation(), status.progress(), false,
                       status.will_powerwash_after_reboot(),
                       status.new_version(), status.new_size());
}

void VersionUpdater::OnUpdateProgress(
    UpdateEngineClient::UpdateCheckResult result) {
  switch (result) {
    case chromeos::UpdateEngineClient::UPDATE_RESULT_SUCCESS:
      // Nothing to do if the update check started successfully.
      break;
    case chromeos::UpdateEngineClient::UPDATE_RESULT_FAILED:
      // TODO(gavindodd): Pass a specific error to UI.
      ReportUpdateFailure(status_callback_,
                          update_engine::REPORTING_ERROR_EVENT);
      break;
    case chromeos::UpdateEngineClient::UPDATE_RESULT_NOTIMPLEMENTED:
      // No point retrying if the operation is not implemented.
      LOG(ERROR) << "Update check failed: Operation not implemented.";
      ReportUpdateFailure(status_callback_, update_engine::DISABLED);
      break;
  }
}

}  // namespace shimless_rma
}  // namespace ash
