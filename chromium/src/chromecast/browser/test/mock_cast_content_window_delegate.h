// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMECAST_BROWSER_TEST_MOCK_CAST_CONTENT_WINDOW_DELEGATE_H_
#define CHROMECAST_BROWSER_TEST_MOCK_CAST_CONTENT_WINDOW_DELEGATE_H_

#include "base/memory/weak_ptr.h"
#include "chromecast/browser/cast_content_window.h"
#include "testing/gmock/include/gmock/gmock.h"

namespace chromecast {

class MockCastContentWindowDelegate
    : public base::SupportsWeakPtr<MockCastContentWindowDelegate>,
      public CastContentWindow::Delegate {
 public:
  MockCastContentWindowDelegate();
  ~MockCastContentWindowDelegate() override;

  MOCK_METHOD(void, OnWindowDestroyed, (), (override));
  MOCK_METHOD(void, OnVisibilityChange, (VisibilityType), (override));
};

}  // namespace chromecast

#endif  // CHROMECAST_BROWSER_TEST_MOCK_CAST_CONTENT_WINDOW_DELEGATE_H_
