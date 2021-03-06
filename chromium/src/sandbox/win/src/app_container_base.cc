// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include <aclapi.h>
#include <sddl.h>
#include <userenv.h>

#include "base/strings/stringprintf.h"
#include "base/win/scoped_co_mem.h"
#include "base/win/scoped_handle.h"
#include "sandbox/win/src/acl.h"
#include "sandbox/win/src/app_container_base.h"
#include "sandbox/win/src/restricted_token_utils.h"
#include "sandbox/win/src/win_utils.h"

namespace sandbox {

namespace {

typedef decltype(::CreateAppContainerProfile) CreateAppContainerProfileFunc;

typedef decltype(::DeriveAppContainerSidFromAppContainerName)
    DeriveAppContainerSidFromAppContainerNameFunc;

typedef decltype(::DeleteAppContainerProfile) DeleteAppContainerProfileFunc;

typedef decltype(::GetAppContainerFolderPath) GetAppContainerFolderPathFunc;

typedef decltype(
    ::GetAppContainerRegistryLocation) GetAppContainerRegistryLocationFunc;

struct FreeSidDeleter {
  inline void operator()(void* ptr) const { ::FreeSid(ptr); }
};

bool IsValidObjectType(SE_OBJECT_TYPE object_type) {
  switch (object_type) {
    case SE_FILE_OBJECT:
    case SE_REGISTRY_KEY:
      return true;
    default:
      break;
  }
  return false;
}

bool GetGenericMappingForType(SE_OBJECT_TYPE object_type,
                              GENERIC_MAPPING* generic_mapping) {
  if (!IsValidObjectType(object_type))
    return false;
  if (object_type == SE_FILE_OBJECT) {
    generic_mapping->GenericRead = FILE_GENERIC_READ;
    generic_mapping->GenericWrite = FILE_GENERIC_WRITE;
    generic_mapping->GenericExecute = FILE_GENERIC_EXECUTE;
    generic_mapping->GenericAll = FILE_ALL_ACCESS;
  } else {
    generic_mapping->GenericRead = KEY_READ;
    generic_mapping->GenericWrite = KEY_WRITE;
    generic_mapping->GenericExecute = KEY_EXECUTE;
    generic_mapping->GenericAll = KEY_ALL_ACCESS;
  }
  return true;
}

class ScopedImpersonation {
 public:
  explicit ScopedImpersonation(const base::win::ScopedHandle& token) {
    BOOL result = ::ImpersonateLoggedOnUser(token.Get());
    DCHECK(result);
  }

  ~ScopedImpersonation() {
    BOOL result = ::RevertToSelf();
    DCHECK(result);
  }
};

}  // namespace

// static
AppContainerBase* AppContainerBase::CreateProfile(const wchar_t* package_name,
                                                  const wchar_t* display_name,
                                                  const wchar_t* description) {
  static auto create_app_container_profile =
      reinterpret_cast<CreateAppContainerProfileFunc*>(GetProcAddress(
          GetModuleHandle(L"userenv"), "CreateAppContainerProfile"));
  if (!create_app_container_profile)
    return nullptr;

  PSID package_sid = nullptr;
  HRESULT hr = create_app_container_profile(
      package_name, display_name, description, nullptr, 0, &package_sid);
  if (hr == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS))
    return Open(package_name);

  if (FAILED(hr))
    return nullptr;
  std::unique_ptr<void, FreeSidDeleter> sid_deleter(package_sid);
  return new AppContainerBase(Sid(package_sid), AppContainerType::kProfile);
}

// static
AppContainerBase* AppContainerBase::Open(const wchar_t* package_name) {
  static auto derive_app_container_sid =
      reinterpret_cast<DeriveAppContainerSidFromAppContainerNameFunc*>(
          GetProcAddress(GetModuleHandle(L"userenv"),
                         "DeriveAppContainerSidFromAppContainerName"));
  if (!derive_app_container_sid)
    return nullptr;

  PSID package_sid = nullptr;
  HRESULT hr = derive_app_container_sid(package_name, &package_sid);
  if (FAILED(hr))
    return nullptr;

  std::unique_ptr<void, FreeSidDeleter> sid_deleter(package_sid);
  return new AppContainerBase(Sid(package_sid), AppContainerType::kDerived);
}

// static
AppContainerBase* AppContainerBase::CreateLowbox(const wchar_t* sid) {
  PSID package_sid;
  if (!ConvertStringSidToSid(sid, &package_sid))
    return nullptr;

  std::unique_ptr<void, LocalFreeDeleter> sid_deleter(package_sid);
  return new AppContainerBase(Sid(package_sid), AppContainerType::kLowbox);
}

