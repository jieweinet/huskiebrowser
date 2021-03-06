// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_SSL_TLS_DEPRECATION_TEST_UTILS_H_
#define CHROME_BROWSER_SSL_TLS_DEPRECATION_TEST_UTILS_H_

#include <memory>

namespace content {
class NavigationSimulator;
class WebContents;
}  // namespace content

class GURL;

const char kLegacyTLSHost[] = "example-nonsecure.test";
const char kLegacyTLSURL[] = "https://example-nonsecure.test";

// Creates and starts a simulated navigation using the specified SSL protocol
// version (e.g., net::SSL_CONNECTION_VERSION_TLS1_2).
std::unique_ptr<content::NavigationSimulator> CreateTLSNavigation(
    const GURL& url,
    content::WebContents* web_contents,
    uint16_t ssl_protocol_version);

// Creates and starts a simulated navigation using TLS 1.0.
std::unique_ptr<content::NavigationSimulator> CreateLegacyTLSNavigation(
    const GURL& url,
    content::WebContents* web_contents);

// Creates and starts a simulated navigation using TLS 1.2.
std::unique_ptr<content::NavigationSimulator> CreateNonlegacyTLSNavigation(
    const GURL& url,
    content::WebContents* web_contents);

#endif  // CHROME_BROWSER_SSL_TLS_DEPRECATION_TEST_UTILS_H_
