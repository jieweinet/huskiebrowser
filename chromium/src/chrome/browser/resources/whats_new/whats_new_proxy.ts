// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import {sendWithPromise} from 'chrome://resources/js/cr.m.js';

export interface WhatsNewProxy {
  initialize(isAutoOpen: boolean): Promise<string>;
}

export class WhatsNewProxyImpl implements WhatsNewProxy {
  initialize(isAutoOpen: boolean): Promise<string> {
    return sendWithPromise('initialize', isAutoOpen);
  }

  static getInstance(): WhatsNewProxy {
    return instance || (instance = new WhatsNewProxyImpl());
  }

  static setInstance(obj: WhatsNewProxy) {
    instance = obj;
  }
}

let instance: WhatsNewProxy|null = null;
