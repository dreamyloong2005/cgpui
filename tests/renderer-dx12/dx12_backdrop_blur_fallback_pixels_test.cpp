#include "dx12_backdrop_fallback_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  for (const float scale : {1.0F, 1.5F, 2.0F}) {
    Dx12BackdropFallbackFixture fixture;
    if (!fixture.initialize(scale)) return dx12_fail(1, "fallback real native fixture unavailable");
    fixture.force_unsupported();
    // Half-tint is first: the original DX12 override fails this real present.
    for (unsigned variant = 0; variant != 4; ++variant) {
      auto panel = fallback_panel();
      if (variant == 1) panel.tint.reset();
      if (variant == 2) panel.tint->a = 0;
      if (variant == 3) panel.tint->a = 1;
      auto frame = fixture.begin();
      if (!frame) return dx12_fail(2, "fallback begin failed");
      fallback_split_scene(*frame);
      frame->draw_backdrop_blur(panel);
      fallback_foreground(*frame);
      if (!fixture.capture(*frame)) return dx12_fail(3, "unsupported blur poisoned real present");
      const auto black = variant == 0 ? std::array<unsigned char, 4>{128, 0, 0, 255}
          : variant == 3 ? std::array<unsigned char, 4>{255, 0, 0, 255}
                         : std::array<unsigned char, 4>{0, 0, 0, 255};
      const auto white = variant == 0 ? std::array<unsigned char, 4>{255, 128, 128, 255}
          : variant == 3 ? std::array<unsigned char, 4>{255, 0, 0, 255}
                         : std::array<unsigned char, 4>{255, 255, 255, 255};
      if (!fixture.pixel(24, 24, black) || !fixture.pixel(31, 24, black) ||
          !fixture.pixel(32, 24, white) || !fixture.pixel(40, 24, white) ||
          !fixture.pixel(8, 8, {0, 0, 0, 255}) ||
          !fixture.pixel(54, 8, {255, 255, 255, 255}) ||
          !fixture.pixel(24, 4, {0, 0, 0, 255}) ||
          !fixture.pixel(40, 4, {255, 255, 255, 255}) ||
          !fixture.pixel(31, 31, {0, 255, 0, 255}))
        return dx12_fail(4, "fallback lost tint, rounding, hard edge, scale or foreground order");
      const std::size_t rounded = variant == 1 ? 0 : 1;
      if (!fallback_diagnostics(fixture, 2 + rounded, 2 + rounded, 2, rounded))
        return dx12_fail(5, "fallback mislabeled primitives, commands or native blur work");
    }
  }
  return 0;
}
