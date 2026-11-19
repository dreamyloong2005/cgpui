// A flex child resized by flex-grow is laid out again at its final size,
// so its own children are placed inside the grown box.
#include "cgpui/ui/element.hpp"
#include "cgpui/ui/style.hpp"
#include "cgpui/ui/ui.hpp"

#include <iostream>
#include <optional>
#include <string_view>
#include <utility>

namespace {

using cgpui::Color;
using cgpui::Rect;

constexpr Color child_color = cgpui::rgb(10, 200, 10);

bool same_color(Color lhs, Color rhs) {
  return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
}

bool same_rect(std::optional<Rect> actual, Rect expected) {
  return actual.has_value() && actual->origin.x == expected.origin.x &&
         actual->origin.y == expected.origin.y &&
         actual->size.width == expected.size.width &&
         actual->size.height == expected.size.height;
}

std::optional<Rect> painted_rect(const cgpui::PaintList& list, Color color) {
  for (const cgpui::PaintCommand& command : list.commands()) {
    if (command.kind == cgpui::PaintCommandKind::solid_rect &&
        same_color(command.solid_rect.color, color)) {
      return command.solid_rect.rect;
    }
  }
  return std::nullopt;
}

int fail(int code, std::string_view message) {
  std::cerr << message << '\n';
  return code;
}

// Children of a flex child are laid out again at the size flex-grow (or
// shrink) gives it, so they follow its final box.
int check_grown_row() {
  cgpui::ElementTree tree;
  (void)tree.set_root(cgpui::into_element(
      cgpui::ElementBuilder::row().size(200.0F, 20.0F).child(
          cgpui::ElementBuilder::row().flex_grow(1.0F).justify_end().child(
              cgpui::div().size(20.0F, 10.0F).background(child_color)))));
  (void)tree.layout_root(cgpui::LayoutInput{
      .constraints = {.max_size = {400.0F, 300.0F}}});
  cgpui::PaintList list;
  tree.paint(list);
  // The grown row spans 0..200, so its end-justified child sits at 180.
  if (!same_rect(painted_rect(list, child_color), Rect{{180, 0}, {20, 10}})) {
    return fail(1, "grown child's children must follow its grown box");
  }
  return 0;
}

int check_grown_column() {
  cgpui::ElementTree tree;
  (void)tree.set_root(cgpui::into_element(
      cgpui::ElementBuilder::column().size(50.0F, 100.0F).child(
          cgpui::ElementBuilder::column().flex_grow(1.0F).justify_end().child(
              cgpui::div().size(20.0F, 10.0F).background(child_color)))));
  (void)tree.layout_root(cgpui::LayoutInput{
      .constraints = {.max_size = {400.0F, 300.0F}}});
  cgpui::PaintList list;
  tree.paint(list);
  if (!same_rect(painted_rect(list, child_color), Rect{{0, 90}, {20, 10}})) {
    return fail(2, "grown column's children must follow its grown box");
  }
  return 0;
}

} // namespace

int main() {
  if (const int r = check_grown_row()) return r;
  return check_grown_column();
}
