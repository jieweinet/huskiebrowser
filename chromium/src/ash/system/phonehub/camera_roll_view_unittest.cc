// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/system/phonehub/camera_roll_view.h"

#include "ash/test/ash_test_base.h"
#include "chromeos/components/phonehub/camera_roll_item.h"
#include "chromeos/components/phonehub/fake_camera_roll_manager.h"
#include "chromeos/components/phonehub/fake_user_action_recorder.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "ui/gfx/image/image.h"
#include "ui/views/controls/button/menu_button.h"
#include "ui/views/view.h"

namespace ash {

class CameraRollViewTest : public AshTestBase {
 public:
  CameraRollViewTest() = default;
  ~CameraRollViewTest() override = default;

  // AshTestBase:
  void SetUp() override {
    AshTestBase::SetUp();
    fake_camera_roll_manager_ =
        std::make_unique<chromeos::phonehub::FakeCameraRollManager>();
    fake_user_action_recorder_ =
        std::make_unique<chromeos::phonehub::FakeUserActionRecorder>();
    camera_roll_view_ = std::make_unique<CameraRollView>(
        fake_camera_roll_manager_.get(), fake_user_action_recorder_.get());
  }

  void TearDown() override {
    camera_roll_view_.reset();
    fake_camera_roll_manager_.reset();
    AshTestBase::TearDown();
  }

 protected:
  const CameraRollView* camera_roll_view() const {
    return camera_roll_view_.get();
  }

  chromeos::phonehub::FakeCameraRollManager* fake_camera_roll_manager() {
    return fake_camera_roll_manager_.get();
  }

  const std::vector<chromeos::phonehub::CameraRollItem> CreateFakeItems(
      int num) {
    std::vector<chromeos::phonehub::CameraRollItem> items;
    for (int i = num; i > 0; --i) {
      chromeos::phonehub::proto::CameraRollItemMetadata metadata;
      metadata.set_key(base::NumberToString(i));
      metadata.set_mime_type("image/jpeg");
      metadata.set_last_modified_millis(1577865600 + i);
      metadata.set_file_size_bytes(123456);
      metadata.set_file_name("fake_file_" + base::NumberToString(i) + ".jpg");

      SkBitmap bitmap;
      bitmap.allocN32Pixels(96, 96);
      gfx::Image thumbnail = gfx::Image::CreateFrom1xBitmap(bitmap);

      items.emplace_back(metadata, thumbnail);
    }
    return items;
  }

  const views::View* GetItemsView() const {
    return camera_roll_view()->children().at(1);
  }

  const views::MenuButton* GetThumbnailView(int index) const {
    return static_cast<views::MenuButton*>(
        GetItemsView()->children().at(index));
  }

 private:
  std::unique_ptr<CameraRollView> camera_roll_view_;
  std::unique_ptr<chromeos::phonehub::FakeUserActionRecorder>
      fake_user_action_recorder_;
  std::unique_ptr<chromeos::phonehub::FakeCameraRollManager>
      fake_camera_roll_manager_;
};

TEST_F(CameraRollViewTest, ViewVisibility) {
  // The camera roll view is not visible if there are no items available and
  // visible when there are one or more items available.
  fake_camera_roll_manager()->ClearCurrentItems();
  EXPECT_FALSE(camera_roll_view()->GetVisible());

  fake_camera_roll_manager()->SetCurrentItems(CreateFakeItems(1));
  EXPECT_TRUE(camera_roll_view()->GetVisible());

  fake_camera_roll_manager()->ClearCurrentItems();
  EXPECT_FALSE(camera_roll_view()->GetVisible());
}

TEST_F(CameraRollViewTest, SingleItem) {
  // Set 1 camera roll item.
  size_t expected_size = 1;
  fake_camera_roll_manager()->SetCurrentItems(CreateFakeItems(expected_size));
  EXPECT_EQ(GetItemsView()->children().size(), expected_size);
}

TEST_F(CameraRollViewTest, MultipleItems) {
  // Set 4 camera roll items.
  size_t expected_size = 4;
  fake_camera_roll_manager()->SetCurrentItems(CreateFakeItems(expected_size));
  EXPECT_EQ(GetItemsView()->children().size(), expected_size);
}

TEST_F(CameraRollViewTest, ViewLayout) {
  // Test the layout size and positions of the items. If the layout is being
  // intentionally changed this test will need to be updated.
  fake_camera_roll_manager()->SetCurrentItems(CreateFakeItems(4));
  EXPECT_EQ(camera_roll_view()->items_view_->CalculatePreferredSize(),
            gfx::Size(328, 82));
  EXPECT_EQ(camera_roll_view()->items_view_->GetCameraRollItemPosition(0),
            gfx::Point(4, 4));
  EXPECT_EQ(camera_roll_view()->items_view_->GetCameraRollItemPosition(1),
            gfx::Point(86, 4));
  EXPECT_EQ(camera_roll_view()->items_view_->GetCameraRollItemPosition(2),
            gfx::Point(168, 4));
  EXPECT_EQ(camera_roll_view()->items_view_->GetCameraRollItemPosition(3),
            gfx::Point(250, 4));
}

TEST_F(CameraRollViewTest, AccessibleNameAndTooltip) {
  fake_camera_roll_manager()->SetCurrentItems(CreateFakeItems(4));

  EXPECT_EQ(u"Recent photo 1 of 4.", GetThumbnailView(0)->GetAccessibleName());
  EXPECT_EQ(u"Recent photo 1 of 4.", GetThumbnailView(0)->GetTooltipText());
  EXPECT_EQ(u"Recent photo 2 of 4.", GetThumbnailView(1)->GetAccessibleName());
  EXPECT_EQ(u"Recent photo 2 of 4.", GetThumbnailView(1)->GetTooltipText());
  EXPECT_EQ(u"Recent photo 3 of 4.", GetThumbnailView(2)->GetAccessibleName());
  EXPECT_EQ(u"Recent photo 3 of 4.", GetThumbnailView(2)->GetTooltipText());
  EXPECT_EQ(u"Recent photo 4 of 4.", GetThumbnailView(3)->GetAccessibleName());
  EXPECT_EQ(u"Recent photo 4 of 4.", GetThumbnailView(3)->GetTooltipText());
}

}  // namespace ash
