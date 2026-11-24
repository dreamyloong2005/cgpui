#pragma once

#include "dx12_texture_internal.hpp"
#include "dx12_image_request_internal.hpp"
#include "cgpui/renderer/planning/rect_geometry.hpp"
#include "cgpui/renderer/planning/box_shadow_geometry.hpp"
#include "cgpui/renderer/planning/backdrop_blur_plan.hpp"
#include <array>
#include <optional>
#include <variant>
#include <vector>

namespace cgpui {
struct Dx12TextureVertex {
  std::array<float, 2> position;
  std::array<float, 2> uv;
  std::array<float, 4> color;
};
struct Dx12TexturedDraw {
  Dx12TextureHandle texture;
  std::vector<Dx12TextureVertex> vertices;
  std::optional<Rect> clip_rect;
  ImageSamplingMode sampling = ImageSamplingMode::linear;
  bool glyph = false;
};
using Dx12DrawPayload = std::variant<RectGeometry, Dx12TexturedDraw, BoxShadowGeometry,
                                     BackdropBlurPlan, Dx12PendingImageDraw>;
struct Dx12DrawCommand {
  Dx12DrawPayload payload;
  RendererPrimitiveKind kind = RendererPrimitiveKind::solid_rect;
  std::size_t command_index = 0;
  std::size_t resource_offset = 0;
};

Result<void> dx12_record_draw(Dx12State& state, const Dx12DrawCommand& draw);
bool dx12_finite_textured_draw(const Dx12TexturedDraw& draw);
Result<void> dx12_record_textured_draw(Dx12State& state, const Dx12TexturedDraw& draw);
} // namespace cgpui
