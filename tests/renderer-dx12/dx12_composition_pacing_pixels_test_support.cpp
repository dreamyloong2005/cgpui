#include "dx12_composition_pacing_test_support.hpp"
#include "dx12_pacing_native_test_support.hpp"

namespace cgpui_test {
bool composition_pacing_capture(cgpui::RenderFrame& frame, cgpui::Renderer& renderer,
                                bool blue, unsigned extent) {
  using namespace cgpui;
  frame.clear({0, 0, 0, 0});
  frame.draw_rect({.rect = {{0, 0}, {20, static_cast<float>(extent)}},
      .color = blue ? Color{0, 0, 1, 0.5F} : Color{1, 0, 0, 0.5F}});
  frame.draw_rect({.rect = {{40, 0}, {static_cast<float>(extent - 40), static_cast<float>(extent)}},
      .color = blue ? Color{0, 0, 1, 1} : Color{1, 0, 0, 1}});
  if (!frame.request_pixel_capture()) return false;
  const auto presented = frame.present(); // One real attempt; no helper retries Present.
  if (!presented) {
    std::cerr << "composition renderer capture: " << presented.error().message << '\n';
    return false;
  }
  const auto* pixels = renderer.last_frame_pixels();
  const std::array<unsigned char, 4> half = blue
      ? std::array<unsigned char, 4>{0, 0, 128, 128} : std::array<unsigned char, 4>{128, 0, 0, 128};
  const std::array<unsigned char, 4> full = blue
      ? std::array<unsigned char, 4>{0, 0, 255, 255} : std::array<unsigned char, 4>{255, 0, 0, 255};
  return pixels && pixels->width == extent && pixels->height == extent &&
      dx12_pixel(pixels, 10, 32, half, 2) && dx12_pixel(pixels, 30, 32, {0, 0, 0, 0}, 2) &&
      dx12_pixel(pixels, 52, 32, full, 2);
}
void composition_pacing_trace(const char* stage, const Dx12PacingForwardApi& api,
                              const cgpui::RendererBackendInfo& info) {
  std::cout << "COMPOSITION_RENDERER stage=" << stage << " adapter=" << info.adapter_name
            << " luid=" << info.adapter_luid.value_or(0) << " generation=" << info.device_generation
            << " software=" << info.software_adapter << " sets=" << api.sets << " gets=" << api.gets
            << " polls=" << api.polls << " ready=" << api.ready << " pending=" << api.pending
            << " closes=" << api.closes << " close_ok=" << api.close_ok << std::endl;
}
} // namespace cgpui_test
