// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/policy/messaging_layer/public/report_client.h"

#include <memory>
#include <utility>

#include "base/bind.h"
#include "base/callback.h"
#include "base/feature_list.h"
#include "base/memory/ptr_util.h"
#include "base/memory/scoped_refptr.h"
#include "base/memory/singleton.h"
#include "base/path_service.h"
#include "base/strings/strcat.h"
#include "base/task/bind_post_task.h"
#include "base/task/thread_pool.h"
#include "base/threading/sequence_bound.h"
#include "build/build_config.h"
#include "build/chromeos_buildflags.h"
#include "chrome/browser/policy/messaging_layer/upload/upload_provider.h"
#include "chrome/browser/policy/messaging_layer/util/get_cloud_policy_client.h"
#include "chrome/common/chrome_paths.h"
#include "components/policy/core/common/cloud/cloud_policy_client_registration_helper.h"
#include "components/policy/core/common/cloud/cloud_policy_manager.h"
#include "components/policy/core/common/cloud/device_management_service.h"
#include "components/policy/core/common/cloud/machine_level_user_cloud_policy_manager.h"
#include "components/policy/core/common/cloud/user_cloud_policy_manager.h"
#include "components/reporting/client/report_queue_configuration.h"
#include "components/reporting/client/report_queue_impl.h"
#include "components/reporting/encryption/encryption_module.h"
#include "components/reporting/encryption/verification.h"
#include "components/reporting/proto/record.pb.h"
#include "components/reporting/storage/storage_configuration.h"
#include "components/reporting/storage/storage_module.h"
#include "components/reporting/storage/storage_module_interface.h"
#include "components/reporting/storage/storage_uploader_interface.h"
#include "components/reporting/storage_selector/storage_selector.h"
#include "components/reporting/util/status.h"
#include "components/reporting/util/status_macros.h"
#include "components/reporting/util/statusor.h"
#include "components/signin/public/identity_manager/identity_manager.h"

namespace reporting {

namespace {

const base::FilePath::CharType kReportingDirectory[] =
    FILE_PATH_LITERAL("reporting");

void CreateLocalStorageModule(
    const base::FilePath& local_reporting_path,
    base::StringPiece verification_key,
    CompressionInformation::CompressionAlgorithm compression_algorithm,
    UploaderInterface::AsyncStartUploaderCb async_start_upload_cb,
    base::OnceCallback<void(StatusOr<scoped_refptr<StorageModuleInterface>>)>
        cb) {
  LOG(WARNING) << "Store reporting data locally";
  StorageModule::Create(
      StorageOptions()
          .set_directory(local_reporting_path)
          .set_signature_verification_public_key(verification_key),
      std::move(async_start_upload_cb), EncryptionModule::Create(),
      CompressionModule::Create(512, compression_algorithm), std::move(cb));
}
}  // namespace

// Uploader is passed to Storage in order to upload messages using the
// UploadClient.
class ReportingClient::Uploader : public UploaderInterface {
 public:
  using UploadCallback =
      base::OnceCallback<Status(bool,
                                std::unique_ptr<std::vector<EncryptedRecord>>)>;

  static std::unique_ptr<Uploader> Create(bool need_encryption_key,
                                          UploadCallback upload_callback) {
    return base::WrapUnique(
        new Uploader(need_encryption_key, std::move(upload_callback)));
  }

  ~Uploader() override = default;
  Uploader(const Uploader& other) = delete;
  Uploader& operator=(const Uploader& other) = delete;

  void ProcessRecord(EncryptedRecord data,
                     base::OnceCallback<void(bool)> processed_cb) override {
    helper_.AsyncCall(&Helper::ProcessRecord)
        .WithArgs(std::move(data), std::move(processed_cb));
  }
  void ProcessGap(SequencingInformation start,
                  uint64_t count,
                  base::OnceCallback<void(bool)> processed_cb) override {
    helper_.AsyncCall(&Helper::ProcessGap)
        .WithArgs(std::move(start), count, std::move(processed_cb));
  }

  void Completed(Status final_status) override {
    helper_.AsyncCall(&Helper::Completed).WithArgs(final_status);
  }

 private:
  // Helper class that performs actions, wrapped in SequenceBound by |Uploader|.
  class Helper {
   public:
    Helper(bool need_encryption_key, UploadCallback upload_callback);
    Helper(const Helper& other) = delete;
    Helper& operator=(const Helper& other) = delete;
    void ProcessRecord(EncryptedRecord data,
                       base::OnceCallback<void(bool)> processed_cb);
    void ProcessGap(SequencingInformation start,
                    uint64_t count,
                    base::OnceCallback<void(bool)> processed_cb);
    void Completed(Status final_status);

