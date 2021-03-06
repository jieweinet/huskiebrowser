// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_SERVICES_LIBASSISTANT_GRPC_UTILS_TIMER_UTILS_H_
#define CHROMEOS_SERVICES_LIBASSISTANT_GRPC_UTILS_TIMER_UTILS_H_

#include "chromeos/services/libassistant/public/cpp/assistant_timer.h"
#include "libassistant/shared/internal_api/alarm_timer_manager.h"

namespace assistant {
namespace api {
class OnAlarmTimerEventRequest;

namespace params {
enum class TimerStatus;
class Timer;
}  // namespace params

}  // namespace api
}  // namespace assistant

namespace chromeos {
namespace libassistant {

::assistant::api::OnAlarmTimerEventRequest
CreateOnAlarmTimerEventRequestProtoForV1(
    const std::vector<chromeos::assistant::AssistantTimer>& all_curr_timers);

std::vector<assistant::AssistantTimer> ConstructAssistantTimersFromProto(
    const ::assistant::api::OnAlarmTimerEventRequest& input);

void ConvertAssistantTimerToProtoTimer(const assistant::AssistantTimer& input,
                                       ::assistant::api::params::Timer* output);

void ConvertProtoTimerToAssistantTimer(
    const ::assistant::api::params::Timer& input,
    chromeos::assistant::AssistantTimer* output);

// Used both in |AssistantClientV1| and |FakeAssistantClient|.
std::vector<chromeos::assistant::AssistantTimer> GetAllCurrentTimersFromEvents(
    const std::vector<assistant_client::AlarmTimerManager::Event>& events);

}  // namespace libassistant
}  // namespace chromeos

#endif  // CHROMEOS_SERVICES_LIBASSISTANT_GRPC_UTILS_TIMER_UTILS_H_
