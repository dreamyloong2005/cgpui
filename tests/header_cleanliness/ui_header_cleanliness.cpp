#include "cgpui/ui/ui.hpp"
#include "cgpui/ui/element.hpp"

class TestView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList& paint_list, cgpui::Size) override {
    paint_list.fill_rect(
        cgpui::Rect{.origin = {10.0F, 10.0F}, .size = {20.0F, 20.0F}},
        cgpui::Color{.r = 1.0F, .g = 0.0F, .b = 0.0F, .a = 1.0F});
  }
};

int main() {
  cgpui::PaintList paint_list;
  TestView view;
  view.paint(paint_list, cgpui::Size{100.0F, 100.0F});
  return paint_list.commands().size() == 1 ? 0 : 1;
}
