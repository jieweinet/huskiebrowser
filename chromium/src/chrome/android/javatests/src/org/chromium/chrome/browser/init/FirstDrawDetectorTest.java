// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.init;

import android.view.View;
import android.widget.FrameLayout;

import androidx.test.filters.SmallTest;

import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

import org.chromium.chrome.R;
import org.chromium.chrome.test.ChromeJUnit4ClassRunner;
import org.chromium.content_public.browser.test.util.TestThreadUtils;
import org.chromium.ui.test.util.DummyUiActivity;
import org.chromium.ui.test.util.ThemedDummyUiActivityTestRule;

import java.util.concurrent.CountDownLatch;

/** Tests for {@link FirstDrawDetector}. */
@RunWith(ChromeJUnit4ClassRunner.class)
public class FirstDrawDetectorTest {
    @Rule
    public ThemedDummyUiActivityTestRule<DummyUiActivity> mActivityTestRule =
            new ThemedDummyUiActivityTestRule<>(
                    DummyUiActivity.class, R.style.ColorOverlay_ChromiumAndroid);

    @Test
    @SmallTest
    public void testFirstDraw() throws Exception {
        mActivityTestRule.launchActivity(null);
        final CountDownLatch firstDrawEvent = new CountDownLatch(1);
        TestThreadUtils.runOnUiThreadBlocking(() -> {
            DummyUiActivity activity = mActivityTestRule.getActivity();
            View view = new FrameLayout(activity);
            activity.setContentView(view);

            FirstDrawDetector.waitForFirstDraw(view, () -> firstDrawEvent.countDown());
        });
        firstDrawEvent.await();
    }
}
