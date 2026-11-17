#include "cgpui/ui/ui.hpp"
#include "styled_box_paint_internal.hpp"
#include "ui_paint_internal.hpp"

#include <vector>

// Style::backdrop_blur paints shadow, then backdrop, then the box fill; the
// default RenderFrame (no backdrop support) draws the tint only.
namespace {

class RecordingFrame final : public cgpui::RenderFrame {
 public:
  void clear(cgpui::Color) override {}
  void draw_rect(const cgpui::SolidRect&) override { order.push_back(0); }
  void draw_rounded_rect(const cgpui::RoundedRectDraw& rect) override {
    order.push_back(1);
    rounded.push_back(rect);
  }
  void draw_box_shadow(const cgpui::BoxShadowDraw&) override {
    order.push_back(2);
  }
  cgpui::Result<void> present() override { return {}; }

  std::vector<int> order;
  std::vector<cgpui::RoundedRectDraw> rounded;
};

} // namespace

int main() {
  const cgpui::Style style =
      cgpui::Style{}
          .with_background_color({.r = 1.0F, .a = 0.4F})
          .with_border_radius(cgpui::BorderRadii::all(6.0F))
          .with_box_shadow({.color = {.a = 0.3F}, .blur_radius = 12.0F})
          .with_backdrop_blur(
              {.radius = 8.0F, .tint = cgpui::Color{.b = 1.0F, .a = 0.2F}});
  cgpui::PaintList paint_list;
  cgpui::paint_styled_box_base(
      paint_list, cgpui::Rect{.origin = {0, 0}, .size = {40, 20}}, style);
  const auto commands = paint_list.commands();
  if (commands.size() != 3 ||
      commands[0].kind != cgpui::PaintCommandKind::box_shadow ||
      commands[1].kind != cgpui::PaintCommandKind::backdrop_blur ||
      commands[1].backdrop_blur.blur_radius != 8.0F ||
      commands[2].kind != cgpui::PaintCommandKind::rounded_rect) {
    return 1;
  }
  RecordingFrame frame;
  cgpui::FrameStatistics statistics;
  for (const cgpui::PaintCommand& command : commands) {
    cgpui::submit_paint_command_to_frame(frame, command, &statistics);
  }
  if (statistics.backdrop_blur_command_count != 1 ||
      frame.order != std::vector<int>{2, 1, 1} || frame.rounded.size() != 2) {
    return 2;
  }
  const cgpui::RoundedRectDraw& tint = frame.rounded.front();
  return tint.color.b == 1.0F && tint.color.a == 0.2F &&
                 tint.radius.top_left == 6.0F
             ? 0
             : 3;
}
