#include "cgpui/ui/ui.hpp"
#include "cgpui/ui/element.hpp"
#include "cgpui/ui/layout.hpp"
#include "cgpui/ui/scroll.hpp"
#include "cgpui/ui/style.hpp"
#include "cgpui/ui/text.hpp"

#include <memory>

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
  cgpui::TextModel text_model;
  cgpui::ScrollModel scroll_model;
  const cgpui::Style style =
      cgpui::Style{}
          .with_padding(cgpui::edges(cgpui::px(1.0F)))
          .with_background_color(cgpui::rgb(255, 0, 0))
          .with_foreground_color(cgpui::rgba(255, 255, 255, 0.5F));
  cgpui::AnyElement element =
      cgpui::into_element(cgpui::div().child(
          std::make_unique<cgpui::FixedSizeElement>(cgpui::Size{1.0F, 2.0F})));
  scroll_model.set_viewport_size(cgpui::Size{10.0F, 10.0F});
  text_model.insert_text("x");
  view.paint(paint_list, cgpui::Size{100.0F, 100.0F});
  return paint_list.commands().size() == 1 && text_model.text() == "x" &&
                 style.padding.top == 1.0F &&
                 style.background_color.has_value() &&
                 style.background_color->r == 1.0F &&
                 style.foreground_color.has_value() &&
                 style.foreground_color->a == 0.5F && element != nullptr &&
                 scroll_model.offset().x == 0.0F
             ? 0
             : 1;
}
