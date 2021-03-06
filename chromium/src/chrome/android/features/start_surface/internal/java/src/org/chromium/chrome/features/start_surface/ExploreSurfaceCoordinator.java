// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.features.start_surface;

import android.app.Activity;
import android.view.MotionEvent;
import android.view.ViewGroup;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import org.chromium.base.supplier.Supplier;
import org.chromium.chrome.browser.app.feed.FeedActionDelegateImpl;
import org.chromium.chrome.browser.bookmarks.BookmarkBridge;
import org.chromium.chrome.browser.feed.FeedLaunchReliabilityLoggingState;
import org.chromium.chrome.browser.feed.FeedSurfaceCoordinator;
import org.chromium.chrome.browser.feed.FeedSurfaceLifecycleManager;
import org.chromium.chrome.browser.feed.FeedSwipeRefreshLayout;
import org.chromium.chrome.browser.feed.ScrollableContainerDelegate;
import org.chromium.chrome.browser.feed.shared.FeedSurfaceDelegate;
import org.chromium.chrome.browser.feedback.HelpAndFeedbackLauncherImpl;
import org.chromium.chrome.browser.ntp.NewTabPageLaunchOrigin;
import org.chromium.chrome.browser.privacy.settings.PrivacyPreferencesManagerImpl;
import org.chromium.chrome.browser.profiles.Profile;
import org.chromium.chrome.browser.share.ShareDelegate;
import org.chromium.chrome.browser.tab.Tab;
import org.chromium.chrome.browser.tabmodel.TabModelSelector;
import org.chromium.chrome.browser.toolbar.top.Toolbar;
import org.chromium.chrome.browser.ui.messages.snackbar.SnackbarManager;
import org.chromium.chrome.start_surface.R;
import org.chromium.components.browser_ui.bottomsheet.BottomSheetController;
import org.chromium.ui.base.WindowAndroid;
import org.chromium.ui.modelutil.PropertyModel;
import org.chromium.ui.modelutil.PropertyModelChangeProcessor;

/** The coordinator to control the explore surface. */
class ExploreSurfaceCoordinator implements FeedSurfaceDelegate {
    private final Activity mActivity;
    private final PropertyModelChangeProcessor mPropertyModelChangeProcessor;
    private final FeedSurfaceController mFeedSurfaceController;
    private final Supplier<Tab> mParentTabSupplier;
    private final SnackbarManager mSnackbarManager;
    private final Supplier<ShareDelegate> mShareDelegateSupplier;
    private final WindowAndroid mWindowAndroid;
    private final TabModelSelector mTabModelSelector;
    private ExploreSurfaceFeedLifecycleManager mExploreSurfaceFeedLifecycleManager;

    // mExploreSurfaceNavigationDelegate is lightweight, we keep it across FeedSurfaceCoordinators
    // after creating it during the first show.
    private ExploreSurfaceNavigationDelegate mExploreSurfaceNavigationDelegate;

    /** Interface to control the {@link FeedSurfaceDelegate} */
    interface FeedSurfaceController {
        /**
         * Creates the {@link FeedSurfaceCoordinator} for the specified mode.
         * @param isInNightMode Whether or not the feed surface is going to display in night mode.
         * @param launchOrigin Where the feed was launched from.
         * @return The {@link FeedSurfaceCoordinator}.
         */
        FeedSurfaceCoordinator createFeedSurfaceCoordinator(boolean isInNightMode,
                boolean isPlaceholderShown, @NewTabPageLaunchOrigin int launchOrigin);
    }

