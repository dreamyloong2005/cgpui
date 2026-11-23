#pragma once

#include "dx12_texture_internal.hpp"
#include "cgpui/renderer/planning/rect_geometry.hpp"
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
using Dx12DrawCommand = std::variant<RectGeometry, Dx12TexturedDraw>;

bool dx12_finite_textured_draw(const Dx12TexturedDraw& draw);
Result<void> dx12_record_textured_draw(Dx12State& state, const Dx12TexturedDraw& draw);
} // namespace cgpui
