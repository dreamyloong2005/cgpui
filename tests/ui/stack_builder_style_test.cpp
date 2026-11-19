// v_stack, row and column keep the box style set on their builder:
// size, padding, margin, background, border, radius. Children sit in the
// content box and the background covers the padded box.
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

constexpr Color stack_color = cgpui::rgb(200, 10, 10);
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

cgpui::ElementBuilder child() {
  return cgpui::div().size(20.0F, 10.0F).background(child_color);
}

int fail(int code, std::string_view message) {
  std::cerr << message << '\n';
  return code;
}

// Lays out `stack` at (5, 5) in a tree and checks the box and its children.
int check_stack(cgpui::ElementBuilder stack, int code, Rect first_child,
                Rect second_child) {
  cgpui::ElementTree tree;
  (void)tree.set_root(cgpui::into_element(
      cgpui::div().padding(cgpui::edges(5.0F)).child(
          std::move(stack)
              .size(100.0F, 60.0F)
              .padding(cgpui::edges(8.0F))
              .gap(cgpui::px(4.0F))
              .background(stack_color)
              .child(child())
              .child(child()))));
  (void)tree.layout_root(cgpui::LayoutInput{
      .constraints = {.max_size = {400.0F, 300.0F}}});
  cgpui::PaintList list;
  tree.paint(list);
  // Content box 100 x 60 plus 8 px padding on each side.
  if (!same_rect(painted_rect(list, stack_color), Rect{{5, 5}, {116, 76}})) {
    return fail(code, "stack background must cover the padded box");
  }
  int seen = 0;
  for (const cgpui::PaintCommand& command : list.commands()) {
    if (command.kind != cgpui::PaintCommandKind::solid_rect ||
        !same_color(command.solid_rect.color, child_color)) {
      continue;
    }
    const Rect expected = seen == 0 ? first_child : second_child;
    if (!same_rect(command.solid_rect.rect, expected)) {
      return fail(code + 1, "children must sit in the content box");
    }
    seen += 1;
  }
  return seen == 2 ? 0 : fail(code + 2, "both children must paint");
}

// Text style set on a stack reaches its text children, as on a div.
int check_text_inheritance(cgpui::ElementBuilder stack, int code) {
  cgpui::TextModel model{"Hi"};
  cgpui::ElementTree tree;
  (void)tree.set_root(cgpui::into_element(
      std::move(stack).font_size(30.0F).foreground(child_color).child(
          cgpui::text(model))));
  (void)tree.layout_root(cgpui::LayoutInput{
      .constraints = {.max_size = {400.0F, 300.0F}}});
  cgpui::PaintList list;
  tree.paint(list);
  for (const cgpui::PaintCommand& command : list.commands()) {
    if (command.kind == cgpui::PaintCommandKind::text) {
      return command.text.font_size == 30.0F &&
                     same_color(command.text.color, child_color)
                 ? 0
                 : fail(code, "stack text style must reach text children");
    }
  }
  return fail(code + 1, "text child must paint");
}

} // namespace

int main() {
  if (const int r = check_text_inheritance(cgpui::v_stack(), 40)) return r;
  if (const int r = check_text_inheritance(cgpui::ElementBuilder::row(), 50)) {
    return r;
  }
  const Rect first{{13, 13}, {20, 10}};
  if (const int r = check_stack(cgpui::v_stack(), 10, first,
                                Rect{{13, 27}, {20, 10}})) {
    return r;
  }
  if (const int r = check_stack(cgpui::ElementBuilder::column(), 20, first,
                                Rect{{13, 27}, {20, 10}})) {
    return r;
  }
  return check_stack(cgpui::ElementBuilder::row(), 30, first,
                     Rect{{37, 13}, {20, 10}});
}
