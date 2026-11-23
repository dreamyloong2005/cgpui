#include "dx12_backdrop_fallback_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  for (const float scale : {1.0F, 1.5F, 2.0F}) {
    Dx12BackdropFallbackFixture fixture;
    if (!fixture.initialize(scale)) return dx12_fail(1, "fallback composition fixture unavailable");
    fixture.force_unsupported();
    for (unsigned variant = 0; variant != 5; ++variant) {
      auto panel = fallback_panel();
      panel.rect = {{4, 4}, {12, 12}};
      panel.radius = BorderRadii::all(5);
      panel.tint = Color{1, 0, 0, 0.5F};
      panel.metadata = {.opacity = 0.5F, .transform = {.translate_x = 8, .translate_y = 8}};
      if (variant != 0) {
        // Current stack metadata is already composed; it replaces the standalone value.
        const PaintMetadata current{.opacity = 0.5F,
            .transform = {.scale_x = 2, .translate_x = 20, .translate_y = 12}};
        const std::array entries{PaintMetadata{.opacity = 0.1F}, current};
        panel.composition_stack = renderer_composition_stack_record(entries);
      }
      if (variant == 1) {
        panel.clip_rect = Rect{{32, 18}, {16, 8}};
        const std::array clips{Rect{{30, 16}, {16, 12}}, Rect{{34, 20}, {8, 4}}};
        panel.clip_stack = renderer_clip_stack_record(clips);
      }
      if (variant == 2) panel.clip_rect = Rect{{100, 100}, {8, 8}};
      if (variant == 3) panel.composition_stack.current_metadata.opacity = 0;
      if (variant == 4) panel.rect.size.width = 0;
      auto frame = fixture.begin();
      if (!frame) return dx12_fail(2, "fallback composition begin failed");
      frame->clear({0, 0, 1, 1});
      frame->draw_backdrop_blur(panel);
      if (!fixture.capture(*frame)) return dx12_fail(3, "fallback composition present failed");
      if (variant == 0) {
        // [4,16)^2 + (8,8) => [12,24)^2, tint alpha .5 * opacity .5.
        if (!fixture.pixel(18, 18, {64, 0, 191, 255}) ||
            !fixture.pixel(8, 8, {0, 0, 255, 255}) ||
            !fixture.pixel(12, 12, {0, 0, 255, 255}) ||
            !fixture.pixel(26, 18, {0, 0, 255, 255}))
          return dx12_fail(4, "standalone metadata or single DPI conversion lost");
      } else if (variant == 1) {
        // Stack transforms panel to [28,52)x[16,28); effective clip [34,42)x[20,24).
        if (!fixture.pixel(38, 22, {64, 0, 191, 255}) ||
            !fixture.pixel(32, 22, {0, 0, 255, 255}) ||
            !fixture.pixel(44, 22, {0, 0, 255, 255}) ||
            !fixture.pixel(38, 18, {0, 0, 255, 255}) ||
            !fixture.pixel(38, 26, {0, 0, 255, 255}))
          return dx12_fail(5, "fallback clip stack or composition precedence changed");
      } else if (!fixture.pixel(38, 22, {0, 0, 255, 255})) {
        return dx12_fail(6, "invisible fallback changed pixels");
      }
      const std::size_t submitted = variant == 2 || variant == 4 ? 0 : 1;
      if (!fallback_diagnostics(fixture, 1, submitted, 0, submitted))
        return dx12_fail(7, "invisible fallback invented or omitted existing rounded work");
    }
  }
  return 0;
}
