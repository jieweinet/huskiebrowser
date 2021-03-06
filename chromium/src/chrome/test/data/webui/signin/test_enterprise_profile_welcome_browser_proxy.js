// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import {TestBrowserProxy} from '../test_browser_proxy.js';

/** @implements {EnterpriseProfileWelcomeBrowserProxy} */
export class TestEnterpriseProfileWelcomeBrowserProxy extends TestBrowserProxy {
  /** @param {!EnterpriseProfileInfo} info */
  constructor(info) {
    super([
      'initialized',
      'proceed',
      'cancel',
    ]);
    /** @private {!EnterpriseProfileInfo} */
    this.enterpriseProfileInfo_ = info;
  }

  /** @param {!EnterpriseProfileInfo} info */
  setEnterpriseProfileInfo(info) {
    this.enterpriseProfileInfo_ = info;
  }

  /** @override */
  initialized() {
    this.methodCalled('initialized');
    return Promise.resolve(this.enterpriseProfileInfo_);
  }

  /** @param {number} height */
  initializedWithSize(height) {
    this.methodCalled('initializedWithSize');
  }

  /** @override */
  proceed() {
    this.methodCalled('proceed');
  }

  /** @override */
  cancel() {
    this.methodCalled('cancel');
  }
}
