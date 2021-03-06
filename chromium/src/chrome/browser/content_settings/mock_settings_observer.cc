// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/content_settings/mock_settings_observer.h"

#include "chrome/browser/chrome_notification_types.h"
#include "url/gurl.h"

MockSettingsObserver::MockSettingsObserver(HostContentSettingsMap* map)
    : map_(map) {
  observation_.Observe(map_);
}

MockSettingsObserver::~MockSettingsObserver() = default;

void MockSettingsObserver::OnContentSettingChanged(
    const ContentSettingsPattern& primary_pattern,
    const ContentSettingsPattern& secondary_pattern,
    ContentSettingsTypeSet content_type_set) {
  bool all_hosts =
      primary_pattern.MatchesAllHosts() && secondary_pattern.MatchesAllHosts();
  OnContentSettingsChanged(map_, content_type_set.GetTypeOrDefault(),
                           content_type_set.ContainsAllTypes(), primary_pattern,
                           secondary_pattern, all_hosts);
  // This checks that calling a Get function from an observer doesn't
  // deadlock.
  GURL url("http://random-hostname.com/");
  map_->GetContentSetting(url, url, ContentSettingsType::COOKIES);
}