    /**
     * @param activity The current {@link Activity}.
     * @param parentView The parent {@link ViewGroup} for the start surface.
     * @param containerPropertyModel The {@link PropertyModel} for the container.
     * @param bottomSheetController Controls the state of the bottom sheet.
     * @param parentTabSupplier Supplies the current {@link Tab}.
     * @param scrollableContainerDelegate Delegate for the scrollable container.
     * @param snackbarManager Manages the snackbar.
     * @param shareDelegateSupplier Supplies the {@link ShareDelegate}.
     * @param windowAndroid The current {@link WindowAndroid}.
     * @param tabModelSelector The current {@link TabModelSelector}.
     * @param toolbarSupplier Supplies the {@link Toolbar}.
     * @param feedLaunchReliabilityLoggingState Holds the state for feed surface creation.
     * @param swipeRefreshLayout The layout to support pull-to-refresg.
     */
    ExploreSurfaceCoordinator(@NonNull Activity activity, @NonNull ViewGroup parentView,
            @NonNull PropertyModel containerPropertyModel,
            @NonNull BottomSheetController bottomSheetController,
            @NonNull Supplier<Tab> parentTabSupplier,
            @NonNull ScrollableContainerDelegate scrollableContainerDelegate,
            @NonNull SnackbarManager snackbarManager,
            @NonNull Supplier<ShareDelegate> shareDelegateSupplier,
            @NonNull WindowAndroid windowAndroid, @NonNull TabModelSelector tabModelSelector,
            @NonNull Supplier<Toolbar> toolbarSupplier,
            FeedLaunchReliabilityLoggingState feedLaunchReliabilityLoggingState,
            @Nullable FeedSwipeRefreshLayout swipeRefreshLayout) {
        mActivity = activity;
        mParentTabSupplier = parentTabSupplier;
        mSnackbarManager = snackbarManager;
        mShareDelegateSupplier = shareDelegateSupplier;
        mWindowAndroid = windowAndroid;
        mTabModelSelector = tabModelSelector;

        mPropertyModelChangeProcessor = PropertyModelChangeProcessor.create(
                containerPropertyModel, parentView, ExploreSurfaceViewBinder::bind);
        mFeedSurfaceController = new FeedSurfaceController() {
            @Override
            public FeedSurfaceCoordinator createFeedSurfaceCoordinator(boolean isInNightMode,
                    boolean isPlaceholderShown, @NewTabPageLaunchOrigin int launchOrigin) {
                return internalCreateFeedSurfaceCoordinator(isInNightMode, isPlaceholderShown,
                        bottomSheetController, scrollableContainerDelegate, launchOrigin,
                        toolbarSupplier, feedLaunchReliabilityLoggingState, swipeRefreshLayout,
                        parentView);
            }
        };
    }

    /**
     * Gets the {@link FeedSurfaceController}.
     * @return the {@link FeedSurfaceController}.
     */
    FeedSurfaceController getFeedSurfaceController() {
        return mFeedSurfaceController;
    }

    // Implements FeedSurfaceDelegate.
    @Override
    public FeedSurfaceLifecycleManager createStreamLifecycleManager(
            Activity activity, FeedSurfaceCoordinator coordinator) {
        mExploreSurfaceFeedLifecycleManager =
                new ExploreSurfaceFeedLifecycleManager(activity, coordinator);
        return mExploreSurfaceFeedLifecycleManager;
    }

    @Override
    public boolean onInterceptTouchEvent(MotionEvent ev) {
        return false;
    }

    private FeedSurfaceCoordinator internalCreateFeedSurfaceCoordinator(boolean isInNightMode,
            boolean isPlaceholderShown, BottomSheetController bottomSheetController,
            ScrollableContainerDelegate scrollableContainerDelegate,
            @NewTabPageLaunchOrigin int launchOrigin, @NonNull Supplier<Toolbar> toolbarSupplier,
            FeedLaunchReliabilityLoggingState feedLaunchReliabilityLoggingState,
            FeedSwipeRefreshLayout swipeRefreshLayout, ViewGroup parentView) {
        if (mExploreSurfaceNavigationDelegate == null) {
            mExploreSurfaceNavigationDelegate =
                    new ExploreSurfaceNavigationDelegate(mParentTabSupplier);
        }
        Profile profile = Profile.getLastUsedRegularProfile();

        FeedSurfaceCoordinator feedSurfaceCoordinator = new FeedSurfaceCoordinator(mActivity,
                mSnackbarManager, mWindowAndroid, null, null,
                mActivity.getResources().getDimensionPixelSize(R.dimen.toolbar_height_no_shadow),
                isInNightMode, this, profile, isPlaceholderShown, bottomSheetController,
                mShareDelegateSupplier, scrollableContainerDelegate, launchOrigin,
                PrivacyPreferencesManagerImpl.getInstance(), toolbarSupplier,
                feedLaunchReliabilityLoggingState, swipeRefreshLayout, /*overScrollDisabled=*/true,
                parentView,
                new FeedActionDelegateImpl(mActivity, mSnackbarManager,
                        mExploreSurfaceNavigationDelegate, new BookmarkBridge(profile)),
                HelpAndFeedbackLauncherImpl.getInstance());
        feedSurfaceCoordinator.getView().setId(R.id.start_surface_explore_view);
        return feedSurfaceCoordinator;
        // TODO(crbug.com/982018): Customize surface background for incognito and dark mode.
        // TODO(crbug.com/982018): Hide signin promo UI in incognito mode.
    }
}