// static
bool AppContainerBase::Delete(const wchar_t* package_name) {
  static auto delete_app_container_profile =
      reinterpret_cast<DeleteAppContainerProfileFunc*>(GetProcAddress(
          GetModuleHandle(L"userenv"), "DeleteAppContainerProfile"));
  if (!delete_app_container_profile)
    return false;

  return SUCCEEDED(delete_app_container_profile(package_name));
}

AppContainerBase::AppContainerBase(const Sid& package_sid,
                                   AppContainerType type)
    : ref_count_(0),
      package_sid_(package_sid),
      enable_low_privilege_app_container_(false),
      type_(type) {}

AppContainerBase::~AppContainerBase() {}

void AppContainerBase::AddRef() {
  // ref_count starts at 0 for this class so can increase from 0 (once).
  CHECK(::InterlockedIncrement(&ref_count_) > 0);
}

void AppContainerBase::Release() {
  LONG result = ::InterlockedDecrement(&ref_count_);
  CHECK(result >= 0);
  if (result == 0) {
    delete this;
  }
}

bool AppContainerBase::GetRegistryLocation(REGSAM desired_access,
                                           base::win::ScopedHandle* key) {
  static GetAppContainerRegistryLocationFunc*
      get_app_container_registry_location =
          reinterpret_cast<GetAppContainerRegistryLocationFunc*>(GetProcAddress(
              GetModuleHandle(L"userenv"), "GetAppContainerRegistryLocation"));
  if (!get_app_container_registry_location)
    return false;

  base::win::ScopedHandle token;
  if (BuildLowBoxToken(&token) != SBOX_ALL_OK)
    return false;

  ScopedImpersonation impersonation(token);
  HKEY key_handle;
  if (FAILED(get_app_container_registry_location(desired_access, &key_handle)))
    return false;
  key->Set(key_handle);
  return true;
}

bool AppContainerBase::GetFolderPath(base::FilePath* file_path) {
  static GetAppContainerFolderPathFunc* get_app_container_folder_path =
      reinterpret_cast<GetAppContainerFolderPathFunc*>(GetProcAddress(
          GetModuleHandle(L"userenv"), "GetAppContainerFolderPath"));
  if (!get_app_container_folder_path)
    return false;
  std::wstring sddl_str;
  if (!package_sid_.ToSddlString(&sddl_str))
    return false;
  base::win::ScopedCoMem<wchar_t> path_str;
  if (FAILED(get_app_container_folder_path(sddl_str.c_str(), &path_str)))
    return false;
  *file_path = base::FilePath(path_str.get());
  return true;
}

bool AppContainerBase::GetPipePath(const wchar_t* pipe_name,
                                   base::FilePath* pipe_path) {
  std::wstring sddl_str;
  if (!package_sid_.ToSddlString(&sddl_str))
    return false;
  *pipe_path = base::FilePath(base::StringPrintf(L"\\\\.\\pipe\\%ls\\%ls",
                                                 sddl_str.c_str(), pipe_name));
  return true;
}

bool AppContainerBase::AccessCheck(const wchar_t* object_name,
                                   SE_OBJECT_TYPE object_type,
                                   DWORD desired_access,
                                   DWORD* granted_access,
                                   BOOL* access_status) {
  GENERIC_MAPPING generic_mapping;
  if (!GetGenericMappingForType(object_type, &generic_mapping))
    return false;
  MapGenericMask(&desired_access, &generic_mapping);
  PSECURITY_DESCRIPTOR sd = nullptr;
  PACL dacl = nullptr;
  if (GetNamedSecurityInfo(
          object_name, object_type,
          OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION |
              DACL_SECURITY_INFORMATION | LABEL_SECURITY_INFORMATION,
          nullptr, nullptr, &dacl, nullptr, &sd) != ERROR_SUCCESS) {
    return false;
  }

  std::unique_ptr<void, LocalFreeDeleter> sd_ptr(sd);

  if (enable_low_privilege_app_container_) {
    Sid any_package_sid(::WinBuiltinAnyPackageSid);
    // We can't create a LPAC token directly, so modify the DACL to simulate it.
    // Set mask for ALL APPLICATION PACKAGE Sid to 0.
    for (WORD index = 0; index < dacl->AceCount; ++index) {
      PVOID temp_ace;
      if (!GetAce(dacl, index, &temp_ace))
        return false;
      PACE_HEADER header = static_cast<PACE_HEADER>(temp_ace);
      if ((header->AceType != ACCESS_ALLOWED_ACE_TYPE) &&
          (header->AceType != ACCESS_DENIED_ACE_TYPE)) {
        continue;
      }
      // Allowed and deny aces have the same underlying structure.
      PACCESS_ALLOWED_ACE ace = static_cast<PACCESS_ALLOWED_ACE>(temp_ace);
      if (!::IsValidSid(&ace->SidStart)) {
        continue;
      }
      if (::EqualSid(&ace->SidStart, any_package_sid.GetPSID())) {
        ace->Mask = 0;
      }
    }
  }

  PRIVILEGE_SET priv_set = {};
  DWORD priv_set_length = sizeof(PRIVILEGE_SET);

  base::win::ScopedHandle token;
  if (BuildLowBoxToken(&token) != SBOX_ALL_OK)
    return false;

  return !!::AccessCheck(sd, token.Get(), desired_access, &generic_mapping,
                         &priv_set, &priv_set_length, granted_access,
                         access_status);
}

