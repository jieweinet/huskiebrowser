// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/views/examples/dialog_example.h"

#include <memory>
#include <utility>

#include "base/macros.h"
#include "base/strings/utf_string_conversions.h"
#include "build/build_config.h"
#include "ui/views/bubble/bubble_dialog_delegate_view.h"
#include "ui/views/controls/button/checkbox.h"
#include "ui/views/controls/button/label_button.h"
#include "ui/views/controls/button/md_text_button.h"
#include "ui/views/controls/combobox/combobox.h"
#include "ui/views/controls/label.h"
#include "ui/views/controls/textfield/textfield.h"
#include "ui/views/examples/examples_window.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/flex_layout.h"
#include "ui/views/layout/layout_provider.h"
#include "ui/views/layout/table_layout.h"
#include "ui/views/widget/widget.h"
#include "ui/views/window/dialog_delegate.h"

using base::ASCIIToUTF16;

namespace views {
namespace examples {
namespace {

constexpr int kFakeModeless = ui::MODAL_TYPE_SYSTEM + 1;

}  // namespace

template <class DialogType>
class DialogExample::Delegate : public virtual DialogType {
 public:
  explicit Delegate(DialogExample* parent) : parent_(parent) {
    DialogDelegate::SetButtons(parent_->GetDialogButtons());
    DialogDelegate::SetButtonLabel(ui::DIALOG_BUTTON_OK,
                                   parent_->ok_button_label_->GetText());
    DialogDelegate::SetButtonLabel(ui::DIALOG_BUTTON_CANCEL,
                                   parent_->cancel_button_label_->GetText());
    WidgetDelegate::SetModalType(parent_->GetModalType());
  }

  Delegate(const Delegate&) = delete;
  Delegate& operator=(const Delegate&) = delete;

  void InitDelegate() {
    this->SetLayoutManager(std::make_unique<FillLayout>());
    auto body = std::make_unique<Label>(parent_->body_->GetText());
    body->SetMultiLine(true);
    body->SetHorizontalAlignment(gfx::ALIGN_LEFT);
    // Give the example code a way to change the body text.
    parent_->last_body_label_ = this->AddChildView(std::move(body));

    if (parent_->has_extra_button_->GetChecked()) {
      DialogDelegate::SetExtraView(std::make_unique<views::MdTextButton>(
          Button::PressedCallback(), parent_->extra_button_label_->GetText()));
    }
  }

 protected:
  std::u16string GetWindowTitle() const override {
    return parent_->title_->GetText();
  }

  bool Cancel() override { return parent_->AllowDialogClose(false); }
  bool Accept() override { return parent_->AllowDialogClose(true); }

 private:
  DialogExample* parent_;
};

class DialogExample::Bubble : public Delegate<BubbleDialogDelegateView> {
 public:
  Bubble(DialogExample* parent, View* anchor)
      : BubbleDialogDelegateView(anchor, BubbleBorder::TOP_LEFT),
        Delegate(parent) {
    set_close_on_deactivate(!parent->persistent_bubble_->GetChecked());
  }

  Bubble(const Bubble&) = delete;
  Bubble& operator=(const Bubble&) = delete;

  // BubbleDialogDelegateView:
  void Init() override { InitDelegate(); }
};

class DialogExample::Dialog : public Delegate<DialogDelegateView> {
 public:
  explicit Dialog(DialogExample* parent) : Delegate(parent) {
    // Mac supports resizing of modal dialogs (parent or window-modal). On other
    // platforms this will be weird unless the modal type is "none", but helps
    // test layout.
    SetCanResize(true);
  }

