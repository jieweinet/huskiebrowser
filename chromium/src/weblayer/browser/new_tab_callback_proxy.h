// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBLAYER_BROWSER_NEW_TAB_CALLBACK_PROXY_H_
#define WEBLAYER_BROWSER_NEW_TAB_CALLBACK_PROXY_H_

#include <jni.h>

#include "base/android/scoped_java_ref.h"
#include "base/macros.h"
#include "weblayer/public/new_tab_delegate.h"

namespace weblayer {

class TabImpl;

// NewTabCallbackProxy forwards all NewTabDelegate functions to the Java
// side. There is one NewTabCallbackProxy per Tab.
class NewTabCallbackProxy : public NewTabDelegate {
 public:
  NewTabCallbackProxy(JNIEnv* env, jobject obj, TabImpl* tab);

  NewTabCallbackProxy(const NewTabCallbackProxy&) = delete;
  NewTabCallbackProxy& operator=(const NewTabCallbackProxy&) = delete;

  ~NewTabCallbackProxy() override;

  // NewTabDelegate:
  void OnNewTab(Tab* tab, NewTabType type) override;

 private:
  TabImpl* tab_;
  base::android::ScopedJavaGlobalRef<jobject> java_impl_;
};

}  // namespace weblayer

#endif  // WEBLAYER_BROWSER_NEW_TAB_CALLBACK_PROXY_H_
