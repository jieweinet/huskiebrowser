// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'chrome://resources/cr_elements/icons.m.js';
import 'chrome://resources/polymer/v3_0/iron-icon/iron-icon.js';
import './shimless_rma_shared_css.js';
import './base_page.js';
import './icons.js';

import {html, PolymerElement} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';

import {getShimlessRmaService} from './mojo_interface_provider.js';
import {ProvisioningObserverInterface, ProvisioningObserverReceiver, ProvisioningStep, ShimlessRmaServiceInterface, StateResult} from './shimless_rma_types.js';

// TODO(gavindodd): Update text for i18n
/** @type {!Object<!ProvisioningStep, string>} */
const provisioningStepText = {
  [ProvisioningStep.kProvisioningUnknown]: 'Starting...',
  [ProvisioningStep.kInProgress]: 'In progress...',
  [ProvisioningStep.kProvisioningComplete]: 'Complete.',
};

/**
 * @fileoverview
 * 'reimaging-provisioning-page' enter updated device information if needed.
 *
 * Currently device information is serial number, region and sku. All values are
 * OEM specific.
 */
export class ReimagingProvisioningPageElement extends PolymerElement {
  static get is() {
    return 'reimaging-provisioning-page';
  }

  static get template() {
    return html`{__html_template__}`;
  }

  static get properties() {
    return {
      /** @protected {!ProvisioningStep} */
      step_: {
        type: Object,
        value: ProvisioningStep.kProvisioningUnknown,
      },

      /** @protected */
      progress_: {
        type: Number,
        value: 0.0,
      },

      /** @protected */
      statusString_: {
        type: String,
        computed: 'getStatusString_(step_, progress_)',
      },
    };
  }

  constructor() {
    super();
    /** @private {ShimlessRmaServiceInterface} */
    this.shimlessRmaService_ = getShimlessRmaService();
    /** @private {ProvisioningObserverReceiver} */
    this.provisioningObserverReceiver_ = new ProvisioningObserverReceiver(
        /**
         * @type {!ProvisioningObserverInterface}
         */
        (this));

    this.shimlessRmaService_.observeProvisioningProgress(
        this.provisioningObserverReceiver_.$.bindNewPipeAndPassRemote());
  }

  /**
   * @protected
   * @return {string}
   */
  getStatusString_() {
    // TODO(gavindodd): Update text for i18n
    return provisioningStepText[this.step_] + ' ' +
        Math.round(this.progress_ * 100) + '%';
  }

  /**
   * Implements ProvisioningObserver.onProvisioningUpdated()
   * TODO(joonbug): Add error handling and display failure using cr-dialog.
   * @protected
   * @param {!ProvisioningStep} step
   * @param {number} progress
   */
  onProvisioningUpdated(step, progress) {
    this.step_ = step;
    this.progress_ = progress;
    let disabled = this.step_ != ProvisioningStep.kProvisioningComplete;
    this.dispatchEvent(new CustomEvent(
        'disable-next-button',
        {bubbles: true, composed: true, detail: disabled},
        ));
  }

  /** @return {!Promise<!StateResult>} */
  onNextButtonClick() {
    if (this.step_ == ProvisioningStep.kProvisioningComplete) {
      return this.shimlessRmaService_.provisioningComplete();
    } else {
      return Promise.reject(new Error('Provisioning is not complete.'));
    }
  }
};

customElements.define(
    ReimagingProvisioningPageElement.is, ReimagingProvisioningPageElement);
