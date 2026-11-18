// Layout bounds are window coordinates at any depth: paint commands, hit
// tests and accessibility bounds of nested elements agree with them.
#include "cgpui/ui/element.hpp"
#include "cgpui/ui/scroll.hpp"
#include "cgpui/ui/style.hpp"
#include "cgpui/ui/ui.hpp"

#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace {

using cgpui::Color;
using cgpui::ElementId;
using cgpui::Point;
using cgpui::Rect;
using cgpui::Size;

bool same_rect(std::optional<Rect> actual, Rect expected) {
  return actual.has_value() && actual->origin.x == expected.origin.x &&
         actual->origin.y == expected.origin.y &&
         actual->size.width == expected.size.width &&
         actual->size.height == expected.size.height;
}

bool same_color(Color lhs, Color rhs) {
  return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
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

std::unique_ptr<cgpui::StyledElement> box(cgpui::Style style, ElementId id) {
  auto element = std::make_unique<cgpui::StyledElement>(std::move(style),
                                                        std::unique_ptr<cgpui::Element>{});
  element->assign_id(id);
  return element;
}

} // namespace

namespace {

constexpr Color kSpacer{.r = 0.1F, .g = 0.2F, .b = 0.3F, .a = 1.0F};
constexpr Color kFirstRow{.r = 0.4F, .g = 0.5F, .b = 0.6F, .a = 1.0F};
constexpr Color kSecondRow{.r = 0.7F, .g = 0.8F, .b = 0.9F, .a = 1.0F};

std::unique_ptr<cgpui::Element> list_row(Color color, ElementId id) {
  std::vector<std::unique_ptr<cgpui::Element>> children;
  children.push_back(box(cgpui::Style{}
                             .with_preferred_size(Size{40.0F, 16.0F})
                             .with_background_color(color),
                         id));
  return std::make_unique<cgpui::StyledElement>(
      cgpui::Style{}.with_padding(cgpui::EdgeSizes::all(2.0F)),
      std::move(children));
}

// root (margin 4, padding 10) > column (gap 6) > box (padding 5) >
// scrollable list (offset 8) > row (padding 2) > leaf; the column's last
// child is a window-control (close button) area.
int test_nested_containers_use_window_coordinates() {
  cgpui::ScrollState state;
  std::vector<cgpui::AnyElement> rows;
  rows.push_back(list_row(kFirstRow, ElementId{7}));
  rows.push_back(list_row(kSecondRow, ElementId{8}));
  auto list = std::make_unique<cgpui::ScrollableListElement>(
      state, cgpui::Style{}.with_preferred_size(Size{60.0F, 30.0F}),
      std::move(rows));
  list->assign_id(ElementId{5});
  std::vector<std::unique_ptr<cgpui::Element>> padded_children;
  padded_children.push_back(std::move(list));
  auto padded = std::make_unique<cgpui::StyledElement>(
      cgpui::Style{}.with_padding(cgpui::EdgeSizes::all(5.0F)),
      std::move(padded_children));
  padded->assign_id(ElementId{4});
  auto column = std::make_unique<cgpui::FlexElement>(cgpui::FlexDirection::column);
  column->assign_id(ElementId{2});
  column->set_gap(6.0F);
  column->append_child(box(cgpui::Style{}
                               .with_preferred_size(Size{100.0F, 8.0F})
                               .with_background_color(kSpacer),
                           ElementId{3}));
  column->append_child(std::move(padded));
  column->append_child(cgpui::div()
                           .size(30.0F, 6.0F)
                           .window_control_area(cgpui::WindowControlArea::close_button)
                           .build());
  std::vector<std::unique_ptr<cgpui::Element>> root_children;
  root_children.push_back(std::move(column));
  cgpui::StyledElement root(
      cgpui::Style{}
          .with_margin(cgpui::EdgeSizes::all(4.0F))
          .with_padding(cgpui::EdgeSizes::all(10.0F)),
      std::move(root_children));
  root.assign_id(ElementId{1});

  (void)root.layout(cgpui::LayoutInput{});
  state.set_offset(Point{0.0F, 8.0F});
  (void)root.layout(cgpui::LayoutInput{});
  if (state.offset().y != 8.0F) return 10;

  cgpui::PaintList paint_list;
  root.paint(paint_list);
  if (!same_rect(painted_rect(paint_list, kSpacer), Rect{{14, 14}, {100, 8}})) return 11;
  if (!same_rect(painted_rect(paint_list, kFirstRow), Rect{{21, 27}, {40, 16}})) return 12;
  if (!same_rect(painted_rect(paint_list, kSecondRow), Rect{{21, 47}, {40, 16}})) return 13;

  const auto regions = paint_list.window_control_regions();
  if (regions.size() != 1 || !same_rect(regions[0].bounds, Rect{{14, 74}, {30, 6}})) {
    return 14;
  }

  if (root.hit_test(Point{25.0F, 50.0F}) != ElementId{5}) return 20;
  if (root.hit_test(Point{16.0F, 16.0F}) != ElementId{3}) return 21;
  if (root.hit_test(Point{82.0F, 60.0F}) != ElementId{4}) return 22;
  if (root.hit_test(Point{125.0F, 80.0F}) != ElementId{1}) return 23;
  return 0;
}

} // namespace

