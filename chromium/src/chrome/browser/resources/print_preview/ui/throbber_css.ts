// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import './print_preview_vars_css.js';

const template = document.createElement('template');
template.innerHTML = `
<dom-module id="throbber">{__html_template__}</dom-module>
`;
document.body.appendChild(template.content.cloneNode(true));
