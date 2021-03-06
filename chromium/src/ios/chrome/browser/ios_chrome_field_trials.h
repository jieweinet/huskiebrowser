// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_IOS_CHROME_FIELD_TRIALS_H_
#define IOS_CHROME_BROWSER_IOS_CHROME_FIELD_TRIALS_H_

#include "base/macros.h"
#include "components/variations/platform_field_trials.h"

// Responsible for setting up field trials specific to iOS. Currently all
// functions are stubs, as iOS has no specific field trials.
class IOSChromeFieldTrials : public variations::PlatformFieldTrials {
 public:
  IOSChromeFieldTrials() {}

  IOSChromeFieldTrials(const IOSChromeFieldTrials&) = delete;
  IOSChromeFieldTrials& operator=(const IOSChromeFieldTrials&) = delete;

  ~IOSChromeFieldTrials() override {}

  // variations::PlatformFieldTrials:
  void SetUpFieldTrials() override;
  void SetUpFeatureControllingFieldTrials(
      bool has_seed,
      const base::FieldTrial::EntropyProvider* low_entropy_provider,
      base::FeatureList* feature_list) override;
};

#endif  // IOS_CHROME_BROWSER_IOS_CHROME_FIELD_TRIALS_H_
