// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.password_manager;

import static org.chromium.base.ThreadUtils.assertOnUiThread;

import org.chromium.base.Callback;

/**
 * This factory returns an implementation for the backend. The factory itself is implemented
 * downstream, too.
 */
public abstract class PasswordStoreAndroidBackendFactory {
    class FakePasswordStoreAndroidBackend implements PasswordStoreAndroidBackend {
        public FakePasswordStoreAndroidBackend() {}

        @Override
        public void getAllLogins(
                Callback<byte[]> loginsReply, Callback<Exception> failureCallback) {
            failureCallback.onResult(new Exception("Not implemented"));
        }
    }

    private static PasswordStoreAndroidBackendFactory sInstance;

    /**
     * Returns a backend factory to be invoked whenever {@link #createBackend()} is called. If no
     * factory was used yet, it is created.
     *
     * @return The shared {@link PasswordStoreAndroidBackendFactory} instance.
     */
    public static PasswordStoreAndroidBackendFactory getInstance() {
        assertOnUiThread();
        if (sInstance == null) sInstance = new PasswordStoreAndroidBackendFactoryImpl();
        return sInstance;
    }

    /**
     * Returns the downstream implementation provided by subclasses.
     *
     * @return A non-null implementation of the {@link PasswordStoreAndroidBackend}.
     */
    public PasswordStoreAndroidBackend createBackend() {
        return new FakePasswordStoreAndroidBackend();
    }
}
