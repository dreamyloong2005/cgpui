#include "renderer_pixel_test_support.hpp"
#include "../support/frame_wait_test_support.hpp"

namespace cgpui_test {
cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_pixel_capture(cgpui::Renderer& renderer) {
  auto frame = wait_for_test_frame(renderer);
  if (!frame) {
    report_pixel_test_error("begin frame", frame.error());
    return std::unexpected(frame.error());
  }
  if (!*frame) {
    const cgpui::Error error{cgpui::ErrorCode::frame_acquisition_failed,
                            "pixel test begin_frame returned a null frame"};
    report_pixel_test_error("begin frame", error);
    return std::unexpected(error);
  }
  if (auto requested = (*frame)->request_pixel_capture(); !requested) {
    report_pixel_test_error("request capture", requested.error());
    return std::unexpected(requested.error());
  }
  return frame;
}
std::optional<cgpui::RendererFramePixels> completed_pixel_capture(cgpui::Renderer& renderer) {
  const auto* pixels = renderer.last_frame_pixels();
  if (!pixels || !pixels->valid()) {
    report_pixel_test_error("read capture", {cgpui::ErrorCode::renderer_initialization_failed,
                                            "successful present returned no valid pixel capture"});
    return std::nullopt;
  }
  return *pixels;
}
} // namespace cgpui_test
