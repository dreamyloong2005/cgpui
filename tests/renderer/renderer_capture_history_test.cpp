#include "renderer_pixel_test_support.hpp"
#include "../support/frame_wait_test_support.hpp"

#include <iostream>

namespace {
bool same_capture(const cgpui::RendererFramePixels* actual,
                  const cgpui::RendererFramePixels& expected) {
  return actual && actual->valid() && actual->width == expected.width &&
      actual->height == expected.height && actual->encoding == expected.encoding &&
      actual->rgba8 == expected.rgba8;
}
int fail(int code, const char* message) {
  std::cerr << "capture history: " << message << '\n';
  return code;
}
bool present_color(cgpui::Renderer& renderer, cgpui::Color color, bool capture) {
  auto frame = cgpui_test::wait_for_test_frame(renderer);
  if (!frame) {
    cgpui_test::report_pixel_test_error("begin history frame", frame.error());
    return false;
  }
  (*frame)->clear(color);
  (*frame)->draw_rect({.rect = {{4, 4}, {12, 12}}, .color = {1, 1, 0, 1}});
  if (capture && !cgpui_test::pixel_test_ok((*frame)->request_pixel_capture(), "request history capture"))
    return false;
  return cgpui_test::pixel_test_ok((*frame)->present(), "history present");
}
bool reject_capture(cgpui::Renderer& renderer) {
  auto frame = cgpui_test::begin_pixel_capture(renderer);
  if (!frame) return false;
  (*frame)->clear({0, 1, 0, 1});
  const cgpui::ImageAsset invalid{
      .id = {202611243}, .logical_size = {1, 1},
      .bitmap = {.width = 1, .height = 1, .stride = 0,
          .format = cgpui::ImageFormat::rgba8_unorm, .pixels = {255, 255, 255, 255}},
  };
  (*frame)->upload_image(invalid);
  const auto result = (*frame)->present(); // No retry; backend error codes need not match.
  return !result && result.error().code != cgpui::ErrorCode::frame_retry_required &&
      result.error().code != cgpui::ErrorCode::frame_not_ready;
}
bool pattern(const cgpui::RendererFramePixels& pixels, std::array<std::uint8_t, 4> clear) {
  return pixels.valid() && pixels.width == 64 && pixels.height == 64 &&
      cgpui_test::pixel_near(pixels, 8, 8, {255, 255, 0, 255}) &&
      cgpui_test::pixel_near(pixels, 32, 32, clear);
}
}

int main() {
  cgpui_test::RendererPixelTestWindow window(
      L"CGPUIRendererCaptureHistoryTest", L"CGPUI Capture History Contract");
  if (!window.valid()) return fail(1, "window unavailable");
  auto renderer = cgpui_test::create_pixel_test_renderer({.native_surface = window.surface(),
      .framebuffer_size = window.framebuffer_size(), .scale = {1.0F}});
  if (!renderer) return fail(2, "selected backend unavailable");
  const auto generation = (*renderer)->backend_info().device_generation;
  if ((*renderer)->last_frame_pixels()) return fail(3, "fresh renderer has a capture");
  if (!present_color(**renderer, {0, 0, 1, 1}, false) || (*renderer)->last_frame_pixels())
    return fail(4, "uncaptured cold frame published pixels");
  if (!reject_capture(**renderer) || (*renderer)->last_frame_pixels() ||
      (*renderer)->backend_info().device_generation != generation)
    return fail(5, "failed cold capture published pixels or changed the stable device");
  if (!present_color(**renderer, {1, 0, 0, 1}, true)) return fail(6, "initial successful capture failed");
  const auto red = cgpui_test::completed_pixel_capture(**renderer); // Deep owned value, not borrowed pointer.
  if (!red || !pattern(*red, {255, 0, 0, 255})) return fail(7, "initial captured pattern differs");
  if (!present_color(**renderer, {0, 1, 0, 1}, false) ||
      !same_capture((*renderer)->last_frame_pixels(), *red))
    return fail(8, "uncaptured successful frame discarded/replaced last successful capture");
  if (!reject_capture(**renderer) || !same_capture((*renderer)->last_frame_pixels(), *red) ||
      (*renderer)->backend_info().device_generation != generation)
    return fail(9, "failed requested frame discarded/replaced last successful capture");
  {
    auto abandoned = cgpui_test::begin_pixel_capture(**renderer);
    if (!abandoned) return fail(10, "abandoned capture frame unavailable");
    (*abandoned)->clear({0, 1, 1, 1});
  }
  if (!same_capture((*renderer)->last_frame_pixels(), *red))
    return fail(11, "abandoned request changed successful history");
  if (!present_color(**renderer, {0, 0, 1, 1}, true)) return fail(12, "replacement successful capture failed");
  const auto blue = cgpui_test::completed_pixel_capture(**renderer);
  if (!blue || !pattern(*blue, {0, 0, 255, 255}) || blue->rgba8 == red->rgba8)
    return fail(13, "new successful capture did not replace old pixels");
  if (!present_color(**renderer, {0, 1, 0, 1}, false) ||
      !same_capture((*renderer)->last_frame_pixels(), *blue) ||
      (*renderer)->backend_info().device_generation != generation)
    return fail(14, "latest replacement was lost without surface/device change");
  return 0;
}
