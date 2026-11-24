#include "dx12_image_retention_test_support.hpp"
#include "dx12_test_support.hpp"
#include "../support/frame_wait_test_support.hpp"

namespace cgpui_test {
cgpui::ImageAsset retention_image(std::uint64_t id, std::array<unsigned char, 4> color) {
  return {.id = {id}, .logical_size = {1, 1}, .bitmap = {.width = 1, .height = 1, .stride = 4,
      .pixels = {color[0], color[1], color[2], color[3]}}};
}
bool retention_sample(const cgpui::Renderer& renderer, std::size_t expected,
                      std::optional<std::size_t> uploads) {
  const auto* value = renderer.last_frame_diagnostic_snapshot();
  return value && value->retained_image_byte_count == expected &&
      !value->retained_image_byte_count_saturated &&
      (!uploads || value->upload_bytes.image_byte_count == *uploads);
}
bool retention_present_image(cgpui::Renderer& renderer, cgpui::RenderFrame& frame,
    const cgpui::ImageAssetDescriptor& image, const ImageRetentionExpectation& expected) {
  frame.clear({0, 0, 1, 1});
  frame.draw_image({.bounds = {{0, 0}, {32, 32}}, .asset = image});
  return frame.request_pixel_capture() && frame.present() &&
      dx12_pixel(renderer.last_frame_pixels(), 8, 8, expected.pixel) &&
      dx12_pixel(renderer.last_frame_pixels(), 48, 48, {0, 0, 255, 255}) &&
      retention_sample(renderer, expected.payload, expected.uploads);
}
bool retention_empty_frame(cgpui::Renderer& renderer, std::size_t expected) {
  auto frame = wait_for_test_frame(renderer); // Success only; never used for error/retry assertions.
  if (!frame) return false;
  (*frame)->clear({0, 0, 1, 1});
  return (*frame)->request_pixel_capture() && (*frame)->present() &&
      dx12_pixel(renderer.last_frame_pixels(), 32, 32, {0, 0, 255, 255}) &&
      retention_sample(renderer, expected, 0);
}
} // namespace cgpui_test
