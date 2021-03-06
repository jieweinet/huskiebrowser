// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ASH_POLICY_SCHEDULED_TASK_HANDLER_SCHEDULED_TASK_EXECUTOR_H_
#define CHROME_BROWSER_ASH_POLICY_SCHEDULED_TASK_HANDLER_SCHEDULED_TASK_EXECUTOR_H_

#include "base/callback_forward.h"
#include "base/time/time.h"
#include "chromeos/dbus/power/native_timer.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "third_party/icu/source/i18n/unicode/calendar.h"

namespace policy {

using TimerCallback = base::OnceClosure;

// This class is used to start the native timer for a scheduled task.
class ScheduledTaskExecutor {
 public:
  // Frequency at which the task should occur.
  enum class Frequency {
    kDaily,
    kWeekly,
    kMonthly,
  };

  // Holds the data associated with the current scheduled task policy.
  struct ScheduledTaskData {
    ScheduledTaskData();
    ScheduledTaskData(const ScheduledTaskData&);
    ~ScheduledTaskData();

    // Corresponds to UCAL_HOUR_OF_DAY in icu::Calendar.
    int hour;

    // Corresponds to UCAL_MINUTE in icu::Calendar.
    int minute;

    Frequency frequency;

    // Only set when frequency is |kWeekly|. Corresponds to UCAL_DAY_OF_WEEK in
    // icu::Calendar. Values between 1 (SUNDAY) to 7 (SATURDAY).
    absl::optional<UCalendarDaysOfWeek> day_of_week;

    // Only set when frequency is |kMonthly|. Corresponds to UCAL_DAY_OF_MONTH
    // in icu::Calendar i.e. values between 1 to 31.
    absl::optional<int> day_of_month;

    // Absolute time ticks when the next scheduled task (i.e. |UpdateCheck|)
    // will happen.
    base::TimeTicks next_scheduled_task_time_ticks;
  };

  virtual ~ScheduledTaskExecutor() = default;

  // Starts the native timer. Runs result_cb with false result if there
  // was an error while calculating the next_scheduled_task_time_ticks,
  // otherwise starts NativeTimer.
  virtual void Start(ScheduledTaskData* scheduled_task_data,
                     chromeos::OnStartNativeTimerCallback result_cb,
                     TimerCallback timer_expired_cb) = 0;

  // Resets the native timer.
  virtual void Reset() = 0;
};

}  // namespace policy

#endif  // CHROME_BROWSER_ASH_POLICY_SCHEDULED_TASK_HANDLER_SCHEDULED_TASK_EXECUTOR_H_
