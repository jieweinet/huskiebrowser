// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/ui/download/features.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

const base::Feature kDownloadMobileConfigFile{
    "DownloadMobileconfigFile", base::FEATURE_DISABLED_BY_DEFAULT};

const base::Feature kDownloadVcard{"DownloadVcardFile",
                                   base::FEATURE_DISABLED_BY_DEFAULT};
