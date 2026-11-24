#include "dx12_backdrop_fallback_test_support.hpp"
#include "dx12_diagnostics_support.hpp"
#include <limits>

using namespace cgpui;
using namespace cgpui_test;

namespace {
bool valid_frame(Dx12BackdropFallbackFixture& fixture, float sigma = 4) {
  auto frame = fixture.begin();
  if (!frame) return false;
  fallback_split_scene(*frame);
  auto panel = fallback_panel();
  panel.blur_radius = sigma;
  frame->draw_backdrop_blur(panel);
  return fixture.capture(*frame) && fixture.pixel(24, 24, {128, 0, 0, 255}) &&
      fixture.pixel(40, 24, {255, 128, 128, 255}) &&
      fallback_diagnostics(fixture, 2, 2, 1, 1);
}
void invalidate(BackdropBlurDraw& draw, unsigned index) {
  const auto nan = std::numeric_limits<float>::quiet_NaN();
  const auto infinity = std::numeric_limits<float>::infinity();
  const auto maximum = (std::numeric_limits<float>::max)();
  switch (index) {
    case 0: draw.rect.origin.x = nan; break;
    case 1: draw.rect.origin.y = -infinity; break;
    case 2: draw.rect.size.width = infinity; break;
    case 3: draw.rect.size.height = nan; break;
    case 4: draw.tint->r = nan; break;
    case 5: draw.tint->a = infinity; break;
    case 6: draw.clip_rect = Rect{{nan, 0}, {64, 64}}; break;
    case 7: draw.clip_rect = Rect{{0, 0}, {64, infinity}}; break;
    case 8: draw.clip_stack.current_clip_rect = Rect{{0, -infinity}, {64, 64}}; break;
    case 9: draw.clip_stack.clips = {Rect{{0, 0}, {nan, 64}}}; break;
    case 10: draw.rect.origin.x = maximum * 0.5F; draw.rect.size.width = maximum * 0.5F; break;
    case 11: draw.rect.origin.x = maximum; draw.rect.size.width = 1; break;
  }
}
} // namespace

int main() {
  Dx12BackdropFallbackFixture fixture;
  if (!fixture.initialize(1.5F)) return dx12_fail(1, "fallback validation fixture unavailable");
  fixture.force_unsupported();
  if (!valid_frame(fixture)) return dx12_fail(2, "fallback validation warmup failed");
  for (unsigned variant = 0; variant != 5; ++variant) {
    for (unsigned invalid_case = 0; invalid_case != 12; ++invalid_case) {
      if (variant == 1 && (invalid_case == 4 || invalid_case == 5)) continue;
      auto draw = fallback_panel();
      if (variant == 1) draw.tint.reset();
      if (variant == 2) draw.tint->a = 0;
      if (variant == 3) draw.clip_rect = Rect{{100, 100}, {8, 8}};
      if (variant == 4) draw.rect.size.height = 0;
      invalidate(draw, invalid_case);
      const auto saved = *fixture.state->diagnostics;
      const auto saved_pixels = *fixture.state->pixels;
      auto frame = fixture.begin();
      if (!frame) return dx12_fail(3, "fallback invalid begin failed");
      frame->draw_backdrop_blur(draw);
      fallback_foreground(*frame);
      if (!frame->request_pixel_capture()) return dx12_fail(4, "invalid frame capture request refused");
      const auto result = frame->present();
      const auto& pixels = fixture.state->pixels;
      if (result || result.error().code != ErrorCode::invalid_argument || !pixels ||
          pixels->width != saved_pixels.width || pixels->height != saved_pixels.height ||
          pixels->encoding != saved_pixels.encoding || pixels->rgba8 != saved_pixels.rgba8 ||
          !diagnostic_same(fixture.state->diagnostics ? &*fixture.state->diagnostics : nullptr, saved))
        return dx12_fail(5, "fallback hid invalid raw data or changed successful pixel/diagnostic history");
      if (!valid_frame(fixture)) return dx12_fail(6, "invalid fallback poisoned the next native frame");
    }
  }
  for (const float sigma : {std::numeric_limits<float>::quiet_NaN(),
      std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity(),
      -4.0F, 0.0F, (std::numeric_limits<float>::max)()}) {
    if (!valid_frame(fixture, sigma)) return dx12_fail(7, "fallback introduced a new sigma rejection policy");
  }
  const auto saved = *fixture.state->diagnostics;
  auto frame = fixture.begin();
  if (!frame) return dx12_fail(8, "prior-error frame begin failed");
  frame->clear({std::numeric_limits<float>::quiet_NaN(), 0, 0, 1});
  frame->draw_backdrop_blur(fallback_panel());
  const auto result = frame->present();
  if (result || result.error().code != ErrorCode::invalid_argument ||
      result.error().message != "DX12 clear color must be finite" ||
      !diagnostic_same(fixture.state->diagnostics ? &*fixture.state->diagnostics : nullptr, saved))
    return dx12_fail(9, "fallback replaced or swallowed a preceding frame error");
  return valid_frame(fixture) ? 0 : dx12_fail(10, "prior-error frame poisoned valid fallback");
}