  Dialog(const Dialog&) = delete;
  Dialog& operator=(const Dialog&) = delete;
};

DialogExample::DialogExample()
    : ExampleBase("Dialog"),
      mode_model_({
          ui::SimpleComboboxModel::Item(u"Modeless"),
          ui::SimpleComboboxModel::Item(u"Window Modal"),
          ui::SimpleComboboxModel::Item(u"Child Modal"),
          ui::SimpleComboboxModel::Item(u"System Modal"),
          ui::SimpleComboboxModel::Item(u"Fake Modeless (non-bubbles)"),
      }) {}

DialogExample::~DialogExample() = default;

void DialogExample::CreateExampleView(View* container) {
  auto* flex_layout =
      container->SetLayoutManager(std::make_unique<FlexLayout>());
  flex_layout->SetOrientation(LayoutOrientation::kVertical);

  auto* table = container->AddChildView(std::make_unique<View>());
  views::LayoutProvider* provider = views::LayoutProvider::Get();
  const int horizontal_spacing =
      provider->GetDistanceMetric(views::DISTANCE_RELATED_BUTTON_HORIZONTAL);
  auto* table_layout = table->SetLayoutManager(std::make_unique<TableLayout>());
  table_layout
      ->AddColumn(LayoutAlignment::kStart, LayoutAlignment::kStretch,
                  TableLayout::kFixedSize,
                  TableLayout::ColumnSize::kUsePreferred, 0, 0)
      .AddPaddingColumn(TableLayout::kFixedSize, horizontal_spacing)
      .AddColumn(LayoutAlignment::kStretch, LayoutAlignment::kStretch, 1.0f,
                 TableLayout::ColumnSize::kUsePreferred, 0, 0)
      .AddPaddingColumn(TableLayout::kFixedSize, horizontal_spacing)
      .AddColumn(LayoutAlignment::kStretch, LayoutAlignment::kStretch,
                 TableLayout::kFixedSize,
                 TableLayout::ColumnSize::kUsePreferred, 0, 0);
  const int vertical_padding =
      provider->GetDistanceMetric(views::DISTANCE_RELATED_CONTROL_VERTICAL);
  for (int i = 0; i < 7; ++i) {
    table_layout->AddPaddingRow(TableLayout::kFixedSize, vertical_padding)
        .AddRows(1, TableLayout::kFixedSize);
  }

  StartTextfieldRow(table, &title_, "Dialog Title", "Title");
  table->AddChildView(std::make_unique<View>());

  StartTextfieldRow(table, &body_, "Dialog Body Text", "Body Text");
  table->AddChildView(std::make_unique<View>());

  StartTextfieldRow(table, &ok_button_label_, "OK Button Label", "Done");
  AddCheckbox(table, &has_ok_button_);

  StartTextfieldRow(table, &cancel_button_label_, "Cancel Button Label",
                    "Cancel");
  AddCheckbox(table, &has_cancel_button_);

  StartTextfieldRow(table, &extra_button_label_, "Extra Button Label", "Edit");
  AddCheckbox(table, &has_extra_button_);

  table->AddChildView(std::make_unique<Label>(u"Modal Type"));
  mode_ = table->AddChildView(std::make_unique<Combobox>(&mode_model_));
  mode_->SetCallback(base::BindRepeating(&DialogExample::OnPerformAction,
                                         base::Unretained(this)));
  // TODO(pbos): Figure out a reasonable accessible name here.
  mode_->SetAccessibleName(u"TODO: Add a reasonable Accessible Name");
  mode_->SetSelectedIndex(ui::MODAL_TYPE_CHILD);
  table->AddChildView(std::make_unique<View>());

  table->AddChildView(std::make_unique<Label>(u"Bubble"));
  AddCheckbox(table, &bubble_);
  AddCheckbox(table, &persistent_bubble_);
  persistent_bubble_->SetText(u"Persistent");

  show_ = container->AddChildView(std::make_unique<views::MdTextButton>(
      base::BindRepeating(&DialogExample::ShowButtonPressed,
                          base::Unretained(this)),
      u"Show"));
  show_->SetProperty(kCrossAxisAlignmentKey, LayoutAlignment::kCenter);
  show_->SetProperty(
      kMarginsKey, gfx::Insets(provider->GetDistanceMetric(
                                   views::DISTANCE_UNRELATED_CONTROL_VERTICAL),
                               0, 0, 0));
}

void DialogExample::StartTextfieldRow(View* parent,
                                      Textfield** member,
                                      const char* label,
                                      const char* value) {
  parent->AddChildView(std::make_unique<Label>(base::ASCIIToUTF16(label)));
  auto textfield = std::make_unique<Textfield>();
  textfield->set_controller(this);
  textfield->SetText(base::ASCIIToUTF16(value));
  // TODO(pbos): Figure out a reasonable accessible name here.
  textfield->SetAccessibleName(u"TODO: Add a reasonable Accessible Name");
  *member = parent->AddChildView(std::move(textfield));
}

void DialogExample::AddCheckbox(View* parent, Checkbox** member) {
  auto callback = member == &bubble_ ? &DialogExample::BubbleCheckboxPressed
                                     : &DialogExample::OtherCheckboxPressed;
  auto checkbox = std::make_unique<Checkbox>(
      std::u16string(), base::BindRepeating(callback, base::Unretained(this)));
  checkbox->SetChecked(true);
  // TODO(pbos): Figure out a reasonable accessible name here.
  checkbox->SetAccessibleName(u"TODO: Add a reasonable Accessible Name");
  *member = parent->AddChildView(std::move(checkbox));
}

ui::ModalType DialogExample::GetModalType() const {
  // "Fake" modeless happens when a DialogDelegate specifies window-modal, but
  // doesn't provide a parent window.
  // TODO(ellyjones): This doesn't work on Mac at all - something should happen
  // other than changing modality on the fly like this. In fact, it should be
  // impossible to change modality in a live dialog at all, and this example
  // should stop doing it.
  if (mode_->GetSelectedIndex() == kFakeModeless)
    return ui::MODAL_TYPE_WINDOW;

  return static_cast<ui::ModalType>(mode_->GetSelectedIndex());
}

int DialogExample::GetDialogButtons() const {
  int buttons = 0;
  if (has_ok_button_->GetChecked())
    buttons |= ui::DIALOG_BUTTON_OK;
  if (has_cancel_button_->GetChecked())
    buttons |= ui::DIALOG_BUTTON_CANCEL;
  return buttons;
}

bool DialogExample::AllowDialogClose(bool accept) {
  PrintStatus("Dialog closed with %s.", accept ? "Accept" : "Cancel");
  last_dialog_ = nullptr;
  last_body_label_ = nullptr;
  return true;
}

void DialogExample::ResizeDialog() {
  DCHECK(last_dialog_);
  Widget* widget = last_dialog_->GetWidget();
  gfx::Rect preferred_bounds(widget->GetRestoredBounds());
  preferred_bounds.set_size(widget->non_client_view()->GetPreferredSize());

  // Q: Do we need NonClientFrameView::GetWindowBoundsForClientBounds() here?
  // A: When DialogCientView properly feeds back sizes, we do not.
  widget->SetBoundsConstrained(preferred_bounds);

  // For user-resizable dialogs, ensure the window manager enforces any new
  // minimum size.
  widget->OnSizeConstraintsChanged();
}

void DialogExample::ShowButtonPressed() {
  if (bubble_->GetChecked()) {
    // |bubble| will be destroyed by its widget when the widget is destroyed.
    Bubble* bubble = new Bubble(this, show_);
    last_dialog_ = bubble;
    BubbleDialogDelegateView::CreateBubble(bubble);
  } else {
    // |dialog| will be destroyed by its widget when the widget is destroyed.
    Dialog* dialog = new Dialog(this);
    last_dialog_ = dialog;
    dialog->InitDelegate();

    // constrained_window::CreateBrowserModalDialogViews() allows dialogs to
    // be created as MODAL_TYPE_WINDOW without specifying a parent.
    gfx::NativeView parent = nullptr;
    if (mode_->GetSelectedIndex() != kFakeModeless)
      parent = example_view()->GetWidget()->GetNativeView();

    DialogDelegate::CreateDialogWidget(
        dialog, example_view()->GetWidget()->GetNativeWindow(), parent);
  }
  last_dialog_->GetWidget()->Show();
}

void DialogExample::BubbleCheckboxPressed() {
  if (bubble_->GetChecked() && GetModalType() != ui::MODAL_TYPE_CHILD) {
    mode_->SetSelectedIndex(ui::MODAL_TYPE_CHILD);
    LogStatus("You nearly always want Child Modal for bubbles.");
  }
  persistent_bubble_->SetEnabled(bubble_->GetChecked());
  OnPerformAction();  // Validate the modal type.

  if (!bubble_->GetChecked() && GetModalType() == ui::MODAL_TYPE_CHILD) {
    // Do something reasonable when simply unchecking bubble and re-enable.
    mode_->SetSelectedIndex(ui::MODAL_TYPE_WINDOW);
    OnPerformAction();
  }
}

void DialogExample::OtherCheckboxPressed() {
  // Buttons other than show and bubble are pressed. They are all checkboxes.
  // Update the dialog if there is one.
  if (last_dialog_) {
    // TODO(crbug.com/1261666): This can segfault.
    last_dialog_->DialogModelChanged();
    ResizeDialog();
  }
}

void DialogExample::ContentsChanged(Textfield* sender,
                                    const std::u16string& new_contents) {
  if (!last_dialog_)
    return;

  if (sender == extra_button_label_)
    LogStatus("DialogDelegate can never refresh the extra view.");

  if (sender == title_) {
    last_dialog_->GetWidget()->UpdateWindowTitle();
  } else if (sender == body_) {
    last_body_label_->SetText(new_contents);
  } else {
    last_dialog_->DialogModelChanged();
  }

  ResizeDialog();
}

void DialogExample::OnPerformAction() {
  bool enable = bubble_->GetChecked() || GetModalType() != ui::MODAL_TYPE_CHILD;
#if defined(OS_MAC)
  enable = enable && GetModalType() != ui::MODAL_TYPE_SYSTEM;
#endif
  show_->SetEnabled(enable);
  if (!enable && GetModalType() == ui::MODAL_TYPE_CHILD)
    LogStatus("MODAL_TYPE_CHILD can't be used with non-bubbles.");
  if (!enable && GetModalType() == ui::MODAL_TYPE_SYSTEM)
    LogStatus("MODAL_TYPE_SYSTEM isn't supported on Mac.");
}

}  // namespace examples
}  // namespace views
