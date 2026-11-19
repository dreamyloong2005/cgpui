// scroll(state, child) moves its child (and the child's descendants) by the
// scroll offset and clips the child's paint to the scroll viewport.
#include "cgpui/ui/element.hpp"
#include "cgpui/ui/scroll.hpp"
#include "cgpui/ui/style.hpp"
#include "cgpui/ui/ui.hpp"

#include <iostream>
#include <optional>
#include <string_view>

namespace {

using cgpui::Color;
using cgpui::Rect;

constexpr Color inner_color = cgpui::rgb(10, 200, 10);

bool same_rect(std::optional<Rect> actual, Rect expected) {
  return actual.has_value() && actual->origin.x == expected.origin.x &&
         actual->origin.y == expected.origin.y &&
         actual->size.width == expected.size.width &&
         actual->size.height == expected.size.height;
}

int fail(int code, std::string_view message) {
  std::cerr << message << '\n';
  return code;
}

} // namespace

int main() {
  cgpui::ScrollState state;
  cgpui::ElementTree tree;
  (void)tree.set_root(cgpui::into_element(
      cgpui::div().padding(cgpui::edges(10.0F)).child(cgpui::scroll(
          state,
          cgpui::v_stack().size(100.0F, 300.0F).child(
              cgpui::div().size(30.0F, 20.0F).background(inner_color))))));
  const cgpui::LayoutInput input{
      .constraints = {.max_size = {120.0F, 120.0F}}};
  (void)tree.layout_root(input);
  state.set_offset(cgpui::Point{.x = 0.0F, .y = 50.0F});
  if (state.offset().y != 50.0F) return fail(1, "offset must be scrollable");
  (void)tree.layout_root(input);

  cgpui::PaintList list;
  tree.paint(list);
  for (const cgpui::PaintCommand& command : list.commands()) {
    if (command.kind != cgpui::PaintCommandKind::solid_rect) continue;
    // The inner box starts at the scroll origin (10, 10) minus the offset.
    if (!same_rect(command.solid_rect.rect, Rect{{10, -40}, {30, 20}})) {
      return fail(2, "child descendants must move by the scroll offset");
    }
    if (!same_rect(command.clip_rect, Rect{{10, 10}, {100, 120}})) {
      return fail(3, "child paint must be clipped to the scroll viewport");
    }
    return 0;
  }
  return fail(4, "inner box must paint");
}
