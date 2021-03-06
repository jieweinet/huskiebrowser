// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_SEND_TAB_TO_SELF_SEND_TAB_TO_SELF_SUB_MENU_MODEL_H_
#define CHROME_BROWSER_UI_SEND_TAB_TO_SELF_SEND_TAB_TO_SELF_SUB_MENU_MODEL_H_

// This is the secondary menu model of send tab to self context menu item. This
// menu contains the imformation of valid devices, getting form
// SendTabToSelfModel. Every item of this menu is a valid device.

#include <vector>

#include "base/macros.h"
#include "chrome/browser/send_tab_to_self/send_tab_to_self_desktop_util.h"
#include "content/public/browser/web_contents_observer.h"
#include "ui/base/models/simple_menu_model.h"
#include "url/gurl.h"

class Profile;

namespace content {
class WebContents;
}

namespace send_tab_to_self {

class SendTabToSelfSubMenuModel : public ui::SimpleMenuModel,
                                  public ui::SimpleMenuModel::Delegate,
                                  public content::WebContentsObserver {
 public:
  static const int kMinCommandId = 2000;
  static const int kMaxCommandId = 2020;

  struct ValidDeviceItem;

  SendTabToSelfSubMenuModel(content::WebContents* tab,
                            SendTabToSelfMenuType menu_type);
  SendTabToSelfSubMenuModel(content::WebContents* tab,
                            SendTabToSelfMenuType menu_type,
                            const GURL& link_url);

  SendTabToSelfSubMenuModel(const SendTabToSelfSubMenuModel&) = delete;
  SendTabToSelfSubMenuModel& operator=(const SendTabToSelfSubMenuModel&) =
      delete;

  ~SendTabToSelfSubMenuModel() override;

  // Overridden from ui::SimpleMenuModel::Delegate:
  bool IsCommandIdEnabled(int command_id) const override;
  void ExecuteCommand(int command_id, int event_flags) override;

  // Overridden from content::WebContentsObserver:
  void WebContentsDestroyed() override;

 private:
  void Build(Profile* profile);
  void BuildDeviceItem(const std::string& device_name,
                       const std::string& guid,
                       int index);

  // Injected on construction and set to null if the WebContents is being
  // destroyed.
  content::WebContents* tab_;
  const SendTabToSelfMenuType menu_type_;
  const GURL link_url_;
  std::vector<ValidDeviceItem> valid_device_items_;
};

}  //  namespace send_tab_to_self

#endif  // CHROME_BROWSER_UI_SEND_TAB_TO_SELF_SEND_TAB_TO_SELF_SUB_MENU_MODEL_H_
