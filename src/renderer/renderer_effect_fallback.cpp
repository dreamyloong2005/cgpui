#include "cgpui/renderer/renderer.hpp"

namespace cgpui {

// Backends without backdrop blur keep the overlay readable by drawing the
// tint as a plain rounded rect.
void RenderFrame::draw_backdrop_blur(const BackdropBlurDraw& backdrop) {
  if (!backdrop.tint.has_value()) {
    return;
  }
  draw_rounded_rect(RoundedRectDraw{
      .rect = backdrop.rect,
      .color = *backdrop.tint,
      .radius = backdrop.radius,
      .clip_rect = backdrop.clip_rect,
      .clip_stack = backdrop.clip_stack,
      .composition_stack = backdrop.composition_stack,
      .metadata = backdrop.metadata,
  });
}

RendererEffectSupport Renderer::backdrop_blur_support() const {
  return RendererEffectSupport{
      .supported = false,
      .reason = "renderer does not implement backdrop blur; tint only",
  };
}

} // namespace cgpui
