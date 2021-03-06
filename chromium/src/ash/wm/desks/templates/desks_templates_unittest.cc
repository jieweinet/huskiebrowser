// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "ash/constants/ash_features.h"
#include "ash/constants/ash_pref_names.h"
#include "ash/public/cpp/desk_template.h"
#include "ash/session/session_controller_impl.h"
#include "ash/shell.h"
#include "ash/wm/desks/desks_bar_view.h"
#include "ash/wm/desks/expanded_desks_bar_button.h"
#include "ash/wm/desks/templates/desks_templates_delete_button.h"
#include "ash/wm/desks/templates/desks_templates_dialog_controller.h"
#include "ash/wm/desks/templates/desks_templates_grid_view.h"
#include "ash/wm/desks/templates/desks_templates_item_view.h"
#include "ash/wm/desks/templates/desks_templates_presenter.h"
#include "ash/wm/desks/zero_state_button.h"
#include "ash/wm/overview/overview_grid.h"
#include "ash/wm/overview/overview_highlight_controller.h"
#include "ash/wm/overview/overview_session.h"
#include "ash/wm/overview/overview_test_base.h"
#include "ash/wm/overview/overview_test_util.h"
#include "base/guid.h"
#include "base/strings/utf_string_conversions.h"
#include "base/test/bind.h"
#include "base/time/time.h"
#include "components/prefs/pref_service.h"
#include "ui/aura/window.h"
#include "ui/compositor/layer.h"
#include "ui/views/controls/label.h"
#include "ui/views/controls/textfield/textfield.h"

namespace ash {

// Wrapper for DesksTemplatesPresenter that exposes internal state to test
// functions.
class DesksTemplatesPresenterTestApi {
 public:
  explicit DesksTemplatesPresenterTestApi(DesksTemplatesPresenter* presenter)
      : presenter_(presenter) {
    DCHECK(presenter_);
  }
  DesksTemplatesPresenterTestApi(const DesksTemplatesPresenterTestApi&) =
      delete;
  DesksTemplatesPresenterTestApi& operator=(
      const DesksTemplatesPresenterTestApi&) = delete;
  ~DesksTemplatesPresenterTestApi() = default;

  void SetOnUpdateUiClosure(base::OnceClosure closure) {
    DCHECK(!presenter_->on_update_ui_closure_for_testing_);
    presenter_->on_update_ui_closure_for_testing_ = std::move(closure);
  }

 private:
  DesksTemplatesPresenter* const presenter_;
};

// Wrapper for DesksTemplatesGridView that exposes internal state to test
// functions.
class DesksTemplatesGridViewTestApi {
 public:
  explicit DesksTemplatesGridViewTestApi(
      const DesksTemplatesGridView* grid_view)
      : grid_view_(grid_view) {
    DCHECK(grid_view_);
  }
  DesksTemplatesGridViewTestApi(const DesksTemplatesGridViewTestApi&) = delete;
  DesksTemplatesGridViewTestApi& operator=(
      const DesksTemplatesGridViewTestApi&) = delete;
  ~DesksTemplatesGridViewTestApi() = default;

  const std::vector<DesksTemplatesItemView*>& grid_items() const {
    return grid_view_->grid_items_;
  }

 private:
  const DesksTemplatesGridView* grid_view_;
};

// Wrapper for DesksTemplatesItemView that exposes internal state to test
// functions.
class DesksTemplatesItemViewTestApi {
 public:
  explicit DesksTemplatesItemViewTestApi(
      const DesksTemplatesItemView* item_view)
      : item_view_(item_view) {
    DCHECK(item_view_);
  }
  DesksTemplatesItemViewTestApi(const DesksTemplatesItemViewTestApi&) = delete;
  DesksTemplatesItemViewTestApi& operator=(
      const DesksTemplatesItemViewTestApi&) = delete;
  ~DesksTemplatesItemViewTestApi() = default;

  const views::Textfield* name_view() const { return item_view_->name_view_; }

  const views::Label* time_view() const { return item_view_->time_view_; }

  const DesksTemplatesDeleteButton* delete_button() const {
    return item_view_->delete_button_;
  }

  const base::GUID uuid() const { return item_view_->uuid_; }

 private:
  const DesksTemplatesItemView* item_view_;
};

class DesksTemplatesTest : public OverviewTestBase {
 public:
  DesksTemplatesTest() = default;
  DesksTemplatesTest(const DesksTemplatesTest&) = delete;
  DesksTemplatesTest& operator=(const DesksTemplatesTest&) = delete;
  ~DesksTemplatesTest() override = default;

