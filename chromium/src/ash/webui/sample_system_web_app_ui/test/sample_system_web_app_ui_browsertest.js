// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview Test suite for chrome://sample-system-web-app.
 */

GEN('#include "content/public/test/browser_test.h"');

const HOST_ORIGIN = 'chrome://sample-system-web-app';
const UNTRUSTED_HOST_ORIGIN = 'chrome-untrusted://sample-system-web-app';

var SampleSystemWebAppUIBrowserTest = class extends testing.Test {
  /** @override */
  get browsePreload() {
    return HOST_ORIGIN;
  }

  /** @override */
  get runAccessibilityChecks() {
    return false;
  }

  /** @override */
  get isAsync() {
    return true;
  }
};

// Tests that chrome://sample-system-web-app runs js file and that it goes
// somewhere instead of 404ing or crashing.
TEST_F('SampleSystemWebAppUIBrowserTest', 'HasChromeSchemeURL', () => {
  const header = document.querySelector('header');

  assertEquals(header.innerText, 'Sample System Web App');
  assertEquals(document.location.origin, HOST_ORIGIN);
  testDone();
});

// Test the ability to get information from the page handler.
TEST_F('SampleSystemWebAppUIBrowserTest', 'FetchPreferences', async () => {
  const {preferences} = await window.pageHandler.getPreferences();
  assertDeepEquals({background: '#ffffff', foreground: '#000000'}, preferences);
  testDone();
})

// Test the ability to trigger work in the page handler.
TEST_F('SampleSystemWebAppUIBrowserTest', 'DoSomething', async () => {
  const pageHandler = window.pageHandler;
  const callbackRouter = window.callbackRouter;

  // Now execute our test: zero the event count and call doSomething.
  window.eventCount.set('DoSomething is done', 0)
  pageHandler.doSomething();

  // Ensure the DoSomething() is called on the browser side.
  await pageHandler.$.flushForTesting();

  // Await the C++ process to call back with the event.
  await callbackRouter.$.flush();
  // Verify the expected event count.
  assertEquals(1, window.eventCount.get('DoSomething is done'));

  testDone();
})

var SampleSystemWebAppUIUntrustedBrowserTest = class extends testing.Test {
  /** @override */
  get browsePreload() {
    return HOST_ORIGIN + '/sandbox.html';
  }

  /** @override */
  get runAccessibilityChecks() {
    return false;
  }

  /** @override */
  get isAsync() {
    return true;
  }
};

// Tests that chrome://sample-system-web-app/sandbox.html embeds a
// chrome-untrusted:// iframe
TEST_F(
    'SampleSystemWebAppUIUntrustedBrowserTest', 'HasChromeUntrustedIframe',
    () => {
      const iframe = document.querySelector('iframe');
      window.onmessage = (event) => {
        assertEquals(event.origin, UNTRUSTED_HOST_ORIGIN);
        assertEquals(event.data.success, true);
        testDone();
      };
      iframe.contentWindow.postMessage('hello', UNTRUSTED_HOST_ORIGIN);
    });
