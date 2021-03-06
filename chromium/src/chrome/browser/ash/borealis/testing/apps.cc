// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ash/borealis/testing/apps.h"

#include "base/base64.h"
#include "chrome/browser/ash/guest_os/guest_os_registry_service.h"
#include "chrome/browser/ash/guest_os/guest_os_registry_service_factory.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace borealis {

void CreateFakeApp(Profile* profile, std::string desktop_file_id) {
  vm_tools::apps::ApplicationList list;
  list.set_vm_name("borealis");
  list.set_container_name("penguin");
  list.set_vm_type(vm_tools::apps::ApplicationList_VmType_BOREALIS);
  vm_tools::apps::App* app = list.add_apps();
  app->set_desktop_file_id(desktop_file_id);
  vm_tools::apps::App::LocaleString::Entry* entry =
      app->mutable_name()->add_values();
  entry->set_locale(std::string());
  entry->set_value(std::move(desktop_file_id));
  app->set_no_display(false);
  guest_os::GuestOsRegistryServiceFactory::GetForProfile(profile)
      ->UpdateApplicationList(list);
}

void CreateFakeMainApp(Profile* profile) {
  std::string desktop_file_id;
  ASSERT_TRUE(base::Base64Decode("c3RlYW0=", &desktop_file_id));
  CreateFakeApp(profile, std::move(desktop_file_id));
}

}  // namespace borealis
