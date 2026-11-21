#include "metal_pixel_test_support.hpp"
#include "vulkan_box_shadow_pixel_support.hpp"

#include "metal_effects_internal.hpp"

#include <cstdio>

// Box shadows on a CAMetalLayer, against the same references as the Vulkan
// pixel test (vulkan_box_shadow_pixel_support.hpp): the exact Gaussian
// blurred rect and a C++ port of the shader formula, at device pixel
// centers. Blur radius 8 means sigma = 4 logical. The drawable is 64 x 64
// device pixels at every scale.
namespace {

constexpr cgpui::Color white{.r = 1.0F, .g = 1.0F, .b = 1.0F, .a = 1.0F};
constexpr cgpui::Color black{.a = 1.0F};
constexpr cgpui::Rect box{.origin = {16.0F, 16.0F}, .size = {24.0F, 24.0F}};

cgpui::BoxShadowDraw shadow(float blur, cgpui::BorderRadii radius = {}) {
  return {.rect = box, .radius = radius, .shadow = {.color = black, .blur_radius = blur}};
}

int expected_byte(double coverage) {
  return static_cast<int>(std::lround(255.0 * (1.0 - coverage)));
}

bool close_to(const cgpui::RendererFramePixels& pixels, std::uint32_t x,
              std::uint32_t y, int expected, int tolerance) {
  const auto actual = pixels.pixel_rgba8(x, y);
  if (!actual || std::abs(static_cast<int>((*actual)[0]) - expected) > tolerance) {
    std::fprintf(stderr, "pixel %u,%u got %d expected %d\n", x, y,
                 actual ? static_cast<int>((*actual)[0]) : -1, expected);
    return false;
  }
  return true;
}

std::optional<cgpui::RendererFramePixels> capture(
    cgpui_test::MetalPixelTestSurface& surface, float scale,
    const cgpui::BoxShadowDraw& draw) {
  return cgpui_test::capture_metal_pixels(
      surface, white, [&](cgpui::RenderFrame& frame) { frame.draw_box_shadow(draw); },
      {}, cgpui::DpiScale{scale});
}

// Falloff left of and above the box, and the clipped-out box interior.
int test_falloff(cgpui_test::MetalPixelTestSurface& surface, float scale) {
  auto pixels = capture(surface, scale, shadow(8.0F));
  if (!pixels) return 1;
  const double s = scale;
  const double x0 = 16.0 * s;
  const double x1 = 40.0 * s;
  const auto mid = static_cast<std::uint32_t>(28.0 * s);
  for (std::uint32_t d = 0; d < static_cast<std::uint32_t>(x0); ++d) {
    const double c = d + 0.5;
    const double m = mid + 0.5;
    if (!close_to(*pixels, d, mid,
                  expected_byte(cgpui_test::blurred_rect_coverage(
                      c, m, x0, x0, x1, x1, 4.0 * s)), 3) ||
        !close_to(*pixels, mid, d,
                  expected_byte(cgpui_test::blurred_rect_coverage(
                      m, c, x0, x0, x1, x1, 4.0 * s)), 3)) {
      return 2;
    }
  }
  return close_to(*pixels, mid, mid, 255, 0) ? 0 : 3;
}

// Rounded corner falloff against the ported shader formula at scale 2;
// pixels within 1.5 px of the box are skipped (clip-out antialiasing).
int test_rounded_corner(cgpui_test::MetalPixelTestSurface& surface) {
  constexpr double s = 2.0;
  auto pixels = capture(surface, 2.0F, shadow(8.0F, cgpui::BorderRadii::all(8.0F)));
  if (!pixels) return 10;
  const double x0 = 16.0 * s;
  const double x1 = 40.0 * s;
  const double r = 8.0 * s;
  for (std::uint32_t y = 0; y < static_cast<std::uint32_t>(x0 + r); ++y) {
    for (std::uint32_t x = 0; x < static_cast<std::uint32_t>(x0 + r); ++x) {
      const double px = x + 0.5;
      const double py = y + 0.5;
      const double dx = std::max(x0 + r - px, 0.0);
      const double dy = std::max(x0 + r - py, 0.0);
      if (std::sqrt(dx * dx + dy * dy) - r < 1.5) continue;
      const double coverage = cgpui_test::rounded_rect_shadow_coverage(
          px, py, x0, x0, x1, x1, r, 4.0 * s);
      if (!close_to(*pixels, x, y, expected_byte(coverage), 3)) return 11;
    }
  }
  return 0;
}

// A clip rect cuts the shadow; pixels outside the clip stay white.
int test_clip(cgpui_test::MetalPixelTestSurface& surface) {
  cgpui::BoxShadowDraw draw = shadow(8.0F);
  draw.clip_rect = cgpui::Rect{.origin = {0, 0}, .size = {28, 64}};
  auto pixels = capture(surface, 1.0F, draw);
  if (!pixels) return 20;
  const double expected =
      cgpui_test::blurred_rect_coverage(10.5, 28.5, 16, 16, 40, 40, 4.0);
  return close_to(*pixels, 10, 28, expected_byte(expected), 3) &&
                 close_to(*pixels, 28, 12, 255, 0) && close_to(*pixels, 44, 28, 255, 0)
             ? 0
             : 21;
}

// Zero blur with spread and offset equals a solid rect; empty shadows and
// transparent colors produce no quad.
int test_geometry() {
  const auto quad = cgpui::metal_box_shadow_quad(cgpui::BoxShadowDraw{
      .rect = {.origin = {8, 8}, .size = {20, 20}},
      .radius = cgpui::BorderRadii::all(30.0F),
      .shadow = {.color = black, .offset = {40, 2}, .spread_radius = 2}});
  if (!quad || quad->uniforms.sigma != 0.0F ||
      quad->uniforms.shadow_rect.x != 46.0F || quad->uniforms.shadow_rect.w != 32.0F ||
      quad->uniforms.box_radii.x != 10.0F || quad->vertices[0].local.x != 45.0F) {
    return 30;
  }
  cgpui::BoxShadowDraw clear = shadow(8.0F);
  clear.shadow.color.a = 0.0F;
  cgpui::BoxShadowDraw shrunk = shadow(8.0F);
  shrunk.shadow.spread_radius = -20.0F;
  return !cgpui::metal_box_shadow_quad(clear) && !cgpui::metal_box_shadow_quad(shrunk)
             ? 0
             : 31;
}

}  // namespace

int main() {
  @autoreleasepool {
    cgpui_test::MetalPixelTestSurface surface;
    for (const int result :
         {test_geometry(), test_falloff(surface, 1.0F), test_falloff(surface, 2.0F),
          test_rounded_corner(surface), test_clip(surface)}) {
      if (result != 0) return result;
    }
    return 0;
  }
}