  // Adds an entry to the desks model directly without capturing a desk. Allows
  // for testing the names and times of the UI directly.
  void AddEntry(const base::GUID& uuid,
                const std::string& name,
                base::Time created_time) {
    auto desk_template = std::make_unique<DeskTemplate>(
        uuid.AsLowercaseString(), DeskTemplateSource::kUser, name,
        created_time);
    desk_template->set_desk_restore_data(
        std::make_unique<app_restore::RestoreData>());

    base::RunLoop loop;
    desk_model()->AddOrUpdateEntry(
        std::move(desk_template),
        base::BindLambdaForTesting(
            [&](desks_storage::DeskModel::AddOrUpdateEntryStatus status) {
              EXPECT_EQ(desks_storage::DeskModel::AddOrUpdateEntryStatus::kOk,
                        status);
              loop.Quit();
            }));
    loop.Run();
  }

  // Gets the current list of template entries from the desk model directly
  // without updating the UI.
  const std::vector<DeskTemplate*> GetAllEntries() {
    std::vector<DeskTemplate*> templates;

    base::RunLoop loop;
    desk_model()->GetAllEntries(base::BindLambdaForTesting(
        [&](desks_storage::DeskModel::GetAllEntriesStatus status,
            std::vector<DeskTemplate*> entries) {
          EXPECT_EQ(desks_storage::DeskModel::GetAllEntriesStatus::kOk, status);
          templates = entries;
          loop.Quit();
        }));
    loop.Run();

    return templates;
  }

  // Deletes an entry to the desks model directly without interacting with the
  // UI.
  void DeleteEntry(const base::GUID& uuid) {
    base::RunLoop loop;
    desk_model()->DeleteEntry(
        uuid.AsLowercaseString(),
        base::BindLambdaForTesting(
            [&](desks_storage::DeskModel::DeleteEntryStatus status) {
              loop.Quit();
            }));
    loop.Run();
  }

  // A lot of the UI relies on calling into the local desk data manager to
  // update, which sends callbacks via posting tasks. Call `WaitForUI()` if
  // testing a piece of the UI which calls into the desk model.
  void WaitForUI() {
    auto* overview_session = GetOverviewSession();
    DCHECK(overview_session);

    base::RunLoop run_loop;
    DesksTemplatesPresenterTestApi(
        overview_session->desks_templates_presenter())
        .SetOnUpdateUiClosure(run_loop.QuitClosure());
    run_loop.Run();
  }

  views::View* GetDesksTemplatesButtonForRoot(aura::Window* root_window,
                                              bool zero_state) {
    auto* overview_session = GetOverviewSession();
    if (!overview_session)
      return nullptr;

    const auto* overview_grid =
        overview_session->GetGridWithRootWindow(root_window);
    const auto* desks_bar_view = overview_grid->desks_bar_view();
    if (zero_state)
      return desks_bar_view->zero_state_desks_templates_button();
    return desks_bar_view->expanded_state_desks_templates_button();
  }

  // Shows the desks templates grid by emulating a click on the templates
  // button. It is required to have at least one entry in the desk model for the
  // button to be visible and clickable.
  void ShowDesksTemplatesGrids() {
    auto* root_window = Shell::GetPrimaryRootWindow();
    auto* zero_button =
        GetDesksTemplatesButtonForRoot(root_window, /*zero_state=*/true);
    auto* expanded_button =
        GetDesksTemplatesButtonForRoot(root_window, /*zero_state=*/false);
    ASSERT_TRUE(zero_button);
    ASSERT_TRUE(expanded_button);
    ASSERT_TRUE(zero_button->GetVisible() || expanded_button->GetVisible());

    if (zero_button->GetVisible())
      ClickOnView(zero_button);
    else
      ClickOnView(expanded_button);
  }

  void ClickOnView(const views::View* view) {
    DCHECK(view);

    const gfx::Point view_center = view->GetBoundsInScreen().CenterPoint();
    auto* event_generator = GetEventGenerator();
    event_generator->MoveMouseTo(view_center);
    event_generator->ClickLeftButton();
  }

