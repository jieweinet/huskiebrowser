// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/renderer/web_ui_extension_data.h"

#include "content/public/renderer/render_frame.h"
#include "mojo/public/cpp/bindings/self_owned_associated_receiver.h"
#include "third_party/blink/public/common/browser_interface_broker_proxy.h"

namespace content {

void WebUIExtensionData::Create(
    RenderFrame* render_frame,
    mojo::PendingAssociatedReceiver<mojom::WebUI> receiver,
    mojo::PendingAssociatedRemote<mojom::WebUIHost> remote) {
  mojo::MakeSelfOwnedAssociatedReceiver(
      std::make_unique<WebUIExtensionData>(render_frame, std::move(remote)),
      std::move(receiver));
}

WebUIExtensionData::WebUIExtensionData(
    RenderFrame* render_frame,
    mojo::PendingAssociatedRemote<mojom::WebUIHost> remote)
    : RenderFrameObserver(render_frame),
      RenderFrameObserverTracker<WebUIExtensionData>(render_frame),
      remote_(std::move(remote)) {}

WebUIExtensionData::~WebUIExtensionData() = default;

std::string WebUIExtensionData::GetValue(const std::string& key) const {
  auto it = variable_map_.find(key);
  if (it == variable_map_.end())
    return std::string();
  return it->second;
}

void WebUIExtensionData::SendMessage(const std::string& message,
                                     std::unique_ptr<base::ListValue> args) {
  remote_->Send(message, std::move(*args));
}

void WebUIExtensionData::SetProperty(const std::string& name,
                                     const std::string& value) {
  variable_map_[name] = value;
}

}  // namespace content
