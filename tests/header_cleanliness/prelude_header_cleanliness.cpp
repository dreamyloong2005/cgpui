#include "cgpui/cgpui.hpp"

class PreludeView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList& paint_list, cgpui::Size) override {
    paint_list.fill_rect(
        cgpui::Rect{.origin = {0.0F, 0.0F}, .size = {1.0F, 1.0F}},
        cgpui::Color{.r = 1.0F, .g = 1.0F, .b = 1.0F, .a = 1.0F});
  }

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent&,
      const cgpui::ViewContext& context) override {
    context.request_paint();
    return cgpui::EventResult::unhandled();
  }
};

int main() {
  cgpui::ElementTree tree;
  cgpui::TextModel model("x");
  const cgpui::ElementId root_id =
      tree.set_root(cgpui::div()
                        .size(cgpui::px(12.0F), cgpui::px(8.0F))
                        .gap(cgpui::px(1.0F))
                        .margin(cgpui::edges(2.0F, 1.0F))
                        .background(cgpui::rgb(32, 64, 128))
                        .foreground(cgpui::rgba(255, 255, 255, 0.75F))
                        .border_width(cgpui::edges(1.0F))
                        .border_color(cgpui::rgba(255, 255, 255, 0.75F))
                        .border_radius(cgpui::BorderRadii::all(2.0F))
                        .child(cgpui::text(model))
                        .build());
  PreludeView view;
  cgpui::AppRunnerOptions options;
  (void)view;
  (void)options;
  return root_id.value != 0 && tree.root_id() == root_id ? 0 : 1;
}
