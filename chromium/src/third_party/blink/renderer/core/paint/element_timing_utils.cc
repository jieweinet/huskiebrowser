// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/paint/element_timing_utils.h"

#include "third_party/blink/renderer/core/frame/web_frame_widget_impl.h"
#include "third_party/blink/renderer/core/frame/web_local_frame_impl.h"
#include "third_party/blink/renderer/core/layout/layout_object.h"
#include "third_party/blink/renderer/core/layout/layout_view.h"
#include "third_party/blink/renderer/platform/graphics/paint/geometry_mapper.h"
#include "third_party/blink/renderer/platform/graphics/paint/property_tree_state.h"

namespace blink {

// static
FloatRect ElementTimingUtils::ComputeIntersectionRect(
    LocalFrame* frame,
    const IntRect& int_visual_rect,
    const PropertyTreeStateOrAlias& current_paint_chunk_properties) {
  // Compute the visible part of the image rect.
  FloatClipRect visual_rect =
      FloatClipRect(ToGfxRectF(FloatRect(int_visual_rect)));
  GeometryMapper::LocalToAncestorVisualRect(current_paint_chunk_properties,
                                            frame->View()
                                                ->GetLayoutView()
                                                ->FirstFragment()
                                                .LocalBorderBoxProperties(),
                                            visual_rect);
  WebFrameWidgetImpl* widget =
      WebLocalFrameImpl::FromFrame(frame)->LocalRootFrameWidget();
  DCHECK(widget);
  return FloatRect(widget->BlinkSpaceToDIPs(ToGfxRectF(visual_rect.Rect())));
}

}  // namespace blink
