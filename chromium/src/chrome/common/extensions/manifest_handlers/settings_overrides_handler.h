// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_COMMON_EXTENSIONS_MANIFEST_HANDLERS_SETTINGS_OVERRIDES_HANDLER_H_
#define CHROME_COMMON_EXTENSIONS_MANIFEST_HANDLERS_SETTINGS_OVERRIDES_HANDLER_H_

#include <memory>

#include "base/macros.h"
#include "chrome/common/extensions/api/manifest_types.h"
#include "extensions/common/extension.h"
#include "extensions/common/manifest_handler.h"

namespace extensions {

enum SettingsApiOverrideType {
  BUBBLE_TYPE_HOME_PAGE = 0,
  BUBBLE_TYPE_SEARCH_ENGINE,
  BUBBLE_TYPE_STARTUP_PAGES,
};

// SettingsOverride is associated with "chrome_settings_overrides" manifest key.
// An extension can add a search engine as default or non-default, overwrite the
// homepage and append a startup page to the list.
struct SettingsOverrides : public Extension::ManifestData {
  SettingsOverrides();

  SettingsOverrides(const SettingsOverrides&) = delete;
  SettingsOverrides& operator=(const SettingsOverrides&) = delete;

  ~SettingsOverrides() override;

  static const SettingsOverrides* Get(const Extension* extension);

  std::unique_ptr<api::manifest_types::ChromeSettingsOverrides::SearchProvider>
      search_engine;
  std::unique_ptr<GURL> homepage;
  std::vector<GURL> startup_pages;
};

class SettingsOverridesHandler : public ManifestHandler {
 public:
  SettingsOverridesHandler();

  SettingsOverridesHandler(const SettingsOverridesHandler&) = delete;
  SettingsOverridesHandler& operator=(const SettingsOverridesHandler&) = delete;

  ~SettingsOverridesHandler() override;

  bool Parse(Extension* extension, std::u16string* error) override;

 private:
  base::span<const char* const> Keys() const override;
};

}  // namespace extensions
#endif  // CHROME_COMMON_EXTENSIONS_MANIFEST_HANDLERS_SETTINGS_OVERRIDES_HANDLER_H_
