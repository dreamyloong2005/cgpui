#include "cgpui/renderer/renderer.hpp"

#include <limits>

namespace cgpui {

namespace {

std::optional<std::size_t> pixel_byte_size(
    std::uint32_t width,
    std::uint32_t height) {
  constexpr std::size_t channel_count = 4;
  const std::size_t row_byte_count =
      static_cast<std::size_t>(width) * channel_count;
  if (height != 0 &&
      row_byte_count > std::numeric_limits<std::size_t>::max() / height) {
    return std::nullopt;
  }
  return row_byte_count * height;
}

} // namespace

bool RendererFramePixels::valid() const {
  const auto expected_size = pixel_byte_size(width, height);
  return width != 0 && height != 0 && expected_size.has_value() &&
         rgba8.size() == *expected_size;
}

std::size_t RendererFramePixels::row_byte_count() const {
  return static_cast<std::size_t>(width) * 4;
}

std::optional<std::array<std::uint8_t, 4>>
RendererFramePixels::pixel_rgba8(std::uint32_t x, std::uint32_t y) const {
  if (!valid() || x >= width || y >= height) {
    return std::nullopt;
  }
  const std::size_t offset =
      static_cast<std::size_t>(y) * row_byte_count() +
      static_cast<std::size_t>(x) * 4;
  return std::array<std::uint8_t, 4>{
      rgba8[offset], rgba8[offset + 1], rgba8[offset + 2], rgba8[offset + 3]};
}

Result<void> RenderFrame::request_pixel_capture() {
  return std::unexpected(Error{
      .code = ErrorCode::unsupported_platform,
      .message = "renderer frame pixel capture is unavailable",
  });
}

const RendererFramePixels* Renderer::last_frame_pixels() const {
  return nullptr;
}

} // namespace cgpui
