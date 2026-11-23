#include "dx12_composition_probe_test_support.hpp"
#include "dx12_test_support.hpp"
#include <cstdio>

namespace cgpui_test {
bool Dx12CompositionProbe::verify_pixels(bool blue) {
  const auto* pixels = state->pixels ? &*state->pixels : nullptr;
  const std::array<unsigned char, 4> half = blue
      ? std::array<unsigned char, 4>{0, 0, 128, 128} : std::array<unsigned char, 4>{128, 0, 0, 128};
  const std::array<unsigned char, 4> full = blue
      ? std::array<unsigned char, 4>{0, 0, 255, 255} : std::array<unsigned char, 4>{255, 0, 0, 255};
  if (!invariant(pixels && pixels->width == state->width && pixels->height == state->height &&
      dx12_pixel(pixels, 10, 32, half, 2) && dx12_pixel(pixels, 30, 32, {0, 0, 0, 0}, 2) &&
      dx12_pixel(pixels, 52, 32, full, 2), "actual premultiplied pixels/dimensions differ")) return false;
  std::printf("COMPOSITION_PROBE capture width=%u height=%u blue=%d premultiplied=verified\n",
      state->width, state->height, blue);
  if (!visible_) {
    composition_probe_note("desktop_pixels", "NOT measured: initially hidden HWND remains hidden");
    return true;
  }
  return invariant(windows.expect_backdrop() &&
      windows.expect_screen(blue ? 0 : 128, 127, blue ? 128 : 0, 10, 32) &&
      windows.expect_screen(0, 255, 0, 30, 32) &&
      windows.expect_screen(blue ? 0 : 255, 0, blue ? 255 : 0, 52, 32),
      "actual desktop alpha/control pixels differ");
}
} // namespace cgpui_test
