// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromeos/components/projector_app/trusted_projector_ui.h"

#include "ash/public/cpp/projector/projector_annotator_controller.h"
#include "chromeos/components/projector_app/annotator_message_handler.h"
#include "chromeos/components/projector_app/projector_message_handler.h"
#include "chromeos/components/projector_app/public/cpp/projector_app_constants.h"
#include "chromeos/grit/chromeos_projector_app_trusted_resources.h"
#include "chromeos/grit/chromeos_projector_app_trusted_resources_map.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"
#include "content/public/common/url_constants.h"
#include "services/network/public/mojom/content_security_policy.mojom.h"
#include "url/gurl.h"

namespace chromeos {

namespace {

content::WebUIDataSource* CreateProjectorHTMLSource() {
  content::WebUIDataSource* source =
      content::WebUIDataSource::Create(chromeos::kChromeUIProjectorAppHost);

  source->AddResourcePaths(
      base::make_span(kChromeosProjectorAppTrustedResources,
                      kChromeosProjectorAppTrustedResourcesSize));

  source->AddResourcePath("",
                          IDR_CHROMEOS_PROJECTOR_APP_TRUSTED_APP_EMBEDDER_HTML);

  std::string csp =
      std::string("frame-src ") + kChromeUIUntrustedProjectorAppUrl + ";";

  source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::FrameSrc, csp);

  return source;
}

}  // namespace

TrustedProjectorUI::TrustedProjectorUI(content::WebUI* web_ui, const GURL& url)
    : MojoBubbleWebUIController(web_ui, /*enable_chrome_send=*/true) {
  auto* browser_context = web_ui->GetWebContents()->GetBrowserContext();
  content::WebUIDataSource::Add(browser_context, CreateProjectorHTMLSource());

  // The selfie cam doesn't have any dependencies on WebUIMessageHandlers;
  // it also doesn't embed chrome-untrusted:// resources. Therefore, return
  // early.
  if (url == GURL(kChromeUITrustedProjectorSelfieCamUrl))
    return;

  // The Annotator and Projector SWA embed contents in a sandboxed
  // chrome-untrusted:// iframe.
  web_ui->AddRequestableScheme(content::kChromeUIUntrustedScheme);

  // RecordingOverlayViewImpl is responsible for creating the
  // AnnotatorMessageHandler via a helper function in ProjectorClientImpl. Do
  // nothing here.
  if (url == GURL(kChromeUITrustedAnnotatorUrl))
    return;

  // The requested WebUI is hosting the Projector SWA.
  web_ui->AddMessageHandler(std::make_unique<ProjectorMessageHandler>());
}

TrustedProjectorUI::~TrustedProjectorUI() = default;

WEB_UI_CONTROLLER_TYPE_IMPL(TrustedProjectorUI)

}  // namespace chromeos
