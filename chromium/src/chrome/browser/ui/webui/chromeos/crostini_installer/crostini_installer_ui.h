// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_CHROMEOS_CROSTINI_INSTALLER_CROSTINI_INSTALLER_UI_H_
#define CHROME_BROWSER_UI_WEBUI_CHROMEOS_CROSTINI_INSTALLER_CROSTINI_INSTALLER_UI_H_

#include "base/macros.h"
#include "chrome/browser/ui/webui/chromeos/crostini_installer/crostini_installer.mojom.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "ui/web_dialogs/web_dialog_ui.h"

namespace chromeos {

class CrostiniInstallerPageHandler;

// The WebUI for chrome://crostini-installer
class CrostiniInstallerUI
    : public ui::MojoWebDialogUI,
      public chromeos::crostini_installer::mojom::PageHandlerFactory {
 public:
  explicit CrostiniInstallerUI(content::WebUI* web_ui);

  CrostiniInstallerUI(const CrostiniInstallerUI&) = delete;
  CrostiniInstallerUI& operator=(const CrostiniInstallerUI&) = delete;

  ~CrostiniInstallerUI() override;

  // Send a close request to the web page. Return true if the page is already
  // closed.
  bool RequestClosePage();

  void ClickInstallForTesting();

  // Instantiates implementor of the mojom::PageHandlerFactory
  // mojo interface passing the pending receiver that will be internally bound.
  void BindInterface(mojo::PendingReceiver<
                     chromeos::crostini_installer::mojom::PageHandlerFactory>
                         pending_receiver);

 private:
  // chromeos::crostini_installer::mojom::PageHandlerFactory:
  void CreatePageHandler(
      mojo::PendingRemote<chromeos::crostini_installer::mojom::Page>
          pending_page,
      mojo::PendingReceiver<chromeos::crostini_installer::mojom::PageHandler>
          pending_page_handler) override;

  void OnPageClosed();

  std::unique_ptr<CrostiniInstallerPageHandler> page_handler_;
  mojo::Receiver<chromeos::crostini_installer::mojom::PageHandlerFactory>
      page_factory_receiver_{this};
  bool page_closed_ = false;

  WEB_UI_CONTROLLER_TYPE_DECL();
};

}  // namespace chromeos

#endif  // CHROME_BROWSER_UI_WEBUI_CHROMEOS_CROSTINI_INSTALLER_CROSTINI_INSTALLER_UI_H_
