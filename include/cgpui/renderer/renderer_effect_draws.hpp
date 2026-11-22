#pragma once

#include "cgpui/renderer/renderer_types.hpp"
#include "cgpui/core/paint_values.hpp"

#include <cstddef>
#include <optional>
#include <string>

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

// Blurs what the frame drew before this command inside the rounded rect,
// then mixes `tint` over it (tint alpha = mix amount). `blur_radius` is the
// Gaussian standard deviation in logical pixels, like CSS
// `backdrop-filter: blur()`. Draws after it land on top. Without renderer
// support only the tint is drawn.
struct BackdropBlurDraw {
  Rect rect;
  BorderRadii radius;
  float blur_radius = 0.0F;
  std::optional<Color> tint;
  std::optional<Rect> clip_rect;
  RendererClipStackRecord clip_stack;
  RendererCompositionStackRecord composition_stack;
  PaintMetadata metadata;
};

struct RendererEffectSupport {
  bool supported = false;
  std::string reason;
};

struct RendererBackdropBlurDiagnostics {
  std::size_t blurred_backdrop_count = 0;
  std::size_t offscreen_pass_count = 0;
  bool offscreen_targets_allocated = false;
};

} // namespace cgpui
