// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ash/policy/dlp/dlp_content_manager.h"

#include <string>
#include <vector>

#include "ash/public/cpp/privacy_screen_dlp_helper.h"
#include "base/bind.h"
#include "base/check.h"
#include "base/containers/contains.h"
#include "base/containers/cxx20_erase.h"
#include "base/threading/thread_task_runner_handle.h"
#include "chrome/browser/ash/policy/dlp/dlp_notification_helper.h"
#include "chrome/browser/ash/policy/dlp/dlp_warn_dialog.h"
#include "chrome/browser/chromeos/policy/dlp/dlp_histogram_helper.h"
#include "chrome/browser/chromeos/policy/dlp/dlp_reporting_manager.h"
#include "chrome/browser/chromeos/policy/dlp/dlp_rules_manager.h"
#include "chrome/browser/chromeos/policy/dlp/dlp_rules_manager_factory.h"
#include "chrome/browser/ui/ash/capture_mode/chrome_capture_mode_delegate.h"
#include "content/public/browser/visibility.h"
#include "content/public/browser/web_contents.h"
#include "extensions/browser/guest_view/mime_handler_view/mime_handler_view_guest.h"
#include "ui/aura/window.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/geometry/skia_conversions.h"
#include "url/gurl.h"

namespace policy {

namespace {
// Delay to wait to turn off privacy screen enforcement after confidential data
// becomes not visible. This is done to not blink the privacy screen in case of
// a quick switch from one confidential data to another.
const base::TimeDelta kPrivacyScreenOffDelay = base::Milliseconds(500);

// Reports events to `reporting_manager`.
void ReportEvent(GURL url,
                 DlpRulesManager::Restriction restriction,
                 DlpRulesManager::Level level,
                 DlpReportingManager* reporting_manager) {
  DCHECK(reporting_manager);

  DlpRulesManager* rules_manager =
      DlpRulesManagerFactory::GetForPrimaryProfile();
  if (!rules_manager)
    return;

  const std::string src_url =
      rules_manager->GetSourceUrlPattern(url, restriction, level);

  reporting_manager->ReportEvent(src_url, restriction, level);
}

// Helper method to check whether the restriction level is kBlock
bool IsBlocked(RestrictionLevelAndUrl restriction_info) {
  return restriction_info.level == DlpRulesManager::Level::kBlock;
}

// Helper method to check whether the restriction level is kWarn
bool IsWarn(RestrictionLevelAndUrl restriction_info) {
  return restriction_info.level == DlpRulesManager::Level::kWarn;
}

// Helper method to check if event should be reported
bool IsReported(RestrictionLevelAndUrl restriction_info) {
  return restriction_info.level == DlpRulesManager::Level::kReport ||
         IsBlocked(restriction_info);
}

// If there is an on going video recording, interrupts it and notifies the user.
void InterruptVideoRecording() {
  if (ChromeCaptureModeDelegate::Get()->InterruptVideoRecordingIfAny())
    ShowDlpVideoCaptureStoppedNotification();
}

}  // namespace

static DlpContentManager* g_dlp_content_manager = nullptr;

// static
DlpContentManager* DlpContentManager::Get() {
  if (!g_dlp_content_manager) {
    g_dlp_content_manager = new DlpContentManager();
    g_dlp_content_manager->Init();
  }
  return g_dlp_content_manager;
}

void DlpContentManager::OnWindowOcclusionChanged(aura::Window* window) {
  // Stop video captures that now might include restricted content.
  CheckRunningVideoCapture();
}

DlpContentRestrictionSet DlpContentManager::GetConfidentialRestrictions(
    content::WebContents* web_contents) const {
  if (!base::Contains(confidential_web_contents_, web_contents))
    return DlpContentRestrictionSet();
  return confidential_web_contents_.at(web_contents);
}

DlpContentRestrictionSet DlpContentManager::GetOnScreenPresentRestrictions()
    const {
  return on_screen_restrictions_;
}

bool DlpContentManager::IsScreenshotRestricted(const ScreenshotArea& area) {
  const ConfidentialContentsInfo info =
      GetAreaConfidentialContentsInfo(area, DlpContentRestriction::kScreenshot);
  MaybeReportEvent(info.restriction_info,
                   DlpRulesManager::Restriction::kScreenshot);
  DlpBooleanHistogram(dlp::kScreenshotBlockedUMA,
                      IsBlocked(info.restriction_info));
  return IsBlocked(info.restriction_info);
}

bool DlpContentManager::IsScreenshotApiRestricted(const ScreenshotArea& area) {
  const ConfidentialContentsInfo info =
      GetAreaConfidentialContentsInfo(area, DlpContentRestriction::kScreenshot);
  MaybeReportEvent(info.restriction_info,
                   DlpRulesManager::Restriction::kScreenshot);
  DlpBooleanHistogram(dlp::kScreenshotBlockedUMA,
                      IsBlocked(info.restriction_info));
  // TODO(crbug.com/1252736): Properly handle WARN for screenshots API
  return IsBlocked(info.restriction_info) || IsWarn(info.restriction_info);
}

void DlpContentManager::CheckScreenshotRestriction(
    const ScreenshotArea& area,
    OnDlpRestrictionChecked callback) {
  const ConfidentialContentsInfo info =
      GetAreaConfidentialContentsInfo(area, DlpContentRestriction::kScreenshot);
  MaybeReportEvent(info.restriction_info,
                   DlpRulesManager::Restriction::kScreenshot);
  DlpBooleanHistogram(dlp::kScreenshotBlockedUMA,
                      IsBlocked(info.restriction_info));
  CheckScreenCaptureRestriction(info, std::move(callback));
}

bool DlpContentManager::IsVideoCaptureRestricted(const ScreenshotArea& area) {
  const ConfidentialContentsInfo info = GetAreaConfidentialContentsInfo(
      area, DlpContentRestriction::kVideoCapture);
  MaybeReportEvent(info.restriction_info,
                   DlpRulesManager::Restriction::kScreenshot);
  DlpBooleanHistogram(dlp::kVideoCaptureBlockedUMA,
                      IsBlocked(info.restriction_info));
  return IsBlocked(info.restriction_info);
}

void DlpContentManager::CheckVideoCaptureRestriction(
    const ScreenshotArea& area,
    OnDlpRestrictionChecked callback) {
  const ConfidentialContentsInfo info = GetAreaConfidentialContentsInfo(
      area, DlpContentRestriction::kVideoCapture);
  MaybeReportEvent(info.restriction_info,
                   DlpRulesManager::Restriction::kScreenshot);
  DlpBooleanHistogram(dlp::kVideoCaptureBlockedUMA,
                      IsBlocked(info.restriction_info));
  CheckScreenCaptureRestriction(info, std::move(callback));
}

bool DlpContentManager::IsPrintingRestricted(
    content::WebContents* web_contents) {
  const RestrictionLevelAndUrl restriction_info =
      GetPrintingRestrictionInfo(web_contents);
  MaybeReportEvent(restriction_info, DlpRulesManager::Restriction::kPrinting);
  DlpBooleanHistogram(dlp::kPrintingBlockedUMA, IsBlocked(restriction_info));
  return IsBlocked(restriction_info);
}

bool DlpContentManager::ShouldWarnBeforePrinting(
    content::WebContents* web_contents) {
  const RestrictionLevelAndUrl restriction_info =
      GetPrintingRestrictionInfo(web_contents);
  return IsWarn(restriction_info);
}

bool DlpContentManager::IsScreenCaptureRestricted(
    const content::DesktopMediaID& media_id) {
  const RestrictionLevelAndUrl restriction_info =
      GetScreenCaptureRestrictionInfo(media_id);
  MaybeReportEvent(restriction_info,
                   DlpRulesManager::Restriction::kScreenShare);
  DlpBooleanHistogram(dlp::kScreenShareBlockedUMA, IsBlocked(restriction_info));
  return IsBlocked(restriction_info);
}

void DlpContentManager::OnVideoCaptureStarted(const ScreenshotArea& area) {
  if (IsVideoCaptureRestricted(area)) {
    InterruptVideoRecording();
    return;
  }
  DCHECK(!running_video_capture_info_.has_value());
  running_video_capture_info_.emplace(area);
}

void DlpContentManager::CheckStoppedVideoCapture(
    OnDlpRestrictionChecked callback) {
  if (!running_video_capture_info_.has_value())
    return;

  // If some confidential content was shown during the recording, but not
  // before, warn the user before saving the file.
  if (!user_allowed_screen_capture_ &&
      !running_video_capture_info_->confidential_contents.IsEmpty()) {
    DlpWarnDialog::ShowDlpVideoCaptureWarningDialog(
        std::move(callback),
        running_video_capture_info_->confidential_contents);
  }
  running_video_capture_info_.reset();
}

bool DlpContentManager::IsCaptureModeInitRestricted() {
  const RestrictionLevelAndUrl screenshot_restriction_info =
      GetOnScreenPresentRestrictions().GetRestrictionLevelAndUrl(
          DlpContentRestriction::kScreenshot);
  const RestrictionLevelAndUrl videocapture_restriction_info =
      GetOnScreenPresentRestrictions().GetRestrictionLevelAndUrl(
          DlpContentRestriction::kVideoCapture);
  const RestrictionLevelAndUrl restriction_info =
      screenshot_restriction_info.level >= videocapture_restriction_info.level
          ? screenshot_restriction_info
          : videocapture_restriction_info;
  MaybeReportEvent(restriction_info, DlpRulesManager::Restriction::kScreenshot);
  DlpBooleanHistogram(dlp::kCaptureModeInitBlockedUMA,
                      IsBlocked(restriction_info));
  return IsBlocked(restriction_info);
}

void DlpContentManager::CheckCaptureModeInitRestriction(
    OnDlpRestrictionChecked callback) {
  const ConfidentialContentsInfo screenshot_info =
      GetConfidentialContentsOnScreen(DlpContentRestriction::kScreenshot);
  const ConfidentialContentsInfo videocapture_info =
      GetConfidentialContentsOnScreen(DlpContentRestriction::kVideoCapture);
  ConfidentialContentsInfo info;

  if (screenshot_info.restriction_info.level >
      videocapture_info.restriction_info.level) {
    info = screenshot_info;
  } else if (screenshot_info.restriction_info.level <
             videocapture_info.restriction_info.level) {
    info = videocapture_info;
  } else {
    // both screenshot and video capture have the same restriction level, so a
    // union is needed
    info = screenshot_info;
    info.confidential_contents.UnionWith(
        videocapture_info.confidential_contents);
  }

  MaybeReportEvent(info.restriction_info,
                   DlpRulesManager::Restriction::kScreenshot);
  DlpBooleanHistogram(dlp::kCaptureModeInitBlockedUMA,
                      IsBlocked(info.restriction_info));
  CheckScreenCaptureRestriction(info, std::move(callback));
}

void DlpContentManager::OnScreenCaptureStarted(
    const std::string& label,
    std::vector<content::DesktopMediaID> screen_capture_ids,
    const std::u16string& application_title,
    content::MediaStreamUI::StateChangeCallback state_change_callback) {
  for (const content::DesktopMediaID& id : screen_capture_ids) {
    ScreenShareInfo screen_share_info(label, id, application_title,
                                      state_change_callback);
    DCHECK(!base::Contains(running_screen_shares_, screen_share_info));
    running_screen_shares_.push_back(screen_share_info);
  }
  CheckRunningScreenShares();
}

void DlpContentManager::OnScreenCaptureStopped(
    const std::string& label,
    const content::DesktopMediaID& media_id) {
  base::EraseIf(
      running_screen_shares_,
      [=](const ScreenShareInfo& screen_share_info) -> bool {
        const bool erased = screen_share_info.label == label &&
                            screen_share_info.media_id == media_id;
        if (erased && screen_share_info.showing_paused_notification)
          HideDlpScreenSharePausedNotification(screen_share_info.label);
        if (erased && screen_share_info.showing_resumed_notification)
          HideDlpScreenShareResumedNotification(screen_share_info.label);
        return erased;
      });
  MaybeUpdateScreenShareNotification();
}

void DlpContentManager::ResetCaptureModeAllowance() {
  user_allowed_screen_capture_ = false;
}

/* static */
void DlpContentManager::SetDlpContentManagerForTesting(
    DlpContentManager* dlp_content_manager) {
  if (g_dlp_content_manager)
    delete g_dlp_content_manager;
  g_dlp_content_manager = dlp_content_manager;
}

/* static */
void DlpContentManager::ResetDlpContentManagerForTesting() {
  g_dlp_content_manager = nullptr;
}

DlpContentManager::ScreenShareInfo::ScreenShareInfo() = default;
DlpContentManager::ScreenShareInfo::ScreenShareInfo(
    const std::string& label,
    const content::DesktopMediaID& media_id,
    const std::u16string& application_title,
    content::MediaStreamUI::StateChangeCallback state_change_callback)
    : label(label),
      media_id(media_id),
      application_title(application_title),
      state_change_callback(state_change_callback) {}
DlpContentManager::ScreenShareInfo::ScreenShareInfo(
    const DlpContentManager::ScreenShareInfo& other) = default;
DlpContentManager::ScreenShareInfo&
DlpContentManager::ScreenShareInfo::operator=(
    const DlpContentManager::ScreenShareInfo& other) = default;
DlpContentManager::ScreenShareInfo::~ScreenShareInfo() = default;

bool DlpContentManager::ScreenShareInfo::operator==(
    const DlpContentManager::ScreenShareInfo& other) const {
  return label == other.label && media_id == other.media_id;
}

bool DlpContentManager::ScreenShareInfo::operator!=(
    const DlpContentManager::ScreenShareInfo& other) const {
  return !(*this == other);
}

DlpContentManager::VideoCaptureInfo::VideoCaptureInfo(
    const ScreenshotArea& area)
    : area(area) {}

DlpContentManager::DlpContentManager() = default;

void DlpContentManager::Init() {
  DlpRulesManager* rules_manager =
      DlpRulesManagerFactory::GetForPrimaryProfile();
  if (rules_manager)
    reporting_manager_ =
        DlpRulesManagerFactory::GetForPrimaryProfile()->GetReportingManager();
}

DlpContentManager::~DlpContentManager() = default;

void DlpContentManager::OnConfidentialityChanged(
    content::WebContents* web_contents,
    const DlpContentRestrictionSet& restriction_set) {
  if (restriction_set.IsEmpty()) {
    RemoveFromConfidential(web_contents);
  } else {
    confidential_web_contents_[web_contents] = restriction_set;
    window_observers_[web_contents] = std::make_unique<DlpWindowObserver>(
        web_contents->GetNativeView(), this);
    if (web_contents->GetVisibility() == content::Visibility::VISIBLE) {
      MaybeChangeOnScreenRestrictions();
    }
  }
  CheckRunningScreenShares();
}

void DlpContentManager::OnWebContentsDestroyed(
    content::WebContents* web_contents) {
  RemoveFromConfidential(web_contents);
}

void DlpContentManager::OnVisibilityChanged(
    content::WebContents* web_contents) {
  MaybeChangeOnScreenRestrictions();
}

void DlpContentManager::RemoveFromConfidential(
    content::WebContents* web_contents) {
  confidential_web_contents_.erase(web_contents);
  window_observers_.erase(web_contents);
  MaybeChangeOnScreenRestrictions();
}

void DlpContentManager::MaybeChangeOnScreenRestrictions() {
  DlpContentRestrictionSet new_restriction_set;
  for (const auto& entry : confidential_web_contents_) {
    if (entry.first->GetVisibility() == content::Visibility::VISIBLE) {
      new_restriction_set.UnionWith(entry.second);
    }
  }
  if (on_screen_restrictions_ != new_restriction_set) {
    DlpContentRestrictionSet added_restrictions =
        new_restriction_set.DifferenceWith(on_screen_restrictions_);
    DlpContentRestrictionSet removed_restrictions =
        on_screen_restrictions_.DifferenceWith(new_restriction_set);
    on_screen_restrictions_ = new_restriction_set;
    OnScreenRestrictionsChanged(added_restrictions, removed_restrictions);
  }
  CheckRunningVideoCapture();
  CheckRunningScreenShares();
}

void DlpContentManager::OnScreenRestrictionsChanged(
    const DlpContentRestrictionSet& added_restrictions,
    const DlpContentRestrictionSet& removed_restrictions) const {
  DCHECK(!(added_restrictions.GetRestrictionLevel(
               DlpContentRestriction::kPrivacyScreen) ==
               DlpRulesManager::Level::kBlock &&
           removed_restrictions.GetRestrictionLevel(
               DlpContentRestriction::kPrivacyScreen) ==
               DlpRulesManager::Level::kBlock));
  ash::PrivacyScreenDlpHelper* privacy_screen_helper =
      ash::PrivacyScreenDlpHelper::Get();

  if (!privacy_screen_helper->IsSupported())
    return;

  const RestrictionLevelAndUrl added_restriction_info =
      added_restrictions.GetRestrictionLevelAndUrl(
          DlpContentRestriction::kPrivacyScreen);

  if (added_restriction_info.level == DlpRulesManager::Level::kBlock) {
    DlpBooleanHistogram(dlp::kPrivacyScreenEnforcedUMA, true);
    privacy_screen_helper->SetEnforced(true);
  }

  if (added_restriction_info.level == DlpRulesManager::Level::kBlock ||
      added_restriction_info.level == DlpRulesManager::Level::kReport) {
    if (reporting_manager_) {
      ReportEvent(added_restriction_info.url,
                  DlpRulesManager::Restriction::kPrivacyScreen,
                  added_restriction_info.level, reporting_manager_);
    }
  }

  if (removed_restrictions.GetRestrictionLevel(
          DlpContentRestriction::kPrivacyScreen) ==
      DlpRulesManager::Level::kBlock) {
    base::ThreadTaskRunnerHandle::Get()->PostDelayedTask(
        FROM_HERE,
        base::BindOnce(&DlpContentManager::MaybeRemovePrivacyScreenEnforcement,
                       base::Unretained(this)),
        kPrivacyScreenOffDelay);
  }
}

void DlpContentManager::MaybeRemovePrivacyScreenEnforcement() const {
  if (GetOnScreenPresentRestrictions().GetRestrictionLevel(
          DlpContentRestriction::kPrivacyScreen) !=
      DlpRulesManager::Level::kBlock) {
    DlpBooleanHistogram(dlp::kPrivacyScreenEnforcedUMA, false);
    ash::PrivacyScreenDlpHelper::Get()->SetEnforced(false);
  }
}

DlpContentManager::ConfidentialContentsInfo
DlpContentManager::GetConfidentialContentsOnScreen(
    DlpContentRestriction restriction) const {
  DlpContentManager::ConfidentialContentsInfo info;
  info.restriction_info =
      GetOnScreenPresentRestrictions().GetRestrictionLevelAndUrl(restriction);
  for (auto& entry : confidential_web_contents_) {
    if (entry.first->GetVisibility() != content::Visibility::VISIBLE)
      continue;
    if (entry.second.GetRestrictionLevel(restriction) ==
        info.restriction_info.level) {
      info.confidential_contents.Add(entry.first);
    }
  }
  return info;
}

DlpContentManager::ConfidentialContentsInfo
DlpContentManager::GetAreaConfidentialContentsInfo(
    const ScreenshotArea& area,
    DlpContentRestriction restriction) const {
  DlpContentManager::ConfidentialContentsInfo info;
  // Fullscreen - restricted if any confidential data is visible.
  if (area.type == ScreenshotType::kAllRootWindows) {
    return GetConfidentialContentsOnScreen(restriction);
  }

  // Window - restricted if the window contains confidential data.
  if (area.type == ScreenshotType::kWindow) {
    DCHECK(area.window);
    for (auto& entry : confidential_web_contents_) {
      aura::Window* web_contents_window = entry.first->GetNativeView();
      if (area.window->Contains(web_contents_window)) {
        if (entry.second.GetRestrictionLevel(restriction) ==
            info.restriction_info.level) {
          info.confidential_contents.Add(entry.first);
        } else if (entry.second.GetRestrictionLevel(restriction) >
                   info.restriction_info.level) {
          info.restriction_info =
              entry.second.GetRestrictionLevelAndUrl(restriction);
          info.confidential_contents.ClearAndAdd(entry.first);
        }
      }
    }
    return info;
  }

  DCHECK_EQ(area.type, ScreenshotType::kPartialWindow);
  DCHECK(area.rect);
  DCHECK(area.window);
  // Partial - restricted if any visible confidential WebContents intersects
  // with the area.
  for (auto& entry : confidential_web_contents_) {
    if (entry.first->GetVisibility() != content::Visibility::VISIBLE ||
        entry.second.GetRestrictionLevel(restriction) ==
            DlpRulesManager::Level::kNotSet) {
      continue;
    }
    aura::Window* web_contents_window = entry.first->GetNativeView();
    if (web_contents_window->GetOcclusionState() ==
        aura::Window::OcclusionState::OCCLUDED) {
      continue;
    }
    aura::Window* root_window = web_contents_window->GetRootWindow();
    // If no root window, then the WebContent shouldn't be visible.
    if (!root_window)
      continue;
    // Not allowing if the area intersects with confidential WebContents,
    // but the intersection doesn't belong to occluded area.
    gfx::Rect intersection(*area.rect);
    aura::Window::ConvertRectToTarget(area.window, root_window, &intersection);
    intersection.Intersect(web_contents_window->GetBoundsInRootWindow());

    if (intersection.IsEmpty() ||
        web_contents_window->occluded_region_in_root().contains(
            gfx::RectToSkIRect(intersection)))
      continue;

    if (entry.second.GetRestrictionLevel(restriction) ==
        info.restriction_info.level) {
      info.confidential_contents.Add(entry.first);
    } else if (entry.second.GetRestrictionLevel(restriction) >
               info.restriction_info.level) {
      info.restriction_info =
          entry.second.GetRestrictionLevelAndUrl(restriction);
      info.confidential_contents.ClearAndAdd(entry.first);
    }
  }
  return info;
}

RestrictionLevelAndUrl DlpContentManager::GetScreenCaptureRestrictionInfo(
    const content::DesktopMediaID& media_id) const {
  if (media_id.type == content::DesktopMediaID::Type::TYPE_SCREEN) {
    return GetOnScreenPresentRestrictions().GetRestrictionLevelAndUrl(
        DlpContentRestriction::kScreenShare);
  }
  if (media_id.type == content::DesktopMediaID::Type::TYPE_WEB_CONTENTS) {
    content::WebContents* web_contents =
        content::WebContents::FromRenderFrameHost(
            content::RenderFrameHost::FromID(
                media_id.web_contents_id.render_process_id,
                media_id.web_contents_id.main_render_frame_id));

    return GetConfidentialRestrictions(web_contents)
        .GetRestrictionLevelAndUrl(DlpContentRestriction::kScreenShare);
  }
  DCHECK_EQ(media_id.type, content::DesktopMediaID::Type::TYPE_WINDOW);
  RestrictionLevelAndUrl restriction_info;
  aura::Window* window = content::DesktopMediaID::GetNativeWindowById(media_id);
  if (window) {
    for (auto& entry : confidential_web_contents_) {
      aura::Window* web_contents_window = entry.first->GetNativeView();
      if (window->Contains(web_contents_window) &&
          entry.second.GetRestrictionLevel(
              DlpContentRestriction::kScreenShare) > restriction_info.level) {
        restriction_info = entry.second.GetRestrictionLevelAndUrl(
            DlpContentRestriction::kScreenShare);
      }
    }
  }
  return restriction_info;
}

void DlpContentManager::CheckRunningVideoCapture() {
  if (!running_video_capture_info_.has_value())
    return;
  const ConfidentialContentsInfo info = GetAreaConfidentialContentsInfo(
      running_video_capture_info_->area, DlpContentRestriction::kVideoCapture);
  MaybeReportEvent(info.restriction_info,
                   DlpRulesManager::Restriction::kScreenshot);
  if (IsBlocked(info.restriction_info)) {
    DlpBooleanHistogram(dlp::kVideoCaptureInterruptedUMA, true);
    InterruptVideoRecording();
    running_video_capture_info_.reset();
  }
  if (IsWarn(info.restriction_info)) {
    // Remember any confidential content observed during the recording, so we
    // can inform the user about it after the recording is finished.
    running_video_capture_info_->confidential_contents.UnionWith(
        info.confidential_contents);
  }
}

void DlpContentManager::MaybeUpdateScreenShareNotification() {
  for (auto& screen_share : running_screen_shares_) {
    // If the screen share was paused and a "paused" notification was shown, but
    // the share is resumed - hide the "paused" notification.
    if (screen_share.showing_paused_notification && screen_share.is_running) {
      HideDlpScreenSharePausedNotification(screen_share.label);
      screen_share.showing_paused_notification = false;
      // If the screen share was paused and later resumed - show a "resumed"
      // notification if not yet shown.
      if (!screen_share.showing_resumed_notification) {
        ShowDlpScreenShareResumedNotification(screen_share.label,
                                              screen_share.application_title);
        screen_share.showing_resumed_notification = true;
      }
    }
    // If the screen share was resumed and "resumed" notification was shown, but
    // the share is not running anymore - hide the "resumed" notification.
    if (screen_share.showing_resumed_notification && !screen_share.is_running) {
      HideDlpScreenShareResumedNotification(screen_share.label);
      screen_share.showing_resumed_notification = false;
    }
    // If the screen share was paused, but no notification is yet shown - show a
    // "paused" notification.
    if (!screen_share.showing_paused_notification && !screen_share.is_running) {
      ShowDlpScreenSharePausedNotification(screen_share.label,
                                           screen_share.application_title);
      screen_share.showing_paused_notification = true;
    }
  }
}

void DlpContentManager::CheckRunningScreenShares() {
  for (auto& screen_share : running_screen_shares_) {
    const RestrictionLevelAndUrl restriction_info =
        GetScreenCaptureRestrictionInfo(screen_share.media_id);
    const bool is_allowed =
        restriction_info.level != DlpRulesManager::Level::kBlock;
    if (is_allowed != screen_share.is_running) {
      if (screen_share.is_running) {
        MaybeReportEvent(restriction_info,
                         DlpRulesManager::Restriction::kScreenShare);
      }
      DlpBooleanHistogram(dlp::kScreenSharePausedOrResumedUMA, !is_allowed);
      screen_share.state_change_callback.Run(
          screen_share.media_id,
          screen_share.is_running ? blink::mojom::MediaStreamStateChange::PAUSE
                                  : blink::mojom::MediaStreamStateChange::PLAY);
      screen_share.is_running = is_allowed;
      MaybeUpdateScreenShareNotification();
    }
  }
}

void DlpContentManager::SetReportingManagerForTesting(
    DlpReportingManager* reporting_manager) {
  reporting_manager_ = reporting_manager;
}

// static
base::TimeDelta DlpContentManager::GetPrivacyScreenOffDelayForTesting() {
  return kPrivacyScreenOffDelay;
}

RestrictionLevelAndUrl DlpContentManager::GetPrintingRestrictionInfo(
    content::WebContents* web_contents) const {
  // If we're viewing the PDF in a MimeHandlerViewGuest, use its embedded
  // WebContents.
  auto* guest_view =
      extensions::MimeHandlerViewGuest::FromWebContents(web_contents);
  web_contents =
      guest_view ? guest_view->embedder_web_contents() : web_contents;

  return GetConfidentialRestrictions(web_contents)
      .GetRestrictionLevelAndUrl(DlpContentRestriction::kPrint);
}

void DlpContentManager::CheckScreenCaptureRestriction(
    ConfidentialContentsInfo info,
    OnDlpRestrictionChecked callback) {
  if (IsBlocked(info.restriction_info)) {
    ShowDlpScreenCaptureDisabledNotification();
    std::move(callback).Run(false);
  } else if (IsWarn(info.restriction_info)) {
    if (user_allowed_screen_capture_) {
      std::move(callback).Run(true);
    } else {
      DlpWarnDialog::ShowDlpScreenCaptureWarningDialog(
          base::BindOnce(&DlpContentManager::OnScreenCaptureReply,
                         base::Unretained(this), std::move(callback)),
          info.confidential_contents);
    }
  } else {
    std::move(callback).Run(true);
  }
}

void DlpContentManager::MaybeReportEvent(
    const RestrictionLevelAndUrl& restriction_info,
    DlpRulesManager::Restriction restriction) {
  // TODO(crbug.com/1247190): Add reporting and metrics for screenshot WARN
  // TODO(crbug.com/1227700): Add reporting and metrics for printing WARN
  if (IsReported(restriction_info) && reporting_manager_) {
    ReportEvent(restriction_info.url, restriction, restriction_info.level,
                reporting_manager_);
  }
}

void DlpContentManager::OnScreenCaptureReply(OnDlpRestrictionChecked callback,
                                             bool should_proceed) {
  user_allowed_screen_capture_ = should_proceed;
  std::move(callback).Run(should_proceed);
}

// ScopedDlpContentManagerForTesting
ScopedDlpContentManagerForTesting::ScopedDlpContentManagerForTesting(
    DlpContentManager* test_dlp_content_manager) {
  DlpContentManager::SetDlpContentManagerForTesting(test_dlp_content_manager);
}

ScopedDlpContentManagerForTesting::~ScopedDlpContentManagerForTesting() {
  DlpContentManager::ResetDlpContentManagerForTesting();
}

}  // namespace policy
