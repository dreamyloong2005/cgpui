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
  cgpui::AnyElement element =
      cgpui::into_element(cgpui::div()
                              .size(cgpui::px(1.0F), cgpui::px(2.0F))
                              .padding(cgpui::edges(cgpui::px(1.0F)))
                              .background(cgpui::rgb(255, 0, 0))
                              .foreground(cgpui::rgba(255, 255, 255, 0.5F)));
  const auto* styled =
      dynamic_cast<const cgpui::StyledElement*>(element.get());
  scroll_model.set_viewport_size(cgpui::Size{10.0F, 10.0F});
  text_model.insert_text("x");
  view.paint(paint_list, cgpui::Size{100.0F, 100.0F});
  return paint_list.commands().size() == 1 && text_model.text() == "x" &&
                 styled != nullptr && styled->style().padding.top == 1.0F &&
                 styled->style().preferred_size.width == 1.0F &&
                 styled->style().background_color.has_value() &&
                 styled->style().background_color->r == 1.0F &&
                 styled->style().foreground_color.has_value() &&
                 styled->style().foreground_color->a == 0.5F &&
                 scroll_model.offset().x == 0.0F
             ? 0
             : 1;
}