namespace {

// A tree child positioned by its parent-relative layout origin.
class OffsetElement final : public cgpui::Element {
 public:
  OffsetElement(Point origin, Size size, Color color)
      : origin_(origin), size_(size), color_(color) {}

  [[nodiscard]] cgpui::LayoutOutput layout(cgpui::LayoutInput input) const override {
    (void)input;
    set_layout_bounds(Rect{.origin = origin_, .size = size_});
    return cgpui::LayoutOutput{.origin = origin_, .size = size_};
  }

  void paint(cgpui::PaintList& paint_list) const override {
    if (layout_bounds().has_value()) paint_list.fill_rect(*layout_bounds(), color_);
  }

  [[nodiscard]] cgpui::AccessibilityRole accessibility_role() const override {
    return cgpui::AccessibilityRole::button;
  }

 private:
  Point origin_;
  Size size_;
  Color color_;
};

// ElementTree children: root (padding 10, scrolled list inside) > panel at
// (10, 10) > group at (20, 5) > button at (7, 9) => button at (37, 24).
int test_element_tree_children_use_window_coordinates() {
  cgpui::ElementTree tree;
  const ElementId root = tree.set_root(std::make_unique<cgpui::StyledElement>(
      cgpui::Style{}
          .with_preferred_size(Size{200.0F, 200.0F})
          .with_padding(cgpui::EdgeSizes::all(10.0F)),
      std::unique_ptr<cgpui::Element>{}));
  const ElementId panel = tree.append_child(
      root, std::make_unique<OffsetElement>(Point{10, 10}, Size{150, 150}, kSpacer));
  const ElementId group = tree.append_child(
      panel, std::make_unique<OffsetElement>(Point{20, 5}, Size{100, 100}, kFirstRow));
  const ElementId button = tree.append_child(
      group, std::make_unique<OffsetElement>(Point{7, 9}, Size{30, 20}, kSecondRow));
  (void)tree.layout_root(cgpui::LayoutInput{});

  if (!same_rect(tree.get(button)->layout_bounds(), Rect{{37, 24}, {30, 20}})) return 30;
  cgpui::PaintList paint_list;
  tree.paint(paint_list);
  if (!same_rect(painted_rect(paint_list, kFirstRow), Rect{{30, 15}, {100, 100}})) return 31;
  if (!same_rect(painted_rect(paint_list, kSecondRow), Rect{{37, 24}, {30, 20}})) return 32;

  if (tree.hit_test_root(Point{40.0F, 30.0F}) != button) return 40;
  if (tree.hit_test_root(Point{32.0F, 17.0F}) != group) return 41;
  if (tree.hit_test_root(Point{12.0F, 12.0F}) != panel) return 42;
  if (tree.hit_test_root(Point{190.0F, 190.0F}) != root) return 43;

  const cgpui::AccessibilityTreeSnapshot snapshot = tree.accessibility_snapshot();
  const cgpui::AccessibilityNode* node = snapshot.node(button);
  if (node == nullptr || !same_rect(node->bounds, Rect{{37, 24}, {30, 20}})) return 50;
  node = snapshot.node(group);
  if (node == nullptr || !same_rect(node->bounds, Rect{{30, 15}, {100, 100}})) return 51;

  // A second layout pass keeps the same window coordinates.
  (void)tree.layout_root(cgpui::LayoutInput{});
  if (!same_rect(tree.get(button)->layout_bounds(), Rect{{37, 24}, {30, 20}})) return 52;
  return 0;
}

} // namespace

int main() {
  if (const int result = test_nested_containers_use_window_coordinates(); result != 0) {
    return result;
  }
  return test_element_tree_children_use_window_coordinates();
}
