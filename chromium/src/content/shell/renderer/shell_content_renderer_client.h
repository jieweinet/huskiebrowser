// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_SHELL_RENDERER_SHELL_CONTENT_RENDERER_CLIENT_H_
#define CONTENT_SHELL_RENDERER_SHELL_CONTENT_RENDERER_CLIENT_H_

#include <memory>
#include <string>

#include "base/compiler_specific.h"
#include "build/build_config.h"
#include "content/public/renderer/content_renderer_client.h"
#include "media/mojo/buildflags.h"

namespace web_cache {
class WebCacheImpl;
}

namespace content {

class ShellContentRendererClient : public ContentRendererClient {
 public:
  ShellContentRendererClient();
  ~ShellContentRendererClient() override;

  // ContentRendererClient implementation.
  void RenderThreadStarted() override;
  void ExposeInterfacesToBrowser(mojo::BinderMap* binders) override;
  void RenderFrameCreated(RenderFrame* render_frame) override;
  void PrepareErrorPage(RenderFrame* render_frame,
                        const blink::WebURLError& error,
                        const std::string& http_method,
                        std::string* error_html) override;
  void PrepareErrorPageForHttpStatusError(content::RenderFrame* render_frame,
                                          const blink::WebURLError& error,
                                          const std::string& http_method,
                                          int http_status,
                                          std::string* error_html) override;

  void DidInitializeWorkerContextOnWorkerThread(
      v8::Local<v8::Context> context) override;

#if BUILDFLAG(ENABLE_MOJO_CDM)
  void AddSupportedKeySystems(
      std::vector<std::unique_ptr<media::KeySystemProperties>>* key_systems)
      override;
#endif

 private:
  std::unique_ptr<web_cache::WebCacheImpl> web_cache_impl_;
};

}  // namespace content

#endif  // CONTENT_SHELL_RENDERER_SHELL_CONTENT_RENDERER_CLIENT_H_
