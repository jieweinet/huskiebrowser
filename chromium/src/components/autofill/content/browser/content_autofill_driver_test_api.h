// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_AUTOFILL_CONTENT_BROWSER_CONTENT_AUTOFILL_DRIVER_TEST_API_H_
#define COMPONENTS_AUTOFILL_CONTENT_BROWSER_CONTENT_AUTOFILL_DRIVER_TEST_API_H_

#include "components/autofill/content/browser/content_autofill_driver.h"

namespace autofill {

class ContentAutofillRouter;

// Exposes some testing operations for ContentAutofillDriver.
class ContentAutofillDriverTestApi {
 public:
  explicit ContentAutofillDriverTestApi(ContentAutofillDriver* driver)
      : driver_(driver) {
    DCHECK(driver_);
  }

  ContentAutofillRouter& autofill_router() {
    return *driver_->autofill_router_;
  }

  void SetFrameAndFormMetaData(FormData& form, FormFieldData* field) const {
    driver_->SetFrameAndFormMetaData(form, field);
  }

  FormData GetFormWithFrameAndFormMetaData(const FormData& form) const {
    return driver_->GetFormWithFrameAndFormMetaData(form);
  }

 private:
  // Non-null pointer to wrapped ContentAutofillDriver.
  ContentAutofillDriver* driver_;
};

}  // namespace autofill

#endif  // COMPONENTS_AUTOFILL_CONTENT_BROWSER_CONTENT_AUTOFILL_DRIVER_TEST_API_H_
