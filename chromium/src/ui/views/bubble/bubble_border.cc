// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/views/bubble/bubble_border.h"

#include <algorithm>
#include <map>
#include <tuple>
#include <utility>

#include "base/check_op.h"
#include "base/logging.h"
#include "base/no_destructor.h"
#include "base/notreached.h"
#include "cc/paint/paint_flags.h"
#include "third_party/skia/include/core/SkDrawLooper.h"
#include "third_party/skia/include/core/SkPath.h"
#include "third_party/skia/include/core/SkPoint.h"
#include "ui/color/color_id.h"
#include "ui/color/color_provider.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/color_palette.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/geometry/rrect_f.h"
#include "ui/gfx/geometry/skia_conversions.h"
#include "ui/gfx/geometry/vector2d.h"
#include "ui/gfx/scoped_canvas.h"
#include "ui/gfx/shadow_value.h"
#include "ui/gfx/skia_paint_util.h"
#include "ui/resources/grit/ui_resources.h"
#include "ui/views/view.h"

namespace views {

namespace {

// GetShadowValues and GetBorderAndShadowFlags cache their results. The shadow
// values depend on both the shadow elevation and color, so we create a tuple to
// key the cache.
using ShadowCacheKey = std::tuple<int, SkColor>;

enum class BubbleArrowSide { kLeft, kRight, kTop, kBottom };

BubbleArrowSide GetBubbleArrowSide(BubbleBorder::Arrow arrow) {
  DCHECK(BubbleBorder::has_arrow(arrow));
  // Note: VERTICAL arrows are on the sides of the bubble, while !VERTICAL are
  // on the top or bottom.
  if (arrow & BubbleBorder::VERTICAL) {
    return (arrow & BubbleBorder::RIGHT) ? BubbleArrowSide::kRight
                                         : BubbleArrowSide::kLeft;
  }
  return (arrow & BubbleBorder::BOTTOM) ? BubbleArrowSide::kBottom
                                        : BubbleArrowSide::kTop;
}

// Utility functions for getting alignment points on the edge of a rectangle.
gfx::Point CenterTop(const gfx::Rect& rect) {
  return gfx::Point(rect.CenterPoint().x(), rect.y());
}

gfx::Point CenterBottom(const gfx::Rect& rect) {
  return gfx::Point(rect.CenterPoint().x(), rect.bottom());
}

gfx::Point LeftCenter(const gfx::Rect& rect) {
  return gfx::Point(rect.x(), rect.CenterPoint().y());
}

gfx::Point RightCenter(const gfx::Rect& rect) {
  return gfx::Point(rect.right(), rect.CenterPoint().y());
}

SkColor GetKeyShadowColor(int elevation,
                          const ui::ColorProvider* color_provider) {
  switch (elevation) {
    case 3: {
      return color_provider->GetColor(
          ui::kColorShadowValueKeyShadowElevationThree);
    }
    case 16: {
      return color_provider->GetColor(
          ui::kColorShadowValueKeyShadowElevationSixteen);
    }
    default:
      // This surface has not been updated for Refresh. Fall back to the
      // deprecated style.
      return color_provider->GetColor(ui::kColorShadowBase);
  }
}

SkColor GetAmbientShadowColor(int elevation,
                              const ui::ColorProvider* color_provider) {
  switch (elevation) {
    case 3: {
      return color_provider->GetColor(
          ui::kColorShadowValueAmbientShadowElevationThree);
    }
    case 16: {
      return color_provider->GetColor(
          ui::kColorShadowValueAmbientShadowElevationSixteen);
    }
    default:
      // This surface has not been updated for Refresh. Fall back to the
      // deprecated style.
      return color_provider->GetColor(ui::kColorShadowBase);
  }
}

gfx::Insets GetVisibleArrowInsets(BubbleBorder::Arrow arrow, bool include_gap) {
  DCHECK(BubbleBorder::has_arrow(arrow));
  const int arrow_size = include_gap ? BubbleBorder::kVisibleArrowGap +
                                           BubbleBorder::kVisibleArrowLength
                                     : BubbleBorder::kVisibleArrowLength;
  gfx::Insets result;
  switch (GetBubbleArrowSide(arrow)) {
    case BubbleArrowSide::kRight:
      result.set_right(arrow_size);
      break;
    case BubbleArrowSide::kLeft:
      result.set_left(arrow_size);
      break;
    case BubbleArrowSide::kTop:
      result.set_top(arrow_size);
      break;
    case BubbleArrowSide::kBottom:
      result.set_bottom(arrow_size);
      break;
  }
  return result;
}

enum BubbleArrowPart { kFill, kBorder };

SkPath GetVisibleArrowPath(BubbleBorder::Arrow arrow,
                           const gfx::Rect& bounds,
                           BubbleArrowPart part) {
  constexpr size_t kNumPoints = 4;
  gfx::RectF bounds_f(bounds);
  SkPoint points[kNumPoints];
  switch (GetBubbleArrowSide(arrow)) {
    case BubbleArrowSide::kRight:
      points[0] = {bounds_f.x(), bounds_f.y()};
      points[1] = {bounds_f.right(),
                   bounds_f.y() + BubbleBorder::kVisibleArrowRadius - 1};
      points[2] = {bounds_f.right(),
                   bounds_f.y() + BubbleBorder::kVisibleArrowRadius};
      points[3] = {bounds_f.x(), bounds_f.bottom() - 1};
      break;
    case BubbleArrowSide::kLeft:
      points[0] = {bounds_f.right(), bounds_f.bottom() - 1};
      points[1] = {bounds_f.x(),
                   bounds_f.y() + BubbleBorder::kVisibleArrowRadius};
      points[2] = {bounds_f.x(),
                   bounds_f.y() + BubbleBorder::kVisibleArrowRadius - 1};
      points[3] = {bounds_f.right(), bounds_f.y()};
      break;
    case BubbleArrowSide::kTop:
      points[0] = {bounds_f.x(), bounds_f.bottom()};
      points[1] = {bounds_f.x() + BubbleBorder::kVisibleArrowRadius - 1,
                   bounds_f.y()};
      points[2] = {bounds_f.x() + BubbleBorder::kVisibleArrowRadius,
                   bounds_f.y()};
      points[3] = {bounds_f.right() - 1, bounds_f.bottom()};
      break;
    case BubbleArrowSide::kBottom:
      points[0] = {bounds_f.right() - 1, bounds_f.y()};
      points[1] = {bounds_f.x() + BubbleBorder::kVisibleArrowRadius,
                   bounds_f.bottom()};
      points[2] = {bounds_f.x() + BubbleBorder::kVisibleArrowRadius - 1,
                   bounds_f.bottom()};
      points[3] = {bounds_f.x(), bounds_f.y()};
      break;
  }

  return SkPath::Polygon(points, kNumPoints, part == BubbleArrowPart::kFill);
}

const gfx::ShadowValues& GetShadowValues(
    const ui::ColorProvider* color_provider,
    absl::optional<int> elevation) {
  // If the color provider does not exist the shadow values are being created in
  // order to calculate Insets. In that case the color plays no role so always
  // set those colors to gfx::kPlaceholderColor.

  SkColor color = color_provider
                      ? color_provider->GetColor(ui::kColorShadowBase)
                      : gfx::kPlaceholderColor;

  // The shadows are always the same for any elevation and color combination, so
  // construct them once and cache.
  static base::NoDestructor<std::map<ShadowCacheKey, gfx::ShadowValues>>
      shadow_map;
  ShadowCacheKey key(elevation.value_or(-1), color);

  if (shadow_map->find(key) != shadow_map->end())
    return shadow_map->find(key)->second;

  gfx::ShadowValues shadows;
  if (elevation.has_value()) {
    DCHECK_GE(elevation.value(), 0);
    SkColor key_shadow_color =
        color_provider ? GetKeyShadowColor(elevation.value(), color_provider)
                       : gfx::kPlaceholderColor;
    SkColor ambient_shadow_color =
        color_provider
            ? GetAmbientShadowColor(elevation.value(), color_provider)
            : gfx::kPlaceholderColor;
    shadows = gfx::ShadowValue::MakeShadowValues(
        elevation.value(), key_shadow_color, ambient_shadow_color);
  } else {
    constexpr int kSmallShadowVerticalOffset = 2;
    constexpr int kSmallShadowBlur = 4;
    SkColor kSmallShadowColor =
        color_provider
            ? color_provider->GetColor(ui::kColorBubbleBorderShadowSmall)
            : gfx::kPlaceholderColor;
    SkColor kLargeShadowColor =
        color_provider
            ? color_provider->GetColor(ui::kColorBubbleBorderShadowLarge)
            : gfx::kPlaceholderColor;
    // gfx::ShadowValue counts blur pixels both inside and outside the shape,
    // whereas these blur values only describe the outside portion, hence they
    // must be doubled.
    shadows = gfx::ShadowValues({
        {gfx::Vector2d(0, kSmallShadowVerticalOffset), 2 * kSmallShadowBlur,
         kSmallShadowColor},
        {gfx::Vector2d(0, BubbleBorder::kShadowVerticalOffset),
         2 * BubbleBorder::kShadowBlur, kLargeShadowColor},
    });
  }

  shadow_map->insert({key, shadows});
  return shadow_map->find(key)->second;
}

const cc::PaintFlags& GetBorderAndShadowFlags(
    const ui::ColorProvider* color_provider,
    absl::optional<int> elevation) {
  // The flags are always the same for any elevation and color combination, so
  // construct them once and cache.
  static base::NoDestructor<std::map<ShadowCacheKey, cc::PaintFlags>> flag_map;
  ShadowCacheKey key(elevation.value_or(-1),
                     color_provider->GetColor(ui::kColorShadowBase));

  if (flag_map->find(key) != flag_map->end())
    return flag_map->find(key)->second;

  cc::PaintFlags flags;
  flags.setColor(
      color_provider->GetColor(ui::kColorBubbleBorderWhenShadowPresent));
  flags.setAntiAlias(true);
  flags.setLooper(
      gfx::CreateShadowDrawLooper(GetShadowValues(color_provider, elevation)));
  flag_map->insert({key, flags});
  return flag_map->find(key)->second;
}

template <typename T>
void DrawBorderAndShadowImpl(
    T rect,
    void (cc::PaintCanvas::*draw)(const T&, const cc::PaintFlags&),
    gfx::Canvas* canvas,
    const ui::ColorProvider* color_provider,
    absl::optional<int> shadow_elevation = absl::nullopt) {
  // Borders with custom shadow elevations do not draw the 1px border.
  if (!shadow_elevation.has_value()) {
    // Provide a 1 px border outside the bounds.
    constexpr int kBorderStrokeThicknessPx = 1;
    const SkScalar one_pixel =
        SkFloatToScalar(kBorderStrokeThicknessPx / canvas->image_scale());
    rect.outset(one_pixel, one_pixel);
  }

  (canvas->sk_canvas()->*draw)(
      rect, GetBorderAndShadowFlags(color_provider, shadow_elevation));
}

// Returns the appropriate anchor point on the edge of the |anchor_rect| for a
// given |arrow| position.
gfx::Point GetArrowAnchorPointFromAnchorRect(BubbleBorder::Arrow arrow,
                                             const gfx::Rect& anchor_rect) {
  switch (arrow) {
    case BubbleBorder::TOP_LEFT:
      return anchor_rect.bottom_left();

    case BubbleBorder::TOP_RIGHT:
      return anchor_rect.bottom_right();

    case BubbleBorder::BOTTOM_LEFT:
      return anchor_rect.origin();

    case BubbleBorder::BOTTOM_RIGHT:
      return anchor_rect.top_right();

    case BubbleBorder::LEFT_TOP:
      return anchor_rect.top_right();

    case BubbleBorder::RIGHT_TOP:
      return anchor_rect.origin();

    case BubbleBorder::LEFT_BOTTOM:
      return anchor_rect.bottom_right();

    case BubbleBorder::RIGHT_BOTTOM:
      return anchor_rect.bottom_left();

    case BubbleBorder::TOP_CENTER:
      return CenterBottom(anchor_rect);

    case BubbleBorder::BOTTOM_CENTER:
      return CenterTop(anchor_rect);

    case BubbleBorder::LEFT_CENTER:
      return RightCenter(anchor_rect);

    case BubbleBorder::RIGHT_CENTER:
      return LeftCenter(anchor_rect);

    default:
      NOTREACHED();
      return gfx::Point();
  }
}

// Returns the origin offset to move the |contents_bounds| to be placed
// appropriately for a given |arrow| at the |anchor_point|.
gfx::Vector2d GetContentBoundsOffsetToArrowAnchorPoint(
    const gfx::Rect& contents_bounds,
    BubbleBorder::Arrow arrow,
    const gfx::Point& anchor_point) {
  switch (arrow) {
    case BubbleBorder::TOP_LEFT:
      return anchor_point - contents_bounds.origin();

    case BubbleBorder::TOP_RIGHT:
      return anchor_point - contents_bounds.top_right();

    case BubbleBorder::BOTTOM_LEFT:
      return anchor_point - contents_bounds.bottom_left();

    case BubbleBorder::BOTTOM_RIGHT:
      return anchor_point - contents_bounds.bottom_right();

    case BubbleBorder::LEFT_TOP:
      return anchor_point - contents_bounds.origin();

    case BubbleBorder::RIGHT_TOP:
      return anchor_point - contents_bounds.top_right();

    case BubbleBorder::LEFT_BOTTOM:
      return anchor_point - contents_bounds.bottom_left();

    case BubbleBorder::RIGHT_BOTTOM:
      return anchor_point - contents_bounds.bottom_right();

    case BubbleBorder::TOP_CENTER:
      return anchor_point - CenterTop(contents_bounds);

    case BubbleBorder::BOTTOM_CENTER:
      return anchor_point - CenterBottom(contents_bounds);

    case BubbleBorder::LEFT_CENTER:
      return anchor_point - LeftCenter(contents_bounds);

    case BubbleBorder::RIGHT_CENTER:
      return anchor_point - RightCenter(contents_bounds);

    default:
      NOTREACHED();
      return gfx::Vector2d();
  }
}

}  // namespace

constexpr int BubbleBorder::kBorderThicknessDip;
constexpr int BubbleBorder::kShadowBlur;
constexpr int BubbleBorder::kShadowVerticalOffset;
constexpr int BubbleBorder::kVisibleArrowGap;
constexpr int BubbleBorder::kVisibleArrowLength;
constexpr int BubbleBorder::kVisibleArrowRadius;
constexpr int BubbleBorder::kVisibleArrowBuffer;

BubbleBorder::BubbleBorder(Arrow arrow, Shadow shadow, SkColor color)
    : arrow_(arrow),
      arrow_offset_(0),
      shadow_(shadow),
      background_color_(color),
      use_theme_background_color_(false) {
  DCHECK(shadow_ < SHADOW_COUNT);
}

BubbleBorder::~BubbleBorder() = default;

// static
gfx::Insets BubbleBorder::GetBorderAndShadowInsets(
    absl::optional<int> elevation) {
  // Borders with custom shadow elevations do not draw the 1px border.
  if (elevation.has_value())
    return -gfx::ShadowValue::GetMargin(GetShadowValues(nullptr, elevation));

  constexpr gfx::Insets blur(kShadowBlur + kBorderThicknessDip);
  constexpr gfx::Insets offset(-kShadowVerticalOffset, 0, kShadowVerticalOffset,
                               0);
  return blur + offset;
}

void BubbleBorder::SetCornerRadius(int corner_radius) {
  corner_radius_ = corner_radius;
}

// static
gfx::Size BubbleBorder::GetVisibleArrowSize(BubbleBorder::Arrow arrow) {
  constexpr int kVisibleArrowDiameter = 2 * BubbleBorder::kVisibleArrowRadius;

  return IsVerticalArrow(arrow) ? gfx::Size(kVisibleArrowDiameter,
                                            BubbleBorder::kVisibleArrowLength)
                                : gfx::Size(BubbleBorder::kVisibleArrowLength,
                                            kVisibleArrowDiameter);
}

gfx::Rect BubbleBorder::GetBounds(const gfx::Rect& anchor_rect,
                                  const gfx::Size& contents_size) const {
  const gfx::Size size(GetSizeForContentsSize(contents_size));
  // In floating mode, the bounds of the bubble border and the |anchor_rect|
  // have coinciding central points.
  if (arrow_ == FLOAT) {
    gfx::Rect rect(anchor_rect.CenterPoint(), size);
    rect.Offset(gfx::Vector2d(-size.width() / 2, -size.height() / 2));
    return rect;
  }

  // If no arrow is used, in the vertical direction, the bubble is placed below
  // the |anchor_rect| while they have coinciding horizontal centers.
  if (arrow_ == NONE) {
    gfx::Rect rect(anchor_rect.bottom_center(), size);
    rect.Offset(gfx::Vector2d(-size.width() / 2, 0));
    return rect;
  }

  // In all other cases, the used arrow determines the placement of the bubble
  // with respect to the |anchor_rect|.
  gfx::Rect contents_bounds(contents_size);
  // Always apply the border part of the inset before calculating coordinates,
  // that ensures the bubble's border is aligned with the anchor's border.
  // For the purposes of positioning, the border is rounded up to a dip, which
  // may cause misalignment in scale factors greater than 1.
  // TODO(estade): when it becomes possible to provide px bounds instead of
  // dip bounds, fix this.
  // Borders with custom shadow elevations do not draw the 1px border.
  const gfx::Insets border_insets =
      shadow_ == NO_SHADOW || md_shadow_elevation_.has_value()
          ? gfx::Insets()
          : gfx::Insets(kBorderThicknessDip);
  const gfx::Insets insets = GetInsets();
  const gfx::Insets shadow_insets = insets - border_insets;
  // TODO(dfried): Collapse border into visible arrow where applicable.
  contents_bounds.Inset(-border_insets);
  DCHECK(!avoid_shadow_overlap_ || !visible_arrow_);

  // If |avoid_shadow_overlap_| is true, the shadow part of the inset is also
  // applied now, to ensure that the shadow itself doesn't overlap the anchor.
  if (avoid_shadow_overlap_)
    contents_bounds.Inset(-shadow_insets);

  // Adjust the contents to align with the arrow. The `anchor_point` is the
  // point on `anchor_rect` to offset from; it is also used as part of the
  // visible arrow calculation if present.
  gfx::Point anchor_point =
      GetArrowAnchorPointFromAnchorRect(arrow_, anchor_rect);

  contents_bounds += GetContentBoundsOffsetToArrowAnchorPoint(
      contents_bounds, arrow_, anchor_point);

  // With NO_SHADOW, there should be further insets, but the same logic is
  // used to position the bubble origin according to |anchor_rect|.
  DCHECK((shadow_ != NO_SHADOW && shadow_ != NO_SHADOW_LEGACY) ||
         insets_.has_value() || shadow_insets.IsEmpty() || visible_arrow_);
  if (!avoid_shadow_overlap_)
    contents_bounds.Inset(-shadow_insets);

  // |arrow_offset_| is used to adjust bubbles that would normally be
  // partially offscreen.
  if (is_arrow_on_horizontal(arrow_))
    contents_bounds += gfx::Vector2d(-arrow_offset_, 0);
  else
    contents_bounds += gfx::Vector2d(0, -arrow_offset_);

  // If no visible arrow is shown, return the content bounds.
  if (!visible_arrow_)
    return contents_bounds;

  // Finally, get the needed movement vector of |contents_bounds| to create the
  // space needed to place the visible arrow. adjustments because we don't want
  // the positioning to be altered. Offset by the size of the arrow's inset on
  // each side (only one side will be nonzero) to create space for the visible
  // arrow.
  contents_bounds +=
      GetContentsBoundsOffsetToPlaceVisibleArrow(contents_bounds);

  // We have an anchor point which is appropriate for the arrow type, but
  // when anchoring to a small view it looks better to track from the middle
  // of the view rather than a corner. We may still adjust this point if
  // it's too close to the edge of the bubble (in this case by adjusting the
  // bubble by a few pixels rather than the anchor point).
  const gfx::Point anchor_center = anchor_rect.CenterPoint();
  const gfx::Point contents_center = contents_bounds.CenterPoint();
  if (IsVerticalArrow(arrow_)) {
    const int right_bound =
        contents_bounds.right() -
        (kVisibleArrowBuffer + kVisibleArrowRadius + shadow_insets.right());
    const int left_bound = contents_bounds.x() + kVisibleArrowBuffer +
                           kVisibleArrowRadius + shadow_insets.left();
    if (anchor_point.x() > anchor_center.x() &&
        anchor_center.x() > contents_center.x()) {
      anchor_point.set_x(anchor_center.x());
    } else if (anchor_point.x() > right_bound) {
      anchor_point.set_x(std::max(anchor_rect.x(), right_bound));
    } else if (anchor_point.x() < anchor_center.x() &&
               anchor_center.x() < contents_center.x()) {
      anchor_point.set_x(anchor_center.x());
    } else if (anchor_point.x() < left_bound) {
      anchor_point.set_x(std::min(anchor_rect.right(), left_bound));
    }
    if (anchor_point.x() < left_bound) {
      contents_bounds -= gfx::Vector2d(left_bound - anchor_point.x(), 0);
    } else if (anchor_point.x() > right_bound) {
      contents_bounds += gfx::Vector2d(anchor_point.x() - right_bound, 0);
    }
  } else {
    const int bottom_bound =
        contents_bounds.bottom() -
        (kVisibleArrowBuffer + kVisibleArrowRadius + shadow_insets.bottom());
    const int top_bound = contents_bounds.y() + kVisibleArrowBuffer +
                          kVisibleArrowRadius + shadow_insets.top();
    if (anchor_point.y() > anchor_center.y() &&
        anchor_center.y() > contents_center.y()) {
      anchor_point.set_y(anchor_center.y());
    } else if (anchor_point.y() > bottom_bound) {
      anchor_point.set_y(std::max(anchor_rect.y(), bottom_bound));
    } else if (anchor_point.y() < anchor_center.y() &&
               anchor_center.y() < contents_center.y()) {
      anchor_point.set_y(anchor_center.y());
    } else if (anchor_point.y() < top_bound) {
      anchor_point.set_y(std::min(anchor_rect.bottom(), top_bound));
    }
    if (anchor_point.y() < top_bound) {
      contents_bounds -= gfx::Vector2d(0, top_bound - anchor_point.y());
    } else if (anchor_point.y() > bottom_bound) {
      contents_bounds += gfx::Vector2d(0, anchor_point.y() - bottom_bound);
    }
  }

  CalculateVisibleArrowRect(contents_bounds, anchor_point);

  return contents_bounds;
}

gfx::Vector2d BubbleBorder::GetContentsBoundsOffsetToPlaceVisibleArrow(
    const gfx::Rect& contents_bounds) const {
  if (!visible_arrow_)
    return gfx::Vector2d();

  const gfx::Insets visible_arrow_insets =
      GetVisibleArrowInsets(arrow_, /*include_gap=*/true);
  // Since the arrow is placed on one specific side, only one side of the inset
  // will contribute.
  return gfx::Vector2d(
      visible_arrow_insets.left() - visible_arrow_insets.right(),
      visible_arrow_insets.top() - visible_arrow_insets.bottom());
}

// static
bool BubbleBorder::IsVerticalArrow(BubbleBorder::Arrow arrow) {
  const BubbleArrowSide side = GetBubbleArrowSide(arrow);
  return side == BubbleArrowSide::kTop || side == BubbleArrowSide::kBottom;
}

void BubbleBorder::Paint(const views::View& view, gfx::Canvas* canvas) {
  if (shadow_ == NO_SHADOW) {
    PaintNoShadow(view, canvas);
    return;
  }

  if (shadow_ == NO_SHADOW_LEGACY) {
    PaintNoShadowLegacy(view, canvas);
  } else {
    gfx::ScopedCanvas scoped(canvas);
    SkRRect r_rect = GetClientRect(view);
    canvas->sk_canvas()->clipRRect(r_rect, SkClipOp::kDifference,
                                   true /*doAntiAlias*/);
    DrawBorderAndShadowImpl(r_rect, &cc::PaintCanvas::drawRRect, canvas,
                            view.GetColorProvider(), md_shadow_elevation_);
  }

  if (visible_arrow_)
    PaintVisibleArrow(view, canvas);
}

// static
void BubbleBorder::DrawBorderAndShadow(
    SkRect rect,
    gfx::Canvas* canvas,
    const ui::ColorProvider* color_provider) {
  DrawBorderAndShadowImpl(rect, &cc::PaintCanvas::drawRect, canvas,
                          color_provider);
}

gfx::Insets BubbleBorder::GetInsets() const {
  // Visible arrow is not compatible with OS-drawn shadow or with manual insets.
  DCHECK((!insets_ && shadow_ != NO_SHADOW) || !visible_arrow_);
  if (insets_.has_value())
    return insets_.value();
  gfx::Insets insets;
  if (shadow_ == NO_SHADOW_LEGACY)
    insets = gfx::Insets(kBorderThicknessDip);
  else if (shadow_ == STANDARD_SHADOW)
    insets = GetBorderAndShadowInsets(md_shadow_elevation_);
  if (visible_arrow_) {
    const gfx::Insets arrow_insets = GetVisibleArrowInsets(arrow_, false);
    insets = gfx::Insets(std::max(insets.top(), arrow_insets.top()),
                         std::max(insets.left(), arrow_insets.left()),
                         std::max(insets.bottom(), arrow_insets.bottom()),
                         std::max(insets.right(), arrow_insets.right()));
  }
  return insets;
}

gfx::Size BubbleBorder::GetMinimumSize() const {
  return GetSizeForContentsSize(gfx::Size());
}

gfx::Size BubbleBorder::GetSizeForContentsSize(
    const gfx::Size& contents_size) const {
  // Enlarge the contents size by the thickness of the border images.
  gfx::Size size(contents_size);
  const gfx::Insets insets = GetInsets();
  size.Enlarge(insets.width(), insets.height());
  return size;
}

void BubbleBorder::CalculateVisibleArrowRect(
    const gfx::Rect& contents_bounds,
    const gfx::Point& anchor_point) const {
  const gfx::Insets insets = GetInsets();

  gfx::Point new_origin;
  switch (GetBubbleArrowSide(arrow_)) {
    case BubbleArrowSide::kTop:
      new_origin =
          gfx::Point(anchor_point.x() - kVisibleArrowRadius + 1,
                     contents_bounds.y() + insets.top() - kVisibleArrowLength);
      break;

    case BubbleArrowSide::kBottom:
      new_origin = gfx::Point(anchor_point.x() - kVisibleArrowRadius + 1,
                              contents_bounds.bottom() - insets.bottom());
      break;

    case BubbleArrowSide::kRight:
      new_origin = gfx::Point(contents_bounds.right() - insets.right(),
                              anchor_point.y() - kVisibleArrowRadius + 1);
      break;

    case BubbleArrowSide::kLeft:
      new_origin =
          gfx::Point(contents_bounds.x() + insets.left() - kVisibleArrowLength,
                     anchor_point.y() - kVisibleArrowRadius + 1);
      break;
  }
  visible_arrow_rect_.set_origin(new_origin);
  visible_arrow_rect_.set_size(GetVisibleArrowSize(arrow_));
}

SkRRect BubbleBorder::GetClientRect(const View& view) const {
  gfx::RectF bounds(view.GetLocalBounds());
  bounds.Inset(GetInsets());
  return SkRRect::MakeRectXY(gfx::RectFToSkRect(bounds), corner_radius(),
                             corner_radius());
}

void BubbleBorder::PaintNoShadow(const View& view, gfx::Canvas* canvas) {
  gfx::ScopedCanvas scoped(canvas);
  canvas->sk_canvas()->clipRRect(GetClientRect(view), SkClipOp::kDifference,
                                 true /*doAntiAlias*/);
  canvas->sk_canvas()->drawColor(SK_ColorTRANSPARENT, SkBlendMode::kSrc);
}

void BubbleBorder::PaintNoShadowLegacy(const View& view, gfx::Canvas* canvas) {
  gfx::RectF bounds(view.GetLocalBounds());
  bounds.Inset(gfx::InsetsF(kBorderThicknessDip / 2.0f));
  cc::PaintFlags flags;
  flags.setAntiAlias(true);
  flags.setStyle(cc::PaintFlags::kStroke_Style);
  flags.setStrokeWidth(kBorderThicknessDip);
  SkColor kBorderColor =
      view.GetColorProvider()->GetColor(ui::kColorBubbleBorder);
  flags.setColor(kBorderColor);
  canvas->DrawRoundRect(bounds, corner_radius(), flags);
}

void BubbleBorder::PaintVisibleArrow(const View& view, gfx::Canvas* canvas) {
  gfx::Point arrow_origin = visible_arrow_rect_.origin();
  View::ConvertPointFromScreen(&view, &arrow_origin);
  const gfx::Rect arrow_bounds(arrow_origin, visible_arrow_rect_.size());

  // Clip the canvas to a box that's big enough to hold the shadow in every
  // dimension but won't overlap the bubble itself.
  gfx::ScopedCanvas scoped(canvas);
  gfx::Rect clip_rect = arrow_bounds;
  const BubbleArrowSide side = GetBubbleArrowSide(arrow_);
  clip_rect.Inset(side == BubbleArrowSide::kRight ? 0 : -2,
                  side == BubbleArrowSide::kBottom ? 0 : -2,
                  side == BubbleArrowSide::kLeft ? 0 : -2,
                  side == BubbleArrowSide::kTop ? 0 : -2);
  canvas->ClipRect(clip_rect);

  cc::PaintFlags flags;
  flags.setStrokeCap(cc::PaintFlags::kRound_Cap);

  flags.setColor(view.GetColorProvider()->GetColor(
      ui::kColorBubbleBorderWhenShadowPresent));
  flags.setStyle(cc::PaintFlags::kStroke_Style);
  flags.setStrokeWidth(1.2);
  flags.setAntiAlias(true);
  canvas->DrawPath(
      GetVisibleArrowPath(arrow_, arrow_bounds, BubbleArrowPart::kBorder),
      flags);

  flags.setColor(background_color());
  flags.setStyle(cc::PaintFlags::kFill_Style);
  flags.setStrokeWidth(1.0);
  flags.setAntiAlias(true);
  canvas->DrawPath(
      GetVisibleArrowPath(arrow_, arrow_bounds, BubbleArrowPart::kFill), flags);
}

void BubbleBackground::Paint(gfx::Canvas* canvas, views::View* view) const {
  // Fill the contents with a round-rect region to match the border images.
  cc::PaintFlags flags;
  flags.setAntiAlias(true);
  flags.setStyle(cc::PaintFlags::kFill_Style);
  flags.setColor(border_->background_color());
  gfx::RectF bounds(view->GetLocalBounds());
  bounds.Inset(gfx::InsetsF(border_->GetInsets()));

  canvas->DrawRoundRect(bounds, border_->corner_radius(), flags);
}

}  // namespace views
