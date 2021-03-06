// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/modules/idle/idle_detector.h"

#include <utility>

#include "base/time/time.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "third_party/blink/public/mojom/idle/idle_manager.mojom-blink.h"
#include "third_party/blink/public/mojom/permissions_policy/permissions_policy.mojom-blink.h"
#include "third_party/blink/renderer/bindings/modules/v8/v8_idle_options.h"
#include "third_party/blink/renderer/core/dom/abort_signal.h"
#include "third_party/blink/renderer/core/dom/dom_exception.h"
#include "third_party/blink/renderer/core/execution_context/execution_context.h"
#include "third_party/blink/renderer/core/execution_context/security_context.h"
#include "third_party/blink/renderer/modules/idle/idle_manager.h"
#include "third_party/blink/renderer/platform/bindings/exception_state.h"
#include "third_party/blink/renderer/platform/bindings/script_state.h"
#include "third_party/blink/renderer/platform/heap/heap.h"
#include "third_party/blink/renderer/platform/heap/persistent.h"
namespace blink {

namespace {

using mojom::blink::IdleManagerError;

const char kAbortMessage[] = "Idle detection aborted.";
const char kFeaturePolicyBlocked[] =
    "Access to the feature \"idle-detection\" is disallowed by permissions "
    "policy.";

constexpr base::TimeDelta kMinimumThreshold = base::Seconds(60);

}  // namespace

IdleDetector* IdleDetector::Create(ScriptState* script_state) {
  return MakeGarbageCollected<IdleDetector>(
      ExecutionContext::From(script_state));
}

IdleDetector::IdleDetector(ExecutionContext* context)
    : ExecutionContextClient(context), receiver_(this, context) {}

IdleDetector::~IdleDetector() = default;

const AtomicString& IdleDetector::InterfaceName() const {
  return event_target_names::kIdleDetector;
}

ExecutionContext* IdleDetector::GetExecutionContext() const {
  return ExecutionContextClient::GetExecutionContext();
}

bool IdleDetector::HasPendingActivity() const {
  // This object should be considered active as long as there are registered
  // event listeners.
  return GetExecutionContext() && HasEventListeners();
}

String IdleDetector::userState() const {
  if (!state_)
    return String();

  switch (state_->user) {
    case mojom::blink::UserIdleState::kActive:
      return "active";
    case mojom::blink::UserIdleState::kIdle:
      return "idle";
  }
}

String IdleDetector::screenState() const {
  if (!state_)
    return String();

  switch (state_->screen) {
    case mojom::blink::ScreenIdleState::kLocked:
      return "locked";
    case mojom::blink::ScreenIdleState::kUnlocked:
      return "unlocked";
  }
}

// static
ScriptPromise IdleDetector::requestPermission(ScriptState* script_state,
                                              ExceptionState& exception_state) {
  if (!script_state->ContextIsValid())
    return ScriptPromise();

  auto* context = ExecutionContext::From(script_state);
  return IdleManager::From(context)->RequestPermission(script_state,
                                                       exception_state);
}

ScriptPromise IdleDetector::start(ScriptState* script_state,
                                  const IdleOptions* options,
                                  ExceptionState& exception_state) {
  ExecutionContext* context = ExecutionContext::From(script_state);
  DCHECK(context->IsContextThread());

  if (!context->IsFeatureEnabled(
          mojom::blink::PermissionsPolicyFeature::kIdleDetection,
          ReportOptions::kReportOnFailure)) {
    exception_state.ThrowSecurityError(kFeaturePolicyBlocked);
    return ScriptPromise();
  }

  if (receiver_.is_bound()) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      "Idle detector is already started.");
    return ScriptPromise();
  }

  if (options->hasThreshold()) {
    auto threshold = base::Milliseconds(options->threshold());
    if (threshold < kMinimumThreshold) {
      exception_state.ThrowTypeError("Minimum threshold is 1 minute.");
      return ScriptPromise();
    }
    threshold_ = threshold;
  }

  if (options->hasSignal()) {
    signal_ = options->signal();
    signal_->AddAlgorithm(WTF::Bind(&IdleDetector::Abort,
                                    WrapWeakPersistent(this),
                                    WrapWeakPersistent(signal_.Get())));
  }

  if (signal_ && signal_->aborted()) {
    exception_state.ThrowDOMException(DOMExceptionCode::kAbortError,
                                      kAbortMessage);
    return ScriptPromise();
  }

  // See https://bit.ly/2S0zRAS for task types.
  scoped_refptr<base::SingleThreadTaskRunner> task_runner =
      context->GetTaskRunner(TaskType::kMiscPlatformAPI);

  mojo::PendingRemote<mojom::blink::IdleMonitor> remote;
  receiver_.Bind(remote.InitWithNewPipeAndPassReceiver(), task_runner);
  receiver_.set_disconnect_handler(WTF::Bind(
      &IdleDetector::OnMonitorDisconnected, WrapWeakPersistent(this)));

  auto* resolver = MakeGarbageCollected<ScriptPromiseResolver>(script_state);
  ScriptPromise promise = resolver->Promise();
  IdleManager::From(context)->AddMonitor(
      threshold_, std::move(remote),
      WTF::Bind(&IdleDetector::OnAddMonitor, WrapWeakPersistent(this),
                WrapPersistent(resolver)));
  return promise;
}

void IdleDetector::Abort(AbortSignal* signal) {
  // There is no RemoveAlgorithm() method on AbortSignal so compare the signal
  // bound to this callback to the one last passed to start().
  if (signal_ != signal)
    return;

  if (resolver_) {
    resolver_->Reject(MakeGarbageCollected<DOMException>(
        DOMExceptionCode::kAbortError, kAbortMessage));
    resolver_ = nullptr;
  }

  receiver_.reset();
}

void IdleDetector::OnMonitorDisconnected() {
  if (resolver_) {
    resolver_->Reject(MakeGarbageCollected<DOMException>(
        DOMExceptionCode::kNotSupportedError, "Idle detection not available."));
    resolver_ = nullptr;
  }

  receiver_.reset();
}

void IdleDetector::OnAddMonitor(ScriptPromiseResolver* resolver,
                                IdleManagerError error,
                                mojom::blink::IdleStatePtr state) {
  switch (error) {
    case IdleManagerError::kPermissionDisabled:
      resolver->Reject(MakeGarbageCollected<DOMException>(
          DOMExceptionCode::kNotAllowedError,
          "Idle detection permission denied"));
      break;
    case IdleManagerError::kSuccess:
      DCHECK(state);
      resolver->Resolve();
      Update(std::move(state));
      break;
  }

  resolver_ = nullptr;
}

void IdleDetector::Update(mojom::blink::IdleStatePtr state) {
  DCHECK(receiver_.is_bound());
  if (!GetExecutionContext() || GetExecutionContext()->IsContextDestroyed())
    return;

  if (state_ && state->Equals(*state_))
    return;

  state_ = std::move(state);

  DispatchEvent(*Event::Create(event_type_names::kChange));
}

void IdleDetector::Trace(Visitor* visitor) const {
  visitor->Trace(signal_);
  visitor->Trace(resolver_);
  visitor->Trace(receiver_);
  EventTargetWithInlineData::Trace(visitor);
  ExecutionContextClient::Trace(visitor);
  ActiveScriptWrappable::Trace(visitor);
}

}  // namespace blink
