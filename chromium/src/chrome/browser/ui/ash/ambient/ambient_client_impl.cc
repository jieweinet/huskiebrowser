// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/ash/ambient/ambient_client_impl.h"

#include <string>
#include <utility>

#include "ash/constants/ash_features.h"
#include "ash/public/cpp/ambient/ambient_prefs.h"
#include "ash/public/cpp/image_downloader.h"
#include "base/callback.h"
#include "chrome/browser/ash/login/demo_mode/demo_session.h"
#include "chrome/browser/ash/profiles/profile_helper.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/signin/identity_manager_factory.h"
#include "chrome/common/channel_info.h"
#include "components/account_id/account_id.h"
#include "components/prefs/pref_service.h"
#include "components/signin/public/base/consent_level.h"
#include "components/signin/public/identity_manager/access_token_fetcher.h"
#include "components/signin/public/identity_manager/access_token_info.h"
#include "components/signin/public/identity_manager/account_info.h"
#include "components/signin/public/identity_manager/identity_manager.h"
#include "components/signin/public/identity_manager/scope_set.h"
#include "components/user_manager/user.h"
#include "components/user_manager/user_manager.h"
#include "components/version_info/channel.h"
#include "content/public/browser/device_service.h"
#include "google_apis/gaia/gaia_auth_util.h"
#include "google_apis/gaia/google_service_auth_error.h"
#include "net/http/http_request_headers.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"

namespace {

constexpr char kPhotosOAuthScope[] = "https://www.googleapis.com/auth/photos";
constexpr char kBackdropOAuthScope[] =
    "https://www.googleapis.com/auth/cast.backdrop";

const user_manager::User* GetActiveUser() {
  return user_manager::UserManager::Get()->GetActiveUser();
}

const user_manager::User* GetPrimaryUser() {
  return user_manager::UserManager::Get()->GetPrimaryUser();
}

Profile* GetProfileForActiveUser() {
  const user_manager::User* const active_user = GetActiveUser();
  DCHECK(active_user);
  return chromeos::ProfileHelper::Get()->GetProfileByUser(active_user);
}

bool IsPrimaryUser() {
  return GetActiveUser() == GetPrimaryUser();
}

bool HasPrimaryAccount(const Profile* profile) {
  auto* identity_manager =
      IdentityManagerFactory::GetForProfileIfExists(profile);
  if (!identity_manager)
    return false;

  return identity_manager->HasPrimaryAccount(signin::ConsentLevel::kSignin);
}

bool IsEmailDomainSupported(const user_manager::User* user) {
  const std::string email = user->GetAccountId().GetUserEmail();
  DCHECK(!email.empty());

  constexpr char kGmailDomain[] = "gmail.com";
  constexpr char kGooglemailDomain[] = "googlemail.com";
  return (gaia::ExtractDomainName(email) == kGmailDomain ||
          gaia::ExtractDomainName(email) == kGooglemailDomain ||
          gaia::IsGoogleInternalAccountEmail(email));
}

}  // namespace

AmbientClientImpl::AmbientClientImpl() = default;

AmbientClientImpl::~AmbientClientImpl() = default;

bool AmbientClientImpl::IsAmbientModeAllowed() {
  DCHECK(chromeos::features::IsAmbientModeEnabled());

  if (ash::DemoSession::IsDeviceInDemoMode())
    return false;

  const user_manager::User* const active_user = GetActiveUser();
  if (!active_user || !active_user->HasGaiaAccount())
    return false;

  if (!IsPrimaryUser())
    return false;

  if (!IsEmailDomainSupported(active_user))
    return false;

  auto* profile = GetProfileForActiveUser();
  if (!profile)
    return false;

  // Primary account might be missing during unittests.
  if (!HasPrimaryAccount(profile))
    return false;

  if (profile->IsOffTheRecord())
    return false;

  return true;
}

void AmbientClientImpl::RequestAccessToken(GetAccessTokenCallback callback) {
  auto* profile = GetProfileForActiveUser();
  DCHECK(profile);

  signin::IdentityManager* identity_manager =
      IdentityManagerFactory::GetForProfile(profile);
  DCHECK(identity_manager);

  CoreAccountInfo account_info =
      identity_manager->GetPrimaryAccountInfo(signin::ConsentLevel::kSignin);
  const signin::ScopeSet scopes{kPhotosOAuthScope, kBackdropOAuthScope};
  auto fetcher_id = base::UnguessableToken::Create();
  auto access_token_fetcher =
      identity_manager->CreateAccessTokenFetcherForAccount(
          account_info.account_id,
          /*oauth_consumer_name=*/"ChromeOS_AmbientMode", scopes,
          base::BindOnce(&AmbientClientImpl::OnGetAccessToken,
                         weak_factory_.GetWeakPtr(), std::move(callback),
                         fetcher_id, account_info.gaia),
          signin::AccessTokenFetcher::Mode::kImmediate);

  token_fetchers_.insert({fetcher_id, std::move(access_token_fetcher)});
}

void AmbientClientImpl::DownloadImage(
    const std::string& url,
    ash::ImageDownloader::DownloadCallback callback) {
  if (ash::features::IsAmbientModeNewUrlEnabled()) {
    RequestAccessToken(base::BindOnce(
        [](const std::string& url,
           ash::ImageDownloader::DownloadCallback callback,
           const std::string& gaia_id, const std::string& access_token,
           const base::Time& expiration_time) {
          if (access_token.empty()) {
            std::move(callback).Run({});
            return;
          }
          net::HttpRequestHeaders headers;
          headers.SetHeader("Authorization", "Bearer " + access_token);

          ash::ImageDownloader::Get()->Download(GURL(url),
                                                NO_TRAFFIC_ANNOTATION_YET,
                                                headers, std::move(callback));
        },
        url, std::move(callback)));
  } else {
    ash::ImageDownloader::Get()->Download(GURL(url), NO_TRAFFIC_ANNOTATION_YET,
                                          /*additional_headers=*/{},
                                          std::move(callback));
  }
}

scoped_refptr<network::SharedURLLoaderFactory>
AmbientClientImpl::GetURLLoaderFactory() {
  auto* profile = GetProfileForActiveUser();
  DCHECK(profile);

  return profile->GetURLLoaderFactory();
}

void AmbientClientImpl::RequestWakeLockProvider(
    mojo::PendingReceiver<device::mojom::WakeLockProvider> receiver) {
  content::GetDeviceService().BindWakeLockProvider(std::move(receiver));
}

bool AmbientClientImpl::ShouldUseProdServer() {
  if (chromeos::features::IsAmbientModeDevUseProdEnabled())
    return true;

  auto channel = chrome::GetChannel();
  return channel == version_info::Channel::STABLE ||
         channel == version_info::Channel::BETA;
}

void AmbientClientImpl::OnGetAccessToken(
    GetAccessTokenCallback callback,
    base::UnguessableToken fetcher_id,

    const std::string& gaia_id,
    GoogleServiceAuthError error,
    signin::AccessTokenInfo access_token_info) {
  if (error.state() == GoogleServiceAuthError::NONE) {
    std::move(callback).Run(gaia_id, access_token_info.token,
                            access_token_info.expiration_time);
  } else {
    LOG(ERROR) << "Failed to retrieve token, error: " << error.ToString();
    std::move(callback).Run(/*gaia_id=*/std::string(),
                            /*access_token=*/std::string(),
                            /*expiration_time=*/base::Time::Now());
  }

  token_fetchers_.erase(fetcher_id);
}
