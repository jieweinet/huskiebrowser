// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_POLICY_URL_BLOCKING_POLICY_TEST_UTILS_H_
#define CHROME_BROWSER_POLICY_URL_BLOCKING_POLICY_TEST_UTILS_H_

#include <string>

#include "chrome/browser/policy/policy_test_utils.h"

class Browser;
class GURL;

namespace content {
class WebContents;
}  // namespace content

namespace policy {

class UrlBlockingPolicyTest : public PolicyTest {
 protected:
  UrlBlockingPolicyTest();
  ~UrlBlockingPolicyTest() override;

  // Verifies that access to the given url |spec| is blocked.
  void CheckURLIsBlockedInWebContents(content::WebContents* web_contents,
                                      const GURL& url);

  // Verifies that access to the given url |spec| is blocked.
  void CheckURLIsBlocked(Browser* browser, const std::string& spec);
};

}  // namespace policy

#endif  // CHROME_BROWSER_POLICY_URL_BLOCKING_POLICY_TEST_UTILS_H_
