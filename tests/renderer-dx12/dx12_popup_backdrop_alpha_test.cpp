#include "dx12_popup_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

namespace {
struct AlphaCase {
  const char* name;
  bool half_red_scene;
  float opacity;
  std::optional<Color> tint;
  std::array<int, 4> texture;
  ScreenColor screen;
};

int check_case(Renderer& renderer, const CompositionTestWindows& windows,
               const AlphaCase& test) {
  std::cout << "popup backdrop: " << test.name << '\n';
  auto frame = renderer.begin_frame();
  if (!frame) return dx12_fail(10, frame.error().message.c_str());
  (*frame)->clear({0, 0, 0, 0});
  // Use drawing rather than nonzero-RGB clear, whose premultiplication has a
  // separate contract. A constant premultiplied scene is invariant under blur.
  if (test.half_red_scene)
    (*frame)->draw_rect({.rect = {{0, 0}, {64, 64}}, .color = {1, 0, 0, 0.5F}});
  (*frame)->draw_backdrop_blur({.rect = {{8, 8}, {48, 48}}, .blur_radius = 4,
      .tint = test.tint, .metadata = {.opacity = test.opacity}});
  if (!(*frame)->request_pixel_capture()) return dx12_fail(11, "alpha backdrop capture refused");
  if (const auto result = (*frame)->present(); !result)
    return dx12_fail(12, result.error().message.c_str());

  const std::array<int, 4> outside = test.half_red_scene
      ? std::array<int, 4>{128, 0, 0, 128} : std::array<int, 4>{0, 0, 0, 0};
  const ScreenColor outside_screen = test.half_red_scene
      ? ScreenColor{128, 127, 0} : ScreenColor{0, 255, 0};
  const auto* pixels = renderer.last_frame_pixels();
  const bool texture_ok = pixels && pixels->width == 64 && pixels->height == 64 &&
      expect_texture_pixel(*pixels, 32, 32, test.texture) &&
      expect_texture_pixel(*pixels, 4, 32, outside);
  // Readback alpha alone is insufficient: an untinted transparent backdrop
  // must still show the green desktop control through the entire popup panel.
  const bool screen_ok = windows.expect({0, 255, 0}, windows.backdrop_point()) &&
      windows.expect(test.screen, windows.popup_point(32, 32)) &&
      windows.expect(outside_screen, windows.popup_point(4, 32));
  if (!texture_ok) return dx12_fail(13, "backdrop changed premultiplied scene alpha or tint interpolation");
  if (!screen_ok) return dx12_fail(14, "backdrop did not retain real desktop transparency");
  return 0;
}
} // namespace

int main() {
  Dx12RecoveryDeadline deadline;
  if (!deadline.done || !dx12_popup_desktop_ready())
    return dx12_fail(1, "popup desktop/DPI/watchdog fixture unavailable; alpha was not tested");
  CompositionTestWindows windows(48, 48);
  if (!windows.valid() || !IsWindowVisible(windows.popup()) || IsIconic(windows.popup()))
    return dx12_fail(2, "visible composition popup unavailable");
  if (!windows.expect({0, 255, 0}, windows.backdrop_point()))
    return dx12_fail(3, "green desktop control pixel is not visible");
  auto renderer = create_dx12_renderer(dx12_popup_descriptor(windows.popup()));
  if (!renderer) return dx12_fail(4, renderer.error().message.c_str());
  if (!(*renderer)->surface_composition().alpha_composited() ||
      !(*renderer)->backdrop_blur_support().supported)
    return dx12_fail(5, "popup alpha/backdrop capability unavailable");

  // Literal premultiplied-RGBA examples, independent of planner/filter code.
  // Blur must replace the scene through its mask, not source-over the same
  // scene again. Neither effect opacity makes uniform half-red more opaque.
  // Half-blue tint over half-red gives (.25,0,.5,.75); applying that effect
  // at half opacity to the original gives (.375,0,.25,.625).
  const AlphaCase cases[] = {
      {"transparent untinted scene", false, 1, std::nullopt,
       {0, 0, 0, 0}, {0, 255, 0}},
      {"half-red, full effect", true, 1, std::nullopt,
       {128, 0, 0, 128}, {128, 127, 0}},
      {"half-red, half effect", true, 0.5F, std::nullopt,
       {128, 0, 0, 128}, {128, 127, 0}},
      {"half-red, half-blue tint, half effect", true, 0.5F, Color{0, 0, 1, 0.5F},
       {96, 0, 64, 159}, {96, 96, 64}},
      {"transparent, half-red tint, half effect", false, 0.5F, Color{1, 0, 0, 0.5F},
       {64, 0, 0, 64}, {64, 191, 0}},
  };
  for (const auto& test : cases) {
    if (const int result = check_case(**renderer, windows, test)) return result;
  }
  return 0;
}
