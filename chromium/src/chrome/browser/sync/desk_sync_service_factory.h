// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_SYNC_DESK_SYNC_SERVICE_FACTORY_H_
#define CHROME_BROWSER_SYNC_DESK_SYNC_SERVICE_FACTORY_H_

#include "base/macros.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"

class Profile;

namespace base {
template <typename T>
struct DefaultSingletonTraits;
}  // namespace base

namespace desks_storage {
class DeskSyncService;
}  // namespace desks_storage

// A factory to create DeskSyncService for a given browser context.
class DeskSyncServiceFactory : public BrowserContextKeyedServiceFactory {
 public:
  static desks_storage::DeskSyncService* GetForProfile(Profile* profile);
  static DeskSyncServiceFactory* GetInstance();

 private:
  friend struct base::DefaultSingletonTraits<DeskSyncServiceFactory>;

  DeskSyncServiceFactory();
  DeskSyncServiceFactory(const DeskSyncServiceFactory&) = delete;
  DeskSyncServiceFactory& operator=(const DeskSyncServiceFactory&) = delete;
  ~DeskSyncServiceFactory() override = default;

  // BrowserContextKeyedServiceFactory:
  KeyedService* BuildServiceInstanceFor(
      content::BrowserContext* context) const override;
};

#endif  // CHROME_BROWSER_SYNC_DESK_SYNC_SERVICE_FACTORY_H_
