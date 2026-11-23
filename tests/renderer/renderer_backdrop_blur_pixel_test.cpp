#include "renderer_pixel_test_support.hpp"

#include <cmath>
#include <cstdio>
#include <functional>

// Backdrop blur on a real swapchain over 2 px black/white stripes (period
// 4 px). A Gaussian with sigma 4 attenuates that period by exp(-2 pi^2
// sigma^2 / 16) ~ 3e-9, so blurred pixels are the stripe mean (127.5 in
// sRGB-encoded bytes, which are what is blended) and pixels outside the
// rounded rect keep 0 or 255.
namespace {

constexpr cgpui::Color white{.r = 1.0F, .g = 1.0F, .b = 1.0F, .a = 1.0F};
constexpr cgpui::Color black{.a = 1.0F};
constexpr cgpui::Rect panel{.origin = {32, 32}, .size = {64, 64}};

struct Capture {
  cgpui::RendererFramePixels pixels;
  cgpui::RendererBackdropBlurDiagnostics backdrop;
  cgpui::RendererDrawCounts submitted;
};

void stripes(cgpui::RenderFrame& frame) {
  for (int x = 0; x < 128; x += 4) {
    frame.draw_rect(cgpui::SolidRect{
        .rect = {.origin = {static_cast<float>(x), 0}, .size = {2, 128}},
        .color = black});
  }
}

cgpui::BackdropBlurDraw backdrop(cgpui::Rect rect, std::optional<cgpui::Color> tint = {}) {
  return cgpui::BackdropBlurDraw{
      .rect = rect,
      .radius = cgpui::BorderRadii::all(12.0F),
      .blur_radius = 4.0F,
      .tint = tint,
  };
}

// Renders `frames` frames on one renderer and returns the last one.
std::optional<Capture> capture(
    cgpui_test::RendererPixelTestWindow& window, float scale, int frames,
    const std::function<void(cgpui::RenderFrame&)>& paint) {
  auto renderer = cgpui_test::create_pixel_test_renderer(cgpui::RenderSurfaceDescriptor{
      .native_surface = window.surface(),
      .framebuffer_size = window.framebuffer_size(),
      .scale = cgpui::DpiScale{scale},
  });
  if (!renderer) return std::nullopt;
  if (!(*renderer)->backdrop_blur_support().supported) {
    cgpui_test::report_pixel_test_error("backdrop capability",
        {cgpui::ErrorCode::unsupported_platform, (*renderer)->backdrop_blur_support().reason});
    return std::nullopt;
  }
  for (int index = 0; index < frames; ++index) {
    auto frame = cgpui_test::begin_pixel_capture(**renderer);
    if (!frame) return std::nullopt;
    (*frame)->clear(white);
    paint(**frame);
    if (!cgpui_test::pixel_test_ok((*frame)->present(), "present")) return std::nullopt;
  }
  const auto* pixels = (*renderer)->last_frame_pixels();
  const auto* snapshot = (*renderer)->last_frame_diagnostic_snapshot();
  if (pixels == nullptr || snapshot == nullptr) return std::nullopt;
  return Capture{*pixels, snapshot->backdrop_blur, snapshot->submitted_draws};
}

int channel(const Capture& capture, std::uint32_t x, std::uint32_t y,
            std::size_t index = 0) {
  const auto pixel = capture.pixels.pixel_rgba8(x, y);
  return pixel ? static_cast<int>((*pixel)[index]) : -1;
}

bool expect(const Capture& capture, std::uint32_t x, std::uint32_t y,
            int expected, int tolerance, std::size_t index = 0) {
  const int actual = channel(capture, x, y, index);
  if (std::abs(actual - expected) > tolerance) {
    std::fprintf(stderr, "pixel %u,%u[%zu] got %d expected %d\n", x, y, index,
                 actual, expected);
    return false;
  }
  return true;
}

// Stripe value of an unblurred pixel at device x for a given scale.
int stripe(std::uint32_t x, float scale) {
  const float logical = (static_cast<float>(x) + 0.5F) / scale;
  return static_cast<int>(logical) % 4 < 2 ? 0 : 255;
}

// A frame without backdrops takes the direct swapchain path.
int test_no_backdrop(cgpui_test::RendererPixelTestWindow& window) {
  const auto result = capture(window, 1.0F, 1, stripes);
  if (!result) return 1;
  return result->backdrop.offscreen_pass_count == 0 &&
                 result->backdrop.blurred_backdrop_count == 0 &&
                 !result->backdrop.offscreen_targets_allocated &&
                 expect(*result, 64, 64, stripe(64, 1.0F), 0) &&
                 expect(*result, 66, 64, stripe(66, 1.0F), 0)
             ? 0
             : 2;
}

// Inside is smoothed, outside and the cut-off rounded corner are exact.
int test_smoothing(cgpui_test::RendererPixelTestWindow& window) {
  const auto result = capture(window, 1.0F, 2, [](cgpui::RenderFrame& frame) {
    stripes(frame);
    frame.draw_backdrop_blur(backdrop(panel));
  });
  if (!result) return 10;
  for (std::uint32_t x = 40; x < 88; ++x) {
    if (!expect(*result, x, 64, 128, 4)) return 11;
  }
  for (const std::uint32_t x : {10U, 11U, 12U, 13U, 110U, 111U}) {
    if (!expect(*result, x, 64, stripe(x, 1.0F), 0)) return 12;
  }
  for (const std::uint32_t x : {64U, 65U, 66U, 67U}) {
    if (!expect(*result, x, 20, stripe(x, 1.0F), 0)) return 13;
  }
  if (!expect(*result, 33, 33, stripe(33, 1.0F), 0) ||
      !expect(*result, 34, 34, stripe(34, 1.0F), 0) ||
      !expect(*result, 64, 33, 128, 8)) {
    return 14;
  }
  return result->backdrop.blurred_backdrop_count == 1 &&
                 result->backdrop.offscreen_targets_allocated &&
                 result->submitted.backdrop_blur_count == 1
             ? 0
             : 15;
}

// The tint mixes by its alpha; a fill drawn after lands on top.
int test_tint_and_order(cgpui_test::RendererPixelTestWindow& window) {
  const auto result = capture(window, 1.0F, 1, [](cgpui::RenderFrame& frame) {
    stripes(frame);
    frame.draw_backdrop_blur(
        backdrop(panel, cgpui::Color{.r = 1.0F, .a = 0.5F}));
    frame.draw_rect(cgpui::SolidRect{
        .rect = {.origin = {56, 56}, .size = {8, 8}},
        .color = {.g = 1.0F, .a = 1.0F}});
  });
  if (!result) return 20;
  return expect(*result, 44, 70, 192, 5, 0) &&
                 expect(*result, 44, 70, 64, 5, 1) &&
                 expect(*result, 44, 70, 64, 5, 2) &&
                 expect(*result, 60, 60, 0, 0, 0) &&
                 expect(*result, 60, 60, 255, 0, 1)
             ? 0
             : 21;
}

// Each backdrop blurs everything drawn before it, including content drawn
// after an earlier backdrop.
int test_multiple(cgpui_test::RendererPixelTestWindow& window) {
  const auto result = capture(window, 1.0F, 1, [](cgpui::RenderFrame& frame) {
    stripes(frame);
    frame.draw_backdrop_blur(backdrop({.origin = {8, 8}, .size = {48, 48}}));
    frame.draw_rect(cgpui::SolidRect{
        .rect = {.origin = {88, 88}, .size = {16, 16}}, .color = black});
    frame.draw_backdrop_blur(backdrop({.origin = {72, 72}, .size = {48, 48}}));
  });
  if (!result) return 30;
  return expect(*result, 32, 32, 128, 4) && channel(*result, 96, 96) < 40 &&
                 channel(*result, 96, 96) > 0 &&
                 result->backdrop.blurred_backdrop_count == 2 &&
                 result->submitted.backdrop_blur_count == 2
             ? 0
             : 31;
}

// Blur radius and rect are logical pixels.
int test_scale(cgpui_test::RendererPixelTestWindow& window) {
  const auto result = capture(window, 1.5F, 1, [](cgpui::RenderFrame& frame) {
    stripes(frame);
    frame.draw_backdrop_blur(backdrop(panel));
  });
  if (!result) return 40;
  // Stay 3 sigma (18 device px) away from the frame edge, where the blur
  // repeats the last column.
  for (std::uint32_t x = 60; x < 110; ++x) {
    if (!expect(*result, x, 96, 128, 4)) return 41;
  }
  for (const std::uint32_t x : {20U, 21U, 22U, 23U, 24U, 25U}) {
    if (!expect(*result, x, 96, stripe(x, 1.5F), 0)) return 42;
  }
  return 0;
}

} // namespace

int main() {
  cgpui_test::RendererPixelTestWindow window(
      L"CGPUIRendererBackdropBlurPixelTestWindow", L"CGPUI Backdrop Blur Test");
  (void)window.resize_client(128, 128);
  for (const int result :
       {test_no_backdrop(window), test_smoothing(window),
        test_tint_and_order(window), test_multiple(window), test_scale(window)}) {
    if (result != 0) return result;
  }
  return 0;
}