  const std::vector<std::unique_ptr<OverviewGrid>>& GetOverviewGridList() {
    auto* overview_session = GetOverviewSession();
    DCHECK(overview_session);

    return overview_session->grid_list();
  }
};

// Tests the helpers `AddEntry()` and `DeleteEntry()`, which will be used in
// different tests.
TEST_F(DesksTemplatesTest, AddDeleteEntry) {
  const base::GUID expected_uuid = base::GUID::GenerateRandomV4();
  const std::string expected_name = "desk name";
  base::Time expected_time = base::Time::Now();
  AddEntry(expected_uuid, expected_name, expected_time);

  std::vector<DeskTemplate*> entries = GetAllEntries();
  ASSERT_EQ(1ul, entries.size());
  EXPECT_EQ(expected_uuid, entries[0]->uuid());
  EXPECT_EQ(base::UTF8ToUTF16(expected_name), entries[0]->template_name());
  EXPECT_EQ(expected_time, entries[0]->created_time());

  DeleteEntry(expected_uuid);
  EXPECT_EQ(0ul, desk_model()->GetEntryCount());
}

// Tests the desks templates button visibility.
TEST_F(DesksTemplatesTest, DesksTemplatesButtonVisibility) {
  // Helper function to verify which of the desks templates buttons are
  // currently shown.
  auto verify_button_visibilities = [this](bool zero_state_shown,
                                           bool expanded_state_shown,
                                           const std::string& trace_string) {
    SCOPED_TRACE(trace_string);
    for (auto* root_window : Shell::GetAllRootWindows()) {
      auto* zero_button =
          GetDesksTemplatesButtonForRoot(root_window, /*zero_state=*/true);
      auto* expanded_button =
          GetDesksTemplatesButtonForRoot(root_window, /*zero_state=*/false);
      ASSERT_TRUE(zero_button);
      ASSERT_TRUE(expanded_button);
      EXPECT_EQ(zero_state_shown, zero_button->GetVisible());
      EXPECT_EQ(expanded_state_shown, expanded_button->GetVisible());
    }
  };

  // The templates button should appear on all root windows.
  UpdateDisplay("800x700,801+0-800x700");
  ASSERT_EQ(2u, Shell::GetAllRootWindows().size());

  // There are no entries initially, so the none of the desks templates buttons
  // are visible.
  ToggleOverview();
  WaitForUI();
  verify_button_visibilities(/*zero_state_shown=*/false,
                             /*expanded_state_shown=*/false,
                             /*trace_string=*/"one-desk-zero-entries");

  // Exit overview and add an entry.
  ToggleOverview();
  const base::GUID guid = base::GUID::GenerateRandomV4();
  AddEntry(guid, "template", base::Time::Now());

  // Reenter overview and verify the zero state desks templates buttons are
  // visible since there is one entry to view.
  ToggleOverview();
  WaitForUI();
  verify_button_visibilities(/*zero_state_shown=*/true,
                             /*expanded_state_shown=*/false,
                             /*trace_string=*/"one-desk-one-entry");

  // Exit overview and create a new desk.
  ToggleOverview();
  DesksController::Get()->NewDesk(DesksCreationRemovalSource::kKeyboard);

  // Reenter overview and verify the expanded state desks templates buttons are
  // visible since there is one entry to view.
  ToggleOverview();
  WaitForUI();
  verify_button_visibilities(/*zero_state_shown=*/false,
                             /*expanded_state_shown=*/true,
                             /*trace_string=*/"two-desk-one-entry");

  // Exit overview and delete the entry.
  ToggleOverview();
  DeleteEntry(guid);

  // Reenter overview and verify neither of the buttons are shown.
  ToggleOverview();
  WaitForUI();
  verify_button_visibilities(/*zero_state_shown=*/false,
                             /*expanded_state_shown=*/false,
                             /*trace_string=*/"two-desk-zero-entries");
}

// Tests that the no windows widget is hidden when the desk templates grid is
// shown.
TEST_F(DesksTemplatesTest, NoWindowsLabelOnTemplateGridShow) {
  UpdateDisplay("400x300,400x300");

  // At least one entry is required for the templates grid to be shown.
  AddEntry(base::GUID::GenerateRandomV4(), "template", base::Time::Now());

  // Start overview mode. The no windows widget should be visible.
  ToggleOverview();
  WaitForUI();
  auto& grid_list = GetOverviewGridList();
  ASSERT_EQ(2u, grid_list.size());
  EXPECT_TRUE(grid_list[0]->no_windows_widget());
  EXPECT_TRUE(grid_list[1]->no_windows_widget());

  // Open the desk templates grid. The no windows widget should now be hidden.
  ShowDesksTemplatesGrids();
  EXPECT_FALSE(grid_list[0]->no_windows_widget());
  EXPECT_FALSE(grid_list[1]->no_windows_widget());
}

// Tests that overview items are hidden when the desk templates grid is shown.
TEST_F(DesksTemplatesTest, HideOverviewItemsOnTemplateGridShow) {
  UpdateDisplay("800x600,800x600");

  const base::GUID uuid_1 = base::GUID::GenerateRandomV4();
  const std::string name_1 = "template_1";
  base::Time time_1 = base::Time::Now();
  AddEntry(uuid_1, name_1, time_1);

  auto test_window = CreateTestWindow();

  // Start overview mode. The window is visible in the overview mode.
  ToggleOverview();
  WaitForUI();
  ASSERT_TRUE(GetOverviewSession());
  EXPECT_EQ(1.0f, test_window->layer()->opacity());

  // Open the desk templates grid. This should hide the window.
  ShowDesksTemplatesGrids();
  EXPECT_EQ(0.0f, test_window->layer()->opacity());

  // Exit overview mode. The window is restored and visible again.
  ToggleOverview();
  EXPECT_EQ(1.0f, test_window->layer()->opacity());
}

// Tests the modality of the dialogs shown in desks templates.
TEST_F(DesksTemplatesTest, DialogSystemModal) {
  UpdateDisplay("800x600,800x600");

  ToggleOverview();
  ASSERT_TRUE(GetOverviewSession());

  // Show one of the dialogs. Activating the dialog keeps us in overview mode.
  auto* dialog_controller = DesksTemplatesDialogController::Get();
  dialog_controller->ShowReplaceDialog(Shell::GetPrimaryRootWindow(), u"Bento");
  EXPECT_TRUE(Shell::IsSystemModalWindowOpen());
  ASSERT_TRUE(GetOverviewSession());

  // Checks that pressing tab does not trigger overview keyboard traversal.
  SendKey(ui::VKEY_TAB);
  EXPECT_FALSE(
      GetOverviewSession()->highlight_controller()->IsFocusHighlightVisible());

  // Fetch the widget for the dialog and test that it appears on the primary
  // root window.
  const views::Widget* dialog_widget = dialog_controller->dialog_widget();
  ASSERT_TRUE(dialog_widget);
  EXPECT_EQ(Shell::GetPrimaryRootWindow(),
            dialog_widget->GetNativeWindow()->GetRootWindow());

  // Hit escape to delete the dialog. Tests that there are no more system modal
  // windows open, and that we are still in overview because the dialog takes
  // the escape event, not the overview session.
  GetEventGenerator()->PressAndReleaseKey(ui::VKEY_ESCAPE);
  EXPECT_FALSE(Shell::IsSystemModalWindowOpen());
  EXPECT_TRUE(GetOverviewSession());
}

// Tests that the desks templates grid and item views are populated correctly.
TEST_F(DesksTemplatesTest, DesksTemplatesGridItems) {
  UpdateDisplay("800x600,800x600");

  const base::GUID uuid_1 = base::GUID::GenerateRandomV4();
  const std::string name_1 = "template_1";
  base::Time time_1 = base::Time::Now();
  AddEntry(uuid_1, name_1, time_1);

  const base::GUID uuid_2 = base::GUID::GenerateRandomV4();
  const std::string name_2 = "template_2";
  base::Time time_2 = time_1 + base::Hours(13);
  AddEntry(uuid_2, name_2, time_2);

  // Enter overview and show the Desks Templates Grid.
  ToggleOverview();
  WaitForUI();
  ShowDesksTemplatesGrids();
  WaitForUI();

  // Check that the grid is populated with the correct number of items, as
  // well as with the correct name and timestamp.
  for (auto& overview_grid : GetOverviewGridList()) {
    views::Widget* grid_widget = overview_grid->desks_templates_grid_widget();
    DCHECK(grid_widget);
    const DesksTemplatesGridView* templates_grid_view =
        static_cast<DesksTemplatesGridView*>(grid_widget->GetContentsView());
    DCHECK(templates_grid_view);
    std::vector<DesksTemplatesItemView*> grid_items =
        DesksTemplatesGridViewTestApi(templates_grid_view).grid_items();

    ASSERT_EQ(2ul, grid_items.size());

    // The grid item order is currently not guaranteed, so need to
    // verify that each template exists by looking them up by their
    // UUID.
    auto verify_template_grid_item = [grid_items](const base::GUID& uuid,
                                                  const std::string& name) {
      auto iter =
          std::find_if(grid_items.cbegin(), grid_items.cend(),
                       [uuid](const DesksTemplatesItemView* v) {
                         return DesksTemplatesItemViewTestApi(v).uuid() == uuid;
                       });
      ASSERT_NE(grid_items.end(), iter);
      DesksTemplatesItemViewTestApi test_api(*iter);
      EXPECT_EQ(base::UTF8ToUTF16(name), test_api.name_view()->GetText());
      EXPECT_FALSE(test_api.time_view()->GetText().empty());
    };

    verify_template_grid_item(uuid_1, name_1);
    verify_template_grid_item(uuid_2, name_2);
  }
}

// Tests that deleting templates in the templates grid functions correctly.
TEST_F(DesksTemplatesTest, DeleteTemplate) {
  UpdateDisplay("800x600,800x600");

  // Populate with several entries.
  const base::GUID uuid_1 = base::GUID::GenerateRandomV4();
  AddEntry(uuid_1, "template_1", base::Time::Now());

  const base::GUID uuid_2 = base::GUID::GenerateRandomV4();
  AddEntry(uuid_2, "template_2", base::Time::Now() + base::Hours(13));

  // This window should be hidden whenever the desk templates grid is open.
  auto test_window = CreateTestWindow();

  // Enter overview and show the Desks Templates Grid.
  ToggleOverview();
  WaitForUI();
  ShowDesksTemplatesGrids();
  WaitForUI();

  // The window is hidden because the desk templates grid is open.
  EXPECT_EQ(0.0f, test_window->layer()->opacity());

  auto& grid_list = GetOverviewGridList();
  ASSERT_EQ(2u, grid_list.size());
  views::Widget* grid_widget = grid_list[0]->desks_templates_grid_widget();
  DCHECK(grid_widget);
  const DesksTemplatesGridView* templates_grid_view =
      static_cast<DesksTemplatesGridView*>(grid_widget->GetContentsView());
  DCHECK(templates_grid_view);

  // Helper function for attempting to delete a template based on its uuid. Also
  // checks if the grid item count is as expected before deleting.
  auto delete_template = [&](const base::GUID uuid,
                             const size_t expected_current_item_count,
                             bool expect_template_exists = true) {
    std::vector<DesksTemplatesItemView*> grid_items =
        DesksTemplatesGridViewTestApi(templates_grid_view).grid_items();

    // Check the current grid item count.
    ASSERT_EQ(expected_current_item_count, grid_items.size());

    auto iter =
        std::find_if(grid_items.cbegin(), grid_items.cend(),
                     [&](const DesksTemplatesItemView* v) {
                       return DesksTemplatesItemViewTestApi(v).uuid() == uuid;
                     });

    if (!expect_template_exists) {
      ASSERT_EQ(grid_items.end(), iter);
      return;
    }

    ASSERT_NE(grid_items.end(), iter);

    ClickOnView(DesksTemplatesItemViewTestApi(*iter).delete_button());
    WaitForUI();
  };

  EXPECT_EQ(2ul, desk_model()->GetEntryCount());

  // Delete the template with `uuid_1`.
  delete_template(uuid_1, /*expected_current_item_count=*/2);
  EXPECT_EQ(0.0f, test_window->layer()->opacity());
  EXPECT_EQ(1ul, desk_model()->GetEntryCount());

  // Verifies that the template with `uuid_1`, doesn't exist anymore.
  delete_template(uuid_1, /*expected_current_item_count=*/1,
                  /*expect_template_exists=*/false);
  EXPECT_EQ(0.0f, test_window->layer()->opacity());
  EXPECT_EQ(1ul, desk_model()->GetEntryCount());

  // Delete the template with `uuid_2`.
  delete_template(uuid_2, /*expected_current_item_count=*/1);
  EXPECT_EQ(0ul, desk_model()->GetEntryCount());

  // After all templates have been deleted, check to ensure we have exited the
  // Desks Templates Grid on all displays. Also check to make sure the hidden
  // windows are shown again.
  EXPECT_EQ(1.0f, test_window->layer()->opacity());
  for (auto& overview_grid : GetOverviewGridList())
    EXPECT_FALSE(overview_grid->IsShowingDesksTemplatesGrid());
}

}  // namespace ash
