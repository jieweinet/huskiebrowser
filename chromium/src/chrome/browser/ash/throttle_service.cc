// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ash/throttle_service.h"

#include <utility>

#include "base/bind.h"
#include "base/logging.h"

namespace chromeos {

ThrottleService::ThrottleService(content::BrowserContext* context)
    : context_(context) {}

ThrottleService::~ThrottleService() = default;

void ThrottleService::AddServiceObserver(ServiceObserver* observer) {
  service_observers_.AddObserver(observer);
}

void ThrottleService::RemoveServiceObserver(ServiceObserver* observer) {
  service_observers_.RemoveObserver(observer);
}

void ThrottleService::NotifyObserverStateChangedForTesting() {
  OnObserverStateChanged();
}

void ThrottleService::SetObserversForTesting(
    std::vector<std::unique_ptr<ThrottleObserver>> observers) {
  StopObservers();
  observers_ = std::move(observers);
  StartObservers();
}

void ThrottleService::set_level_for_testing(
    ThrottleObserver::PriorityLevel level) {
  SetLevel(level);
}

void ThrottleService::AddObserver(std::unique_ptr<ThrottleObserver> observer) {
  observers_.push_back(std::move(observer));
}

void ThrottleService::StartObservers() {
  auto callback = base::BindRepeating(&ThrottleService::OnObserverStateChanged,
                                      weak_ptr_factory_.GetWeakPtr());
  for (auto& observer : observers_)
    observer->StartObserving(context_, callback);
}

void ThrottleService::StopObservers() {
  for (auto& observer : observers_)
    observer->StopObserving();
}

void ThrottleService::SetEnforced(ThrottleObserver::PriorityLevel level) {
  if (enforced_level_ == level)
    return;
  enforced_level_ = level;
  OnObserverStateChanged();
}

void ThrottleService::OnObserverStateChanged() {
  ThrottleObserver::PriorityLevel max_level =
      ThrottleObserver::PriorityLevel::LOW;
  ThrottleObserver* effective_observer = nullptr;

  if (enforced_level_ == ThrottleObserver::PriorityLevel::UNKNOWN) {
    // Auto mode
    for (auto& observer : observers_) {
      if (!observer->active())
        continue;
      DVLOG(1) << "Active Throttle Observer: "
               << observer->GetDebugDescription();
      if (observer->level() >= max_level) {
        max_level = observer->level();
        effective_observer = observer.get();
      }
    }
  } else {
    // Enforced mode
    max_level = enforced_level_;
    DVLOG(1) << "Throttle is enforced to " << enforced_level_;
  }

  if (effective_observer != last_effective_observer_) {
    // If there is a new effective observer, record the duration that the last
    // effective observer was active.
    if (last_effective_observer_) {
      RecordCpuRestrictionDisabledUMA(
          last_effective_observer_->name(),
          base::TimeTicks::Now() - last_throttle_transition_);
    }
    last_throttle_transition_ = base::TimeTicks::Now();
    last_effective_observer_ = effective_observer;
  }

  SetLevel(max_level);
}

void ThrottleService::SetLevel(ThrottleObserver::PriorityLevel level) {
  if (level_ == level)
    return;
  level_ = level;
  ThrottleInstance(level);

  for (auto& observer : service_observers_)
    observer.OnThrottle(level);
}

}  // namespace chromeos
