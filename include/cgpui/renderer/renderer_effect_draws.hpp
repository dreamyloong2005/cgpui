#pragma once

#include "cgpui/renderer/renderer_types.hpp"
#include "cgpui/ui/style_values.hpp"

#include <optional>

namespace cgpui {

// Outer (drop) shadow of the rounded rect `rect` with `radius`, following
// CSS box-shadow: the shadow shape is `rect` moved by `shadow.offset` and
// grown by `shadow.spread_radius`; `shadow.blur_radius` is the CSS blur
// radius, a Gaussian with sigma = blur_radius / 2. The shadow is clipped
// out inside `rect`, so a translucent box does not show its own shadow.
// All lengths are logical pixels.
struct BoxShadowDraw {
  Rect rect;
  BorderRadii radius;
  BoxShadow shadow;
  std::optional<Rect> clip_rect;
  RendererClipStackRecord clip_stack;
  RendererCompositionStackRecord composition_stack;
  PaintMetadata metadata;
};

} // namespace cgpui
