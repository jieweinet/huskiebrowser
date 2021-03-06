// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @typedef {{
 *   seccompStatus: number,
 *   pid: string,
 *   uid: string,
 *   secontext: string,
 *   procStatus: string,
 *   androidBuildId: string
 * }}
 */
let AndroidSandboxStatus;

/**
 * This function is only exposed to the Android chrome://sandbox webui.
 * @param {!function(!AndroidSandboxStatus)=} callback
 */
chrome.getAndroidSandboxStatus = function(callback) {};