bool AppContainerBase::AddCapability(const wchar_t* capability_name) {
  return AddCapability(Sid::FromNamedCapability(capability_name), false);
}

bool AppContainerBase::AddCapability(WellKnownCapabilities capability) {
  return AddCapability(Sid::FromKnownCapability(capability), false);
}

bool AppContainerBase::AddCapabilitySddl(const wchar_t* sddl_sid) {
  return AddCapability(Sid::FromSddlString(sddl_sid), false);
}

bool AppContainerBase::AddCapability(const Sid& capability_sid,
                                     bool impersonation_only) {
  if (!capability_sid.IsValid())
    return false;
  if (!impersonation_only)
    capabilities_.push_back(capability_sid);
  impersonation_capabilities_.push_back(capability_sid);
  return true;
}

bool AppContainerBase::AddImpersonationCapability(
    const wchar_t* capability_name) {
  return AddCapability(Sid::FromNamedCapability(capability_name), true);
}

bool AppContainerBase::AddImpersonationCapability(
    WellKnownCapabilities capability) {
  return AddCapability(Sid::FromKnownCapability(capability), true);
}

bool AppContainerBase::AddImpersonationCapabilitySddl(const wchar_t* sddl_sid) {
  return AddCapability(Sid::FromSddlString(sddl_sid), true);
}

const std::vector<Sid>& AppContainerBase::GetCapabilities() {
  return capabilities_;
}

const std::vector<Sid>& AppContainerBase::GetImpersonationCapabilities() {
  return impersonation_capabilities_;
}

Sid AppContainerBase::GetPackageSid() const {
  return package_sid_;
}

void AppContainerBase::SetEnableLowPrivilegeAppContainer(bool enable) {
  enable_low_privilege_app_container_ = enable;
}

bool AppContainerBase::GetEnableLowPrivilegeAppContainer() {
  return enable_low_privilege_app_container_;
}

AppContainerType AppContainerBase::GetAppContainerType() {
  return type_;
}

std::unique_ptr<SecurityCapabilities>
AppContainerBase::GetSecurityCapabilities() {
  return std::make_unique<SecurityCapabilities>(package_sid_, capabilities_);
}

ResultCode AppContainerBase::BuildLowBoxToken(
    base::win::ScopedHandle* token,
    base::win::ScopedHandle* lockdown) {
  if (type_ == AppContainerType::kLowbox) {
    if (!lowbox_directory_.IsValid()) {
      DWORD result = CreateLowBoxObjectDirectory(package_sid_.GetPSID(), true,
                                                 &lowbox_directory_);
      DCHECK(result == ERROR_SUCCESS);
    }

    // The order of handles isn't important in the CreateLowBoxToken call.
    // The kernel will maintain a reference to the object directory handle.
    HANDLE saved_handles[1] = {lowbox_directory_.Get()};
    DWORD saved_handles_count = lowbox_directory_.IsValid() ? 1 : 0;

    if (CreateLowBoxToken(lockdown->Get(), PRIMARY,
                          GetSecurityCapabilities().get(), saved_handles,
                          saved_handles_count, token) != ERROR_SUCCESS) {
      return SBOX_ERROR_CANNOT_CREATE_LOWBOX_TOKEN;
    }

    if (!ReplacePackageSidInDacl(token->Get(), SE_KERNEL_OBJECT, package_sid_,
                                 TOKEN_ALL_ACCESS)) {
      return SBOX_ERROR_CANNOT_MODIFY_LOWBOX_TOKEN_DACL;
    }
  } else if (CreateLowBoxToken(nullptr, IMPERSONATION,
                               GetSecurityCapabilities().get(), nullptr, 0,
                               token) != ERROR_SUCCESS) {
    return SBOX_ERROR_CANNOT_CREATE_LOWBOX_IMPERSONATION_TOKEN;
  }

  return SBOX_ALL_OK;
}

}  // namespace sandbox
