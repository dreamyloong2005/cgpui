#include "renderer_box_shadow_pixel_support.hpp"
#include "renderer_pixel_test_support.hpp"

#include <cstdio>

// Box shadows on a real swapchain. Expected values come from the exact
// Gaussian-blurred rect (std::erf) and a C++ port of the shader formula,
// evaluated at device pixel centers; blur radius 8 means sigma = 4 logical.
namespace {

constexpr cgpui::Color white{.r = 1.0F, .g = 1.0F, .b = 1.0F, .a = 1.0F};
constexpr cgpui::Color black{.a = 1.0F};
constexpr cgpui::Rect box{.origin = {16.0F, 16.0F}, .size = {24.0F, 24.0F}};

cgpui::BoxShadowDraw shadow(float blur, cgpui::BorderRadii radius = {}) {
  return cgpui::BoxShadowDraw{
      .rect = box,
      .radius = radius,
      .shadow = {.color = black, .blur_radius = blur},
  };
}

int expected_byte(double coverage) {
  return static_cast<int>(std::lround(255.0 * (1.0 - coverage)));
}

bool close_to(const cgpui::RendererFramePixels& pixels, std::uint32_t x,
          std::uint32_t y, int expected, int tolerance) {
  const auto actual = pixels.pixel_rgba8(x, y);
  if (!actual || std::abs(static_cast<int>((*actual)[0]) - expected) >
                     tolerance) {
    std::fprintf(stderr, "pixel %u,%u got %d expected %d\n", x, y,
                 actual ? static_cast<int>((*actual)[0]) : -1, expected);
    return false;
  }
  return true;
}

// Falloff left of and above the box, and the clipped-out box interior.
int test_falloff(cgpui_test::RendererPixelTestWindow& window, float scale) {
  auto pixels = cgpui_test::capture_pixels_at_scale(
      window, cgpui::DpiScale{scale}, white,
      [](cgpui::RenderFrame& frame) { frame.draw_box_shadow(shadow(8.0F)); });
  if (!pixels) return 1;
  const double s = scale;
  const double sigma = 4.0 * s;
  const double x0 = 16.0 * s;
  const double x1 = 40.0 * s;
  const auto mid = static_cast<std::uint32_t>(28.0 * s);
  for (std::uint32_t d = 0; d < static_cast<std::uint32_t>(x0); ++d) {
    const double c = d + 0.5;
    const double m = mid + 0.5;
    if (!close_to(*pixels, d, mid,
              expected_byte(cgpui_test::blurred_rect_coverage(
                  c, m, x0, x0, x1, x1, sigma)), 3) ||
        !close_to(*pixels, mid, d,
              expected_byte(cgpui_test::blurred_rect_coverage(
                  m, c, x0, x0, x1, x1, sigma)), 3)) {
      return 2;
    }
  }
  return close_to(*pixels, mid, mid, 255, 0) ? 0 : 3;
}

// Rounded corner falloff against the ported shader formula, at scale 1.5;
// pixels within 1.5 px of the box are skipped (clip-out antialiasing).
int test_rounded_corner(cgpui_test::RendererPixelTestWindow& window) {
  constexpr double s = 1.5;
  auto pixels = cgpui_test::capture_pixels_at_scale(
      window, cgpui::DpiScale{1.5F}, white, [](cgpui::RenderFrame& frame) {
        frame.draw_box_shadow(shadow(8.0F, cgpui::BorderRadii::all(8.0F)));
      });
  if (!pixels) return 10;
  const double x0 = 16.0 * s;
  const double x1 = 40.0 * s;
  const double r = 8.0 * s;
  for (std::uint32_t y = 0; y < static_cast<std::uint32_t>(x0 + r); ++y) {
    for (std::uint32_t x = 0; x < static_cast<std::uint32_t>(x0 + r); ++x) {
      const double px = x + 0.5;
      const double py = y + 0.5;
      const double dx = (std::max)(x0 + r - px, 0.0);
      const double dy = (std::max)(x0 + r - py, 0.0);
      if (std::sqrt(dx * dx + dy * dy) - r < 1.5) continue;
      const double coverage = cgpui_test::rounded_rect_shadow_coverage(
          px, py, x0, x0, x1, x1, r, 4.0 * s);
      if (!close_to(*pixels, x, y, expected_byte(coverage), 3)) return 11;
    }
  }
  return 0;
}

// A clip rect cuts the shadow; pixels outside the clip stay white.
int test_clip(cgpui_test::RendererPixelTestWindow& window) {
  auto pixels = cgpui_test::capture_pixels(
      window, white, [](cgpui::RenderFrame& frame) {
        cgpui::BoxShadowDraw draw = shadow(8.0F);
        draw.clip_rect = cgpui::Rect{.origin = {0, 0}, .size = {28, 64}};
        frame.draw_box_shadow(draw);
      });
  if (!pixels) return 20;
  const double expected =
      cgpui_test::blurred_rect_coverage(10.5, 28.5, 16, 16, 40, 40, 4.0);
  return close_to(*pixels, 10, 28, expected_byte(expected), 3) &&
                 close_to(*pixels, 28, 12, 255, 0) &&
                 close_to(*pixels, 44, 28, 255, 0)
             ? 0
             : 21;
}

// Zero blur equals a solid rect at offset + spread, under the same
// opacity and transform.
int test_zero_blur(cgpui_test::RendererPixelTestWindow& window) {
  const cgpui::PaintMetadata metadata{
      .opacity = 0.5F,
      .transform = cgpui::AffineTransform::translation(4.0F, 6.0F),
  };
  const auto stack = cgpui::renderer_composition_stack_record(
      std::span<const cgpui::PaintMetadata>(&metadata, 1));
  auto shadowed = cgpui_test::capture_pixels(
      window, white, [&](cgpui::RenderFrame& frame) {
        frame.draw_box_shadow(cgpui::BoxShadowDraw{
            .rect = {.origin = {8, 8}, .size = {20, 20}},
            .shadow = {.color = black, .offset = {40, 2}, .spread_radius = 2},
            .composition_stack = stack,
            .metadata = metadata,
        });
      });
  auto solid = cgpui_test::capture_pixels(
      window, white, [&](cgpui::RenderFrame& frame) {
        frame.draw_rect(cgpui::SolidRect{
            .rect = {.origin = {46, 8}, .size = {24, 24}},
            .color = black,
            .composition_stack = stack,
            .metadata = metadata,
        });
      });
  if (!shadowed || !solid || shadowed->rgba8.size() != solid->rgba8.size()) {
    return 30;
  }
  for (std::uint32_t y = 0; y < solid->height; ++y) {
    for (std::uint32_t x = 0; x < solid->width; ++x) {
      if (!close_to(*shadowed, x, y, (*solid->pixel_rgba8(x, y))[0], 1)) return 31;
    }
  }
  return close_to(*solid, 60, 20, 128, 1) ? 0 : 32;
}

} // namespace

int main() {
  cgpui_test::RendererPixelTestWindow window(
      L"CGPUIRendererBoxShadowPixelTestWindow", L"CGPUI Renderer Box Shadow Test");
  (void)window.resize_client(128, 128);
  for (const int result :
       {test_falloff(window, 1.0F), test_falloff(window, 1.5F),
        test_rounded_corner(window), test_clip(window),
        test_zero_blur(window)}) {
    if (result != 0) return result;
  }
  return 0;
}
