// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PDF_PDFIUM_PDFIUM_UNSUPPORTED_FEATURES_H_
#define PDF_PDFIUM_PDFIUM_UNSUPPORTED_FEATURES_H_

namespace chrome_pdf {

class PDFiumEngine;

void InitializeUnsupportedFeaturesHandler();

// Create a local variable of this when calling PDFium functions which can call
// our global callback when an unsupported feature is reached.
class ScopedUnsupportedFeature {
 public:
  // For use by PDFiumEngineExports when there is no PDFiumEngine instance.
  enum NoEngine { kNoEngine };

  explicit ScopedUnsupportedFeature(PDFiumEngine* engine);
  explicit ScopedUnsupportedFeature(NoEngine no_engine);
  ScopedUnsupportedFeature(const ScopedUnsupportedFeature&) = delete;
  ScopedUnsupportedFeature& operator=(const ScopedUnsupportedFeature&) = delete;
  ~ScopedUnsupportedFeature();

 private:
  const bool saved_engine_available_;
  PDFiumEngine* const saved_engine_;
};

}  // namespace chrome_pdf

#endif  // PDF_PDFIUM_PDFIUM_UNSUPPORTED_FEATURES_H_
