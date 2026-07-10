#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace cgpui {

enum class RendererFramePixelEncoding {
  linear,
  srgb,
};

struct RendererFramePixels {
  std::uint32_t width = 0;
  std::uint32_t height = 0;
  RendererFramePixelEncoding encoding = RendererFramePixelEncoding::linear;
  std::vector<std::uint8_t> rgba8;

  [[nodiscard]] bool valid() const;
  [[nodiscard]] std::size_t row_byte_count() const;
  [[nodiscard]] std::optional<std::array<std::uint8_t, 4>> pixel_rgba8(
      std::uint32_t x,
      std::uint32_t y) const;
};

} // namespace cgpui
