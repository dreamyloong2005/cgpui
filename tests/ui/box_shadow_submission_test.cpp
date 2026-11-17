#include "cgpui/ui/ui.hpp"
#include "ui_paint_internal.hpp"

#include <vector>

// Styled boxes paint their shadow before the fill, and the shadow command
// now reaches the renderer instead of being dropped.
namespace {

class RecordingFrame final : public cgpui::RenderFrame {
 public:
  void clear(cgpui::Color) override {}
  void draw_rect(const cgpui::SolidRect&) override { order.push_back(0); }
  void draw_rounded_rect(const cgpui::RoundedRectDraw&) override {
    order.push_back(1);
  }
  void draw_box_shadow(const cgpui::BoxShadowDraw& shadow) override {
    order.push_back(2);
    shadows.push_back(shadow);
  }
  cgpui::Result<void> present() override { return {}; }

  std::vector<int> order;
  std::vector<cgpui::BoxShadowDraw> shadows;
};

} // namespace

int main() {
  cgpui::PaintList paint_list;
  paint_list.push_clip(cgpui::Rect{.origin = {0, 0}, .size = {50, 50}});
  const cgpui::BoxShadow shadow{
      .color = {.a = 0.5F}, .offset = {1, 2}, .blur_radius = 6,
      .spread_radius = 1};
  paint_list.draw_box_shadow(
      cgpui::Rect{.origin = {4, 4}, .size = {20, 10}}, shadow,
      cgpui::BorderRadii::all(3.0F));
  paint_list.fill_rounded_rect(
      cgpui::Rect{.origin = {4, 4}, .size = {20, 10}}, {.a = 1.0F},
      cgpui::BorderRadii::all(3.0F));
  RecordingFrame frame;
  cgpui::FrameStatistics statistics;
  for (const cgpui::PaintCommand& command : paint_list.commands()) {
    cgpui::submit_paint_command_to_frame(frame, command, &statistics);
  }
  if (frame.order != std::vector<int>{2, 1} || frame.shadows.size() != 1 ||
      statistics.box_shadow_command_count != 1 ||
      statistics.submitted_command_count != 2) {
    return 1;
  }
  const cgpui::BoxShadowDraw& draw = frame.shadows.front();
  return draw.rect.size.width == 20.0F && draw.radius.top_left == 3.0F &&
                 draw.shadow.blur_radius == 6.0F &&
                 draw.shadow.offset.y == 2.0F && draw.clip_rect.has_value() &&
                 draw.clip_rect->size.width == 50.0F
             ? 0
             : 2;
}