   private:
    bool completed_{false};
    const bool need_encryption_key_;
    std::unique_ptr<std::vector<EncryptedRecord>> encrypted_records_;

    UploadCallback upload_callback_;
  };

  Uploader(bool need_encryption_key, UploadCallback upload_callback)
      : helper_(base::ThreadPool::CreateSequencedTaskRunner({}),
                need_encryption_key,
                std::move(upload_callback)) {}

  base::SequenceBound<Helper> helper_;
};

ReportingClient::Uploader::Helper::Helper(
    bool need_encryption_key,
    ReportingClient::Uploader::UploadCallback upload_callback)
    : need_encryption_key_(need_encryption_key),
      encrypted_records_(std::make_unique<std::vector<EncryptedRecord>>()),
      upload_callback_(std::move(upload_callback)) {}

void ReportingClient::Uploader::Helper::ProcessRecord(
    EncryptedRecord data,
    base::OnceCallback<void(bool)> processed_cb) {
  if (completed_) {
    std::move(processed_cb).Run(false);
    return;
  }
  encrypted_records_->emplace_back(std::move(data));
  std::move(processed_cb).Run(true);
}

void ReportingClient::Uploader::Helper::ProcessGap(
    SequencingInformation start,
    uint64_t count,
    base::OnceCallback<void(bool)> processed_cb) {
  if (completed_) {
    std::move(processed_cb).Run(false);
    return;
  }
  for (uint64_t i = 0; i < count; ++i) {
    encrypted_records_->emplace_back();
    *encrypted_records_->rbegin()->mutable_sequencing_information() = start;
    start.set_sequencing_id(start.sequencing_id() + 1);
  }
  std::move(processed_cb).Run(true);
}

void ReportingClient::Uploader::Helper::Completed(Status final_status) {
  if (!final_status.ok()) {
    // No work to do - something went wrong with storage and it no longer
    // wants to upload the records. Let the records die with |this|.
    return;
  }
  if (completed_) {
    // Upload has already been invoked. Return.
    return;
  }
  completed_ = true;
  DCHECK(encrypted_records_);
  if (encrypted_records_->empty() && !need_encryption_key_) {
    return;
  }
  DCHECK(upload_callback_);
  Status upload_status =
      std::move(upload_callback_)
          .Run(need_encryption_key_, std::move(encrypted_records_));
  if (!upload_status.ok()) {
    LOG(ERROR) << "Unable to upload records: " << upload_status;
  }
}

ReportingClient::ReportingClient()
    : ReportQueueProvider(base::BindRepeating(
          [](base::OnceCallback<void(
                 StatusOr<scoped_refptr<StorageModuleInterface>>)>
                 storage_created_cb) {
#if BUILDFLAG(IS_CHROMEOS_ASH) || BUILDFLAG(IS_CHROMEOS_LACROS)
            if (StorageSelector::is_use_missive()) {
              StorageSelector::CreateMissiveStorageModule(
                  std::move(storage_created_cb));
              return;
            }
#endif  // BUILDFLAG(IS_CHROMEOS_ASH) || BUILDFLAG(IS_CHROMEOS_LACROS)

            // Storage location in the local file system (if local storage is
            // enabled).
            base::FilePath reporting_path;
            const auto res =
                base::PathService::Get(chrome::DIR_USER_DATA, &reporting_path);
            DCHECK(res) << "Could not retrieve base path";
#if BUILDFLAG(IS_CHROMEOS_ASH)
            reporting_path = reporting_path.Append("user");
#endif
            reporting_path = reporting_path.Append(kReportingDirectory);
            CreateLocalStorageModule(
                reporting_path, SignatureVerifier::VerificationKey(),
                CompressionInformation::COMPRESSION_SNAPPY,
                base::BindRepeating(&ReportingClient::AsyncStartUploader),
                std::move(storage_created_cb));
          })),
      build_cloud_policy_client_cb_(GetCloudPolicyClientCb()) {
}

ReportingClient::~ReportingClient() = default;

// static
ReportingClient* ReportingClient::GetInstance() {
  return base::Singleton<ReportingClient>::get();
}

// static
ReportQueueProvider* ReportQueueProvider::GetInstance() {
  // Forward to ReportingClient::GetInstance, because
  // base::Singleton<ReportingClient>::get() cannot be called
  // outside ReportingClient class.
  return ReportingClient::GetInstance();
}

// static
void ReportingClient::AsyncStartUploader(
    UploaderInterface::UploadReason reason,
    UploaderInterface::UploaderInterfaceResultCb start_uploader_cb) {
  ReportingClient::GetInstance()->DeliverAsyncStartUploader(
      reason, std::move(start_uploader_cb));
}

void ReportingClient::DeliverAsyncStartUploader(
    UploaderInterface::UploadReason reason,
    UploaderInterface::UploaderInterfaceResultCb start_uploader_cb) {
  sequenced_task_runner()->PostTask(
      FROM_HERE,
      base::BindOnce(
          [](UploaderInterface::UploadReason reason,
             UploaderInterface::UploaderInterfaceResultCb start_uploader_cb,
             ReportingClient* instance) {
            if (!instance->upload_provider_) {
              // If non-missived uploading is enabled, it will need upload
              // provider, In case of missived Uploader will be provided by
              // EncryptedReportingServiceProvider so it does not need to be
              // enabled here.
              if (StorageSelector::is_uploader_required() &&
                  !StorageSelector::is_use_missive()) {
                DCHECK(!instance->upload_provider_)
                    << "Upload provider already recorded";
                instance->upload_provider_ = instance->GetDefaultUploadProvider(
                    instance->build_cloud_policy_client_cb_);
              } else {
                std::move(start_uploader_cb)
                    .Run(Status(error::UNAVAILABLE, "Uploader not available"));
                return;
              }
            }
            auto uploader = Uploader::Create(
                reason,
                base::BindOnce(
                    [](UploadClient::ReportSuccessfulUploadCallback
                           report_success_upload_cb,
                       UploadClient::EncryptionKeyAttachedCallback
                           encryption_key_attached_cb,
                       EncryptedReportingUploadProvider* upload_provider,
                       bool need_encryption_key,
                       std::unique_ptr<std::vector<EncryptedRecord>> records) {
                      upload_provider->RequestUploadEncryptedRecords(
                          need_encryption_key, std::move(records),
                          std::move(report_success_upload_cb),
                          std::move(encryption_key_attached_cb),
                          base::DoNothing());
                      return Status::StatusOK();
                    },
                    base::BindOnce(&StorageModuleInterface::ReportSuccess,
                                   instance->storage()),
                    base::BindOnce(&StorageModuleInterface::UpdateEncryptionKey,
                                   instance->storage()),
                    base::Unretained(instance->upload_provider_.get())));
            std::move(start_uploader_cb).Run(std::move(uploader));
          },
          reason, std::move(start_uploader_cb), base::Unretained(this)));
}

std::unique_ptr<EncryptedReportingUploadProvider>
ReportingClient::GetDefaultUploadProvider(
    GetCloudPolicyClientCallback build_cloud_policy_client_cb) {
  return std::make_unique<::reporting::EncryptedReportingUploadProvider>(
      build_cloud_policy_client_cb);
}

ReportingClient::TestEnvironment::TestEnvironment(
    const base::FilePath& reporting_path,
    base::StringPiece verification_key,
    policy::CloudPolicyClient* client)
    : saved_storage_create_cb_(
          std::move(ReportingClient::GetInstance()->storage_create_cb_)),
      saved_build_cloud_policy_client_cb_(std::move(
          ReportingClient::GetInstance()->build_cloud_policy_client_cb_)) {
  ReportingClient::GetInstance()->storage_create_cb_ = base::BindRepeating(
      [](const base::FilePath& reporting_path,
         base::StringPiece verification_key,
         base::OnceCallback<void(
             StatusOr<scoped_refptr<StorageModuleInterface>>)>
             storage_created_cb) {
        CreateLocalStorageModule(
            reporting_path, verification_key,
            CompressionInformation::COMPRESSION_SNAPPY,
            base::BindRepeating(&ReportingClient::AsyncStartUploader),
            std::move(storage_created_cb));
      },
      reporting_path, verification_key);
  ReportingClient::GetInstance()->build_cloud_policy_client_cb_ =
      base::BindRepeating(
          [](policy::CloudPolicyClient* client,
             CloudPolicyClientResultCb build_cb) {
            std::move(build_cb).Run(std::move(client));
          },
          std::move(client));
}

ReportingClient::TestEnvironment::~TestEnvironment() {
  ReportingClient::GetInstance()->storage_create_cb_ =
      std::move(saved_storage_create_cb_);
  ReportingClient::GetInstance()->build_cloud_policy_client_cb_ =
      std::move(saved_build_cloud_policy_client_cb_);
  base::Singleton<ReportingClient>::OnExit(nullptr);
}

}  // namespace reporting
