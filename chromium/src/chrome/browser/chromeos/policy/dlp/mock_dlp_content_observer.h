// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_POLICY_DLP_MOCK_DLP_CONTENT_OBSERVER_H_
#define CHROME_BROWSER_CHROMEOS_POLICY_DLP_MOCK_DLP_CONTENT_OBSERVER_H_

#include "chrome/browser/chromeos/policy/dlp/dlp_content_observer.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "url/gurl.h"

namespace policy {

class MockDlpContentObserver : public DlpContentObserver {
 public:
  MockDlpContentObserver();
  ~MockDlpContentObserver() override;

  MOCK_METHOD2(OnConfidentialityChanged,
               void(content::WebContents*, const DlpContentRestrictionSet&));
  MOCK_METHOD1(OnWebContentsDestroyed, void(content::WebContents*));
  MOCK_CONST_METHOD1(GetRestrictionSetForURL,
                     DlpContentRestrictionSet(const GURL&));
  MOCK_METHOD1(OnVisibilityChanged, void(content::WebContents*));
};

}  // namespace policy

#endif  // CHROME_BROWSER_CHROMEOS_POLICY_DLP_MOCK_DLP_CONTENT_OBSERVER_H_
