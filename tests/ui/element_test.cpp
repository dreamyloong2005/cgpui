#include "cgpui/ui/element.hpp"
#include "cgpui/ui/style.hpp"
#include "cgpui/ui/text.hpp"
#include "cgpui/ui/ui.hpp"

#include <concepts>
#include <memory>
#include <span>

namespace {

class TestElement final : public cgpui::Element {};

class NamedElement final : public cgpui::Element {
 public:
  explicit NamedElement(int value) : value_(value) {}

  [[nodiscard]] int value() const {
    return value_;
  }

 private:
  int value_ = 0;
};

class EventCountingElement final : public cgpui::Element {
 public:
  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::ElementEventContext& context) override {
    event_count += 1;
    last_target = context.target_element_id;
    saw_pointer =
        saw_pointer || std::holds_alternative<cgpui::PointerMoved>(event);
    return result;
  }

  int event_count = 0;
  cgpui::ElementId last_target;
  bool saw_pointer = false;
  cgpui::EventResult result = cgpui::EventResult::unhandled();
};

static_assert(std::same_as<decltype(cgpui::ElementId{}.value), std::uint64_t>);
static_assert(std::equality_comparable<cgpui::ElementId>);

int test_element_id_defaults_to_invalid() {
  const cgpui::ElementId id{};
  return id.value == 0 ? 0 : 1;
}

int test_element_stores_assigned_id() {
  TestElement element;
  if (element.id().value != 0) {
    return 2;
  }

  element.assign_id(cgpui::ElementId{42});
  if (element.id() != cgpui::ElementId{42}) {
    return 3;
  }

  element.assign_id(cgpui::ElementId{43});
  return element.id() == cgpui::ElementId{43} ? 0 : 4;
}

int test_element_is_polymorphic() {
  std::unique_ptr<cgpui::Element> element = std::make_unique<TestElement>();
  element->assign_id(cgpui::ElementId{7});
  return element->id() == cgpui::ElementId{7} ? 0 : 5;
}

int test_base_element_lays_out_zero_size() {
  TestElement element;
  const cgpui::LayoutOutput output = element.layout(cgpui::LayoutInput{
      .constraints =
          {
              .min_size = {.width = 10.0F, .height = 20.0F},
              .max_size = {.width = 100.0F, .height = 200.0F},
          },
  });

  if (output.origin.x != 0.0F || output.origin.y != 0.0F) {
    return 27;
  }
  return output.size.width == 10.0F && output.size.height == 20.0F ? 0 : 28;
}

int test_fixed_size_element_lays_out_preferred_size() {
  cgpui::FixedSizeElement element(cgpui::Size{.width = 42.0F, .height = 24.0F});
  const cgpui::LayoutOutput output = element.layout(cgpui::LayoutInput{});

  if (output.origin.x != 0.0F || output.origin.y != 0.0F) {
    return 29;
  }
  return output.size.width == 42.0F && output.size.height == 24.0F ? 0 : 30;
}

int test_fixed_size_element_layout_applies_constraints() {
  cgpui::FixedSizeElement element(
      cgpui::Size{.width = 120.0F, .height = 5.0F});
  const cgpui::LayoutOutput output = element.layout(cgpui::LayoutInput{
      .constraints =
          {
              .min_size = {.width = 20.0F, .height = 10.0F},
              .max_size = {.width = 80.0F, .height = 60.0F},
          },
  });

  return output.size.width == 80.0F && output.size.height == 10.0F ? 0 : 31;
}

int test_fixed_size_element_records_layout_bounds() {
  cgpui::FixedSizeElement element(cgpui::Size{.width = 42.0F, .height = 24.0F});
  if (element.layout_bounds().has_value()) {
    return 38;
  }

  const cgpui::LayoutOutput output = element.layout(cgpui::LayoutInput{});
  const std::optional<cgpui::Rect> bounds = element.layout_bounds();
  if (!bounds.has_value()) {
    return 39;
  }
  if (output.size.width != 42.0F || output.size.height != 24.0F ||
      bounds->origin.x != 0.0F || bounds->origin.y != 0.0F) {
    return 40;
  }
  return bounds->size.width == 42.0F && bounds->size.height == 24.0F ? 0 : 41;
}

int test_fixed_size_element_hit_tests_layout_bounds() {
  cgpui::FixedSizeElement element(cgpui::Size{.width = 42.0F, .height = 24.0F});
  element.assign_id(cgpui::ElementId{77});

  if (element.hit_test(cgpui::Point{.x = 1.0F, .y = 1.0F}).value != 0) {
    return 42;
  }

  const cgpui::LayoutOutput output = element.layout(cgpui::LayoutInput{});
  if (output.size.width != 42.0F || output.size.height != 24.0F) {
    return 53;
  }
  if (element.hit_test(cgpui::Point{.x = 10.0F, .y = 10.0F}) !=
      cgpui::ElementId{77}) {
    return 43;
  }
  if (element.hit_test(cgpui::Point{.x = 42.0F, .y = 10.0F}).value != 0 ||
      element.hit_test(cgpui::Point{.x = 10.0F, .y = 24.0F}).value != 0 ||
      element.hit_test(cgpui::Point{.x = -1.0F, .y = 10.0F}).value != 0) {
    return 44;
  }
  return 0;
}

int test_vertical_stack_defaults_to_empty_constrained_zero_size() {
  cgpui::VerticalStackElement stack;
  const cgpui::LayoutOutput output = stack.layout(cgpui::LayoutInput{
      .constraints =
          {
              .min_size = {.width = 3.0F, .height = 4.0F},
              .max_size = {.width = 100.0F, .height = 100.0F},
          },
  });

  if (stack.children().size() != 0 || output.origin.x != 0.0F ||
      output.origin.y != 0.0F) {
    return 32;
  }
  return output.size.width == 3.0F && output.size.height == 4.0F ? 0 : 33;
}

int test_vertical_stack_lays_out_children_top_to_bottom() {
  cgpui::VerticalStackElement stack;
  stack.append_child(
      std::make_unique<cgpui::FixedSizeElement>(cgpui::Size{.width = 40.0F,
                                                            .height = 10.0F}));
  stack.append_child(
      std::make_unique<cgpui::FixedSizeElement>(cgpui::Size{.width = 20.0F,
                                                            .height = 30.0F}));

  if (stack.children().size() != 2) {
    return 34;
  }

  const cgpui::LayoutOutput output = stack.layout(cgpui::LayoutInput{});
  if (output.origin.x != 0.0F || output.origin.y != 0.0F) {
    return 35;
  }
  return output.size.width == 40.0F && output.size.height == 40.0F ? 0 : 36;
}

int test_vertical_stack_layout_applies_stack_constraints() {
  cgpui::VerticalStackElement stack;
  stack.append_child(
      std::make_unique<cgpui::FixedSizeElement>(cgpui::Size{.width = 120.0F,
                                                            .height = 15.0F}));
  stack.append_child(
      std::make_unique<cgpui::FixedSizeElement>(cgpui::Size{.width = 30.0F,
                                                            .height = 90.0F}));

  const cgpui::LayoutOutput output = stack.layout(cgpui::LayoutInput{
      .constraints =
          {
              .min_size = {.width = 20.0F, .height = 10.0F},
              .max_size = {.width = 80.0F, .height = 60.0F},
          },
  });

  return output.size.width == 80.0F && output.size.height == 60.0F ? 0 : 37;
}

int test_vertical_stack_records_child_bounds_top_to_bottom() {
  cgpui::VerticalStackElement stack;
  stack.append_child(
      std::make_unique<cgpui::FixedSizeElement>(cgpui::Size{.width = 40.0F,
                                                            .height = 10.0F}));
  stack.append_child(
      std::make_unique<cgpui::FixedSizeElement>(cgpui::Size{.width = 20.0F,
                                                            .height = 30.0F}));

  const cgpui::LayoutOutput output = stack.layout(cgpui::LayoutInput{});
  if (output.size.width != 40.0F || output.size.height != 40.0F) {
    return 54;
  }
  const std::optional<cgpui::Rect> stack_bounds = stack.layout_bounds();
  const std::optional<cgpui::Rect> first_bounds =
      stack.children()[0]->layout_bounds();
  const std::optional<cgpui::Rect> second_bounds =
      stack.children()[1]->layout_bounds();

  if (!stack_bounds.has_value() || !first_bounds.has_value() ||
      !second_bounds.has_value()) {
    return 45;
  }
  if (stack_bounds->origin.x != 0.0F || stack_bounds->origin.y != 0.0F ||
      stack_bounds->size.width != 40.0F ||
      stack_bounds->size.height != 40.0F) {
    return 46;
  }
  if (first_bounds->origin.x != 0.0F || first_bounds->origin.y != 0.0F ||
      first_bounds->size.width != 40.0F ||
      first_bounds->size.height != 10.0F) {
    return 47;
  }
  return second_bounds->origin.x == 0.0F && second_bounds->origin.y == 10.0F &&
                 second_bounds->size.width == 20.0F &&
                 second_bounds->size.height == 30.0F
             ? 0
             : 48;
}

int test_vertical_stack_gap_spaces_children_only_between_items() {
  std::unique_ptr<cgpui::Element> element =
      cgpui::ElementBuilder::v_stack()
          .style(cgpui::Style{}.with_gap(4.0F))
          .child(std::make_unique<cgpui::FixedSizeElement>(
              cgpui::Size{.width = 40.0F, .height = 10.0F}))
          .child(std::make_unique<cgpui::FixedSizeElement>(
              cgpui::Size{.width = 20.0F, .height = 30.0F}))
          .build();

  auto* stack = dynamic_cast<cgpui::VerticalStackElement*>(element.get());
  if (stack == nullptr || stack->gap() != 4.0F) {
    return 105;
  }

  const cgpui::LayoutOutput output = stack->layout(cgpui::LayoutInput{});
  if (output.size.width != 40.0F || output.size.height != 44.0F) {
    return 106;
  }

  const std::optional<cgpui::Rect> second_bounds =
      stack->children()[1]->layout_bounds();
  return second_bounds.has_value() && second_bounds->origin.x == 0.0F &&
                 second_bounds->origin.y == 14.0F &&
                 second_bounds->size.width == 20.0F &&
                 second_bounds->size.height == 30.0F
             ? 0
             : 107;
}

int test_vertical_stack_hit_tests_children_before_self() {
  cgpui::VerticalStackElement stack;
  stack.assign_id(cgpui::ElementId{10});
  auto first = std::make_unique<cgpui::FixedSizeElement>(
      cgpui::Size{.width = 40.0F, .height = 10.0F});
  first->assign_id(cgpui::ElementId{11});
  auto second = std::make_unique<cgpui::FixedSizeElement>(
      cgpui::Size{.width = 20.0F, .height = 30.0F});
  second->assign_id(cgpui::ElementId{12});
  stack.append_child(std::move(first));
  stack.append_child(std::move(second));

  const cgpui::LayoutOutput output = stack.layout(cgpui::LayoutInput{});
  if (output.size.width != 40.0F || output.size.height != 40.0F) {
    return 55;
  }
  if (stack.hit_test(cgpui::Point{.x = 5.0F, .y = 5.0F}) !=
      cgpui::ElementId{11}) {
    return 49;
  }
  if (stack.hit_test(cgpui::Point{.x = 5.0F, .y = 15.0F}) !=
      cgpui::ElementId{12}) {
    return 50;
  }
  if (stack.hit_test(cgpui::Point{.x = 30.0F, .y = 15.0F}) !=
      cgpui::ElementId{10}) {
    return 51;
  }
  if (stack.hit_test(cgpui::Point{.x = 41.0F, .y = 5.0F}).value != 0) {
    return 52;
  }
  return 0;
}

int test_flex_row_lays_out_children_left_to_right() {
  cgpui::FlexElement flex(cgpui::FlexDirection::row);
  flex.append_child(
      std::make_unique<cgpui::FixedSizeElement>(cgpui::Size{.width = 12.0F,
                                                            .height = 20.0F}));
  flex.append_child(
      std::make_unique<cgpui::FixedSizeElement>(cgpui::Size{.width = 30.0F,
                                                            .height = 8.0F}));

  const cgpui::LayoutOutput output = flex.layout(cgpui::LayoutInput{});
  if (flex.direction() != cgpui::FlexDirection::row ||
      flex.children().size() != 2 || output.size.width != 42.0F ||
      output.size.height != 20.0F) {
    return 79;
  }

  const std::optional<cgpui::Rect> first_bounds =
      flex.children()[0]->layout_bounds();
  const std::optional<cgpui::Rect> second_bounds =
      flex.children()[1]->layout_bounds();
  if (!first_bounds.has_value() || !second_bounds.has_value()) {
    return 80;
  }
  if (first_bounds->origin.x != 0.0F || first_bounds->origin.y != 0.0F ||
      first_bounds->size.width != 12.0F ||
      first_bounds->size.height != 20.0F) {
    return 81;
  }
  return second_bounds->origin.x == 12.0F &&
                 second_bounds->origin.y == 0.0F &&
                 second_bounds->size.width == 30.0F &&
                 second_bounds->size.height == 8.0F
             ? 0
             : 82;
}

int test_flex_row_gap_spaces_children_on_main_axis() {
  std::unique_ptr<cgpui::Element> element =
      cgpui::ElementBuilder::row()
          .style(cgpui::Style{}.with_gap(4.0F))
          .child(std::make_unique<cgpui::FixedSizeElement>(
              cgpui::Size{.width = 12.0F, .height = 20.0F}))
          .child(std::make_unique<cgpui::FixedSizeElement>(
              cgpui::Size{.width = 30.0F, .height = 8.0F}))
          .build();

  auto* flex = dynamic_cast<cgpui::FlexElement*>(element.get());
  if (flex == nullptr || flex->direction() != cgpui::FlexDirection::row ||
      flex->gap() != 4.0F) {
    return 108;
  }

  const cgpui::LayoutOutput output = flex->layout(cgpui::LayoutInput{});
  if (output.size.width != 46.0F || output.size.height != 20.0F) {
    return 109;
  }

  const std::optional<cgpui::Rect> second_bounds =
      flex->children()[1]->layout_bounds();
  return second_bounds.has_value() && second_bounds->origin.x == 16.0F &&
                 second_bounds->origin.y == 0.0F &&
                 second_bounds->size.width == 30.0F &&
                 second_bounds->size.height == 8.0F
             ? 0
             : 110;
}

int test_flex_column_lays_out_children_top_to_bottom() {
  cgpui::FlexElement flex(cgpui::FlexDirection::column);
  flex.append_child(
      std::make_unique<cgpui::FixedSizeElement>(cgpui::Size{.width = 12.0F,
                                                            .height = 20.0F}));
  flex.append_child(
      std::make_unique<cgpui::FixedSizeElement>(cgpui::Size{.width = 30.0F,
                                                            .height = 8.0F}));

  const cgpui::LayoutOutput output = flex.layout(cgpui::LayoutInput{
      .constraints =
          {
              .min_size = {.width = 10.0F, .height = 10.0F},
              .max_size = {.width = 24.0F, .height = 100.0F},
          },
  });
  if (flex.direction() != cgpui::FlexDirection::column ||
      output.size.width != 24.0F || output.size.height != 28.0F) {
    return 83;
  }

  const std::optional<cgpui::Rect> second_bounds =
      flex.children()[1]->layout_bounds();
  return second_bounds.has_value() && second_bounds->origin.x == 0.0F &&
                 second_bounds->origin.y == 20.0F &&
                 second_bounds->size.width == 30.0F &&
                 second_bounds->size.height == 8.0F
             ? 0
             : 84;
}

int test_flex_column_gap_spaces_children_on_main_axis() {
  std::unique_ptr<cgpui::Element> element =
      cgpui::ElementBuilder::column()
          .style(cgpui::Style{}.with_gap(6.0F))
          .child(std::make_unique<cgpui::FixedSizeElement>(
              cgpui::Size{.width = 12.0F, .height = 20.0F}))
          .child(std::make_unique<cgpui::FixedSizeElement>(
              cgpui::Size{.width = 30.0F, .height = 8.0F}))
          .build();

  auto* flex = dynamic_cast<cgpui::FlexElement*>(element.get());
  if (flex == nullptr || flex->direction() != cgpui::FlexDirection::column ||
      flex->gap() != 6.0F) {
    return 111;
  }

  const cgpui::LayoutOutput output = flex->layout(cgpui::LayoutInput{});
  if (output.size.width != 30.0F || output.size.height != 34.0F) {
    return 112;
  }

  const std::optional<cgpui::Rect> second_bounds =
      flex->children()[1]->layout_bounds();
  return second_bounds.has_value() && second_bounds->origin.x == 0.0F &&
                 second_bounds->origin.y == 26.0F &&
                 second_bounds->size.width == 30.0F &&
                 second_bounds->size.height == 8.0F
             ? 0
             : 113;
}

int test_flex_hit_tests_children_before_self() {
  cgpui::FlexElement flex(cgpui::FlexDirection::row);
  flex.assign_id(cgpui::ElementId{20});
  auto first = std::make_unique<cgpui::FixedSizeElement>(
      cgpui::Size{.width = 10.0F, .height = 20.0F});
  first->assign_id(cgpui::ElementId{21});
  auto second = std::make_unique<cgpui::FixedSizeElement>(
      cgpui::Size{.width = 20.0F, .height = 10.0F});
  second->assign_id(cgpui::ElementId{22});
  flex.append_child(std::move(first));
  flex.append_child(std::move(second));

  const cgpui::LayoutOutput output = flex.layout(cgpui::LayoutInput{});
  if (output.size.width != 30.0F || output.size.height != 20.0F) {
    return 85;
  }
  if (flex.hit_test(cgpui::Point{.x = 5.0F, .y = 5.0F}) !=
      cgpui::ElementId{21}) {
    return 86;
  }
  if (flex.hit_test(cgpui::Point{.x = 15.0F, .y = 5.0F}) !=
      cgpui::ElementId{22}) {
    return 87;
  }
  if (flex.hit_test(cgpui::Point{.x = 15.0F, .y = 15.0F}) !=
      cgpui::ElementId{20}) {
    return 88;
  }
  return flex.hit_test(cgpui::Point{.x = 31.0F, .y = 5.0F}).value == 0 ? 0
                                                                       : 89;
}

int test_element_tree_stores_root_and_children() {
  cgpui::ElementTree tree;

  const cgpui::ElementId root_id =
      tree.set_root(std::make_unique<NamedElement>(10));
  if (root_id.value == 0 || tree.root_id() != root_id) {
    return 6;
  }

  const auto* root = dynamic_cast<const NamedElement*>(tree.get(root_id));
  if (root == nullptr || root->value() != 10 || root->id() != root_id) {
    return 7;
  }

  const cgpui::ElementId first_child_id =
      tree.append_child(root_id, std::make_unique<NamedElement>(20));
  const cgpui::ElementId second_child_id =
      tree.append_child(root_id, std::make_unique<NamedElement>(30));
  if (first_child_id.value <= root_id.value ||
      second_child_id.value <= first_child_id.value) {
    return 8;
  }

  const std::span<const cgpui::ElementId> children = tree.children(root_id);
  if (children.size() != 2 || children[0] != first_child_id ||
      children[1] != second_child_id) {
    return 9;
  }

  if (!tree.parent(first_child_id).has_value() ||
      *tree.parent(first_child_id) != root_id ||
      !tree.parent(second_child_id).has_value() ||
      *tree.parent(second_child_id) != root_id ||
      tree.parent(root_id).has_value()) {
    return 10;
  }

  auto* second_child = dynamic_cast<NamedElement*>(tree.get(second_child_id));
  if (second_child == nullptr || second_child->value() != 30 ||
      second_child->id() != second_child_id) {
    return 11;
  }

  return 0;
}

int test_element_tree_rejects_unknown_parent() {
  cgpui::ElementTree tree;
  const cgpui::ElementId root_id =
      tree.set_root(std::make_unique<NamedElement>(1));
  const cgpui::ElementId missing =
      tree.append_child(cgpui::ElementId{999}, std::make_unique<NamedElement>(2));
  if (missing.value != 0) {
    return 12;
  }
  if (tree.children(cgpui::ElementId{999}).size() != 0 ||
      tree.parent(cgpui::ElementId{999}).has_value() ||
      tree.get(cgpui::ElementId{999}) != nullptr) {
    return 13;
  }
  return tree.children(root_id).size() == 0 ? 0 : 14;
}

int test_element_tree_replaces_root_with_fresh_tree() {
  cgpui::ElementTree tree;
  const cgpui::ElementId old_root =
      tree.set_root(std::make_unique<NamedElement>(1));
  const cgpui::ElementId old_child =
      tree.append_child(old_root, std::make_unique<NamedElement>(2));

  const cgpui::ElementId new_root =
      tree.set_root(std::make_unique<NamedElement>(3));
  if (new_root.value <= old_child.value || tree.root_id() != new_root) {
    return 15;
  }
  if (tree.get(old_root) != nullptr || tree.get(old_child) != nullptr ||
      tree.children(old_root).size() != 0 ||
      tree.parent(old_child).has_value()) {
    return 16;
  }

  const auto* root = dynamic_cast<const NamedElement*>(tree.get(new_root));
  return root != nullptr && root->value() == 3 ? 0 : 17;
}

int test_element_tree_reconciles_root_in_place() {
  cgpui::ElementTree tree;
  const cgpui::ElementId root_id =
      tree.set_root(std::make_unique<NamedElement>(1));
  const cgpui::ElementId reconciled_root_id =
      tree.reconcile_root(std::make_unique<NamedElement>(10));

  if (reconciled_root_id != root_id || tree.root_id() != root_id) {
    return 18;
  }

  const auto* root = dynamic_cast<const NamedElement*>(tree.get(root_id));
  return root != nullptr && root->value() == 10 && root->id() == root_id
      ? 0
      : 19;
}

int test_element_tree_reconciles_children_by_index() {
  cgpui::ElementTree tree;
  const cgpui::ElementId root_id =
      tree.reconcile_root(std::make_unique<NamedElement>(1));
  const cgpui::ElementId first_child_id =
      tree.reconcile_child(root_id, 0, std::make_unique<NamedElement>(2));
  const cgpui::ElementId second_child_id =
      tree.reconcile_child(root_id, 1, std::make_unique<NamedElement>(3));

  const cgpui::ElementId reconciled_first_child_id =
      tree.reconcile_child(root_id, 0, std::make_unique<NamedElement>(20));
  const cgpui::ElementId reconciled_second_child_id =
      tree.reconcile_child(root_id, 1, std::make_unique<NamedElement>(30));
  if (reconciled_first_child_id != first_child_id ||
      reconciled_second_child_id != second_child_id) {
    return 20;
  }

  const std::span<const cgpui::ElementId> children = tree.children(root_id);
  if (children.size() != 2 || children[0] != first_child_id ||
      children[1] != second_child_id) {
    return 21;
  }

  const auto* first_child =
      dynamic_cast<const NamedElement*>(tree.get(first_child_id));
  const auto* second_child =
      dynamic_cast<const NamedElement*>(tree.get(second_child_id));
  if (first_child == nullptr || first_child->value() != 20 ||
      first_child->id() != first_child_id || second_child == nullptr ||
      second_child->value() != 30 || second_child->id() != second_child_id) {
    return 22;
  }

  if (!tree.parent(first_child_id).has_value() ||
      *tree.parent(first_child_id) != root_id ||
      !tree.parent(second_child_id).has_value() ||
      *tree.parent(second_child_id) != root_id) {
    return 23;
  }

  return 0;
}

int test_element_tree_reconcile_appends_new_child_index() {
  cgpui::ElementTree tree;
  const cgpui::ElementId root_id =
      tree.reconcile_root(std::make_unique<NamedElement>(1));
  const cgpui::ElementId first_child_id =
      tree.reconcile_child(root_id, 0, std::make_unique<NamedElement>(2));
  const cgpui::ElementId second_child_id =
      tree.reconcile_child(root_id, 1, std::make_unique<NamedElement>(3));

  if (second_child_id.value <= first_child_id.value) {
    return 24;
  }
  const std::span<const cgpui::ElementId> children = tree.children(root_id);
  return children.size() == 2 && children[0] == first_child_id &&
          children[1] == second_child_id
      ? 0
      : 25;
}

int test_element_tree_reconcile_rejects_unknown_parent() {
  cgpui::ElementTree tree;
  const cgpui::ElementId missing = tree.reconcile_child(
      cgpui::ElementId{123},
      0,
      std::make_unique<NamedElement>(1));
  return missing.value == 0 && tree.get(cgpui::ElementId{123}) == nullptr ? 0
                                                                          : 26;
}

int test_element_builder_creates_styled_box() {
  const cgpui::Style style =
      cgpui::Style{}
          .with_background_color(
              cgpui::Color{.r = 0.2F, .g = 0.3F, .b = 0.4F, .a = 1.0F})
          .with_preferred_size(cgpui::Size{.width = 64.0F, .height = 32.0F});
  std::unique_ptr<cgpui::Element> element =
      cgpui::ElementBuilder::box().style(style).build();

  const auto* styled = dynamic_cast<const cgpui::StyledElement*>(element.get());
  if (styled == nullptr) {
    return 56;
  }
  if (!styled->style().background_color.has_value() ||
      styled->style().background_color->r != 0.2F ||
      styled->style().preferred_size.width != 64.0F ||
      styled->style().preferred_size.height != 32.0F) {
    return 57;
  }
  if (styled->child() != nullptr) {
    return 58;
  }

  return 0;
}

int test_element_builder_wraps_child() {
  std::unique_ptr<cgpui::Element> element =
      cgpui::ElementBuilder::box()
          .style(cgpui::Style{}.with_padding(cgpui::EdgeSizes::all(3.0F)))
          .child(std::make_unique<cgpui::FixedSizeElement>(
              cgpui::Size{.width = 11.0F, .height = 12.0F}))
          .build();

  auto* styled = dynamic_cast<cgpui::StyledElement*>(element.get());
  if (styled == nullptr || styled->child() == nullptr) {
    return 59;
  }
  if (styled->style().padding.left != 3.0F ||
      styled->style().padding.top != 3.0F) {
    return 60;
  }

  const auto* fixed =
      dynamic_cast<const cgpui::FixedSizeElement*>(styled->child());
  if (fixed == nullptr || fixed->preferred_size().width != 11.0F ||
      fixed->preferred_size().height != 12.0F) {
    return 61;
  }

  return 0;
}

int test_element_builder_builds_flex_row_with_children() {
  std::unique_ptr<cgpui::Element> element =
      cgpui::ElementBuilder::row()
          .child(std::make_unique<cgpui::FixedSizeElement>(
              cgpui::Size{.width = 12.0F, .height = 20.0F}))
          .child(std::make_unique<cgpui::FixedSizeElement>(
              cgpui::Size{.width = 30.0F, .height = 8.0F}))
          .build();

  auto* flex = dynamic_cast<cgpui::FlexElement*>(element.get());
  if (flex == nullptr || flex->direction() != cgpui::FlexDirection::row ||
      flex->children().size() != 2) {
    return 97;
  }

  const cgpui::LayoutOutput output = flex->layout(cgpui::LayoutInput{});
  return output.size.width == 42.0F && output.size.height == 20.0F ? 0 : 98;
}

int test_element_builder_builds_flex_column_with_children() {
  std::unique_ptr<cgpui::Element> element =
      cgpui::ElementBuilder::column()
          .child(std::make_unique<cgpui::FixedSizeElement>(
              cgpui::Size{.width = 12.0F, .height = 20.0F}))
          .child(std::make_unique<cgpui::FixedSizeElement>(
              cgpui::Size{.width = 30.0F, .height = 8.0F}))
          .build();

  auto* flex = dynamic_cast<cgpui::FlexElement*>(element.get());
  if (flex == nullptr || flex->direction() != cgpui::FlexDirection::column ||
      flex->children().size() != 2) {
    return 99;
  }

  const cgpui::LayoutOutput output = flex->layout(cgpui::LayoutInput{});
  return output.size.width == 30.0F && output.size.height == 28.0F ? 0 : 100;
}

int test_element_builder_builds_vertical_stack_with_children() {
  std::unique_ptr<cgpui::Element> element =
      cgpui::ElementBuilder::v_stack()
          .child(std::make_unique<cgpui::FixedSizeElement>(
              cgpui::Size{.width = 12.0F, .height = 20.0F}))
          .child(std::make_unique<cgpui::FixedSizeElement>(
              cgpui::Size{.width = 30.0F, .height = 8.0F}))
          .build();

  auto* stack = dynamic_cast<cgpui::VerticalStackElement*>(element.get());
  if (stack == nullptr || stack->children().size() != 2) {
    return 101;
  }

  const cgpui::LayoutOutput output = stack->layout(cgpui::LayoutInput{});
  return output.size.width == 30.0F && output.size.height == 28.0F ? 0 : 102;
}

int test_element_builder_builds_fixed_size_leaf() {
  std::unique_ptr<cgpui::Element> element =
      cgpui::ElementBuilder::fixed_size(
          cgpui::Size{.width = 44.0F, .height = 18.0F})
          .build();

  auto* fixed = dynamic_cast<cgpui::FixedSizeElement*>(element.get());
  if (fixed == nullptr || fixed->preferred_size().width != 44.0F ||
      fixed->preferred_size().height != 18.0F) {
    return 103;
  }

  const cgpui::LayoutOutput output = fixed->layout(cgpui::LayoutInput{
      .constraints =
          {
              .max_size = {.width = 20.0F, .height = 30.0F},
          },
  });
  return output.size.width == 20.0F && output.size.height == 18.0F ? 0 : 104;
}

int test_styled_element_paints_background_rect_from_layout_bounds() {
  const cgpui::Color color{
      .r = 0.1F,
      .g = 0.2F,
      .b = 0.3F,
      .a = 0.4F,
  };
  std::unique_ptr<cgpui::Element> element =
      cgpui::ElementBuilder::box()
          .style(cgpui::Style{}
                     .with_background_color(color)
                     .with_preferred_size(
                         cgpui::Size{.width = 70.0F, .height = 30.0F}))
          .build();

  const cgpui::LayoutOutput output = element->layout(cgpui::LayoutInput{});
  if (output.size.width != 70.0F || output.size.height != 30.0F) {
    return 62;
  }

  cgpui::PaintList paint_list;
  element->paint(paint_list);

  const std::span<const cgpui::PaintCommand> commands = paint_list.commands();
  if (commands.size() != 1) {
    return 63;
  }

  const cgpui::SolidRect& rect = commands[0].solid_rect;
  if (rect.rect.origin.x != 0.0F || rect.rect.origin.y != 0.0F ||
      rect.rect.size.width != 70.0F || rect.rect.size.height != 30.0F) {
    return 64;
  }
  return rect.color.r == color.r && rect.color.g == color.g &&
                 rect.color.b == color.b && rect.color.a == color.a
             ? 0
             : 65;
}

int test_styled_element_without_background_paints_nothing() {
  std::unique_ptr<cgpui::Element> element =
      cgpui::ElementBuilder::box()
          .style(cgpui::Style{}.with_preferred_size(
              cgpui::Size{.width = 40.0F, .height = 20.0F}))
          .build();
  (void)element->layout(cgpui::LayoutInput{});

  cgpui::PaintList paint_list;
  element->paint(paint_list);
  return paint_list.commands().empty() ? 0 : 66;
}

int test_styled_element_paints_border_rects_after_background() {
  const cgpui::Color background{
      .r = 0.1F,
      .g = 0.2F,
      .b = 0.3F,
      .a = 1.0F,
  };
  const cgpui::Color border{
      .r = 0.8F,
      .g = 0.7F,
      .b = 0.6F,
      .a = 1.0F,
  };
  std::unique_ptr<cgpui::Element> element =
      cgpui::ElementBuilder::box()
          .style(cgpui::Style{}
                     .with_background_color(background)
                     .with_border_color(border)
                     .with_border_width(
                         cgpui::EdgeSizes::trbl(2.0F, 3.0F, 4.0F, 5.0F))
                     .with_preferred_size(
                         cgpui::Size{.width = 80.0F, .height = 40.0F}))
          .build();

  const cgpui::LayoutOutput output = element->layout(cgpui::LayoutInput{});
  if (output.size.width != 80.0F || output.size.height != 40.0F) {
    return 119;
  }

  cgpui::PaintList paint_list;
  element->paint(paint_list);

  const std::span<const cgpui::PaintCommand> commands = paint_list.commands();
  if (commands.size() != 5) {
    return 120;
  }
  if (commands[0].solid_rect.color.r != background.r ||
      commands[1].solid_rect.color.r != border.r ||
      commands[4].solid_rect.color.g != border.g) {
    return 121;
  }

  const cgpui::Rect top = commands[1].solid_rect.rect;
  const cgpui::Rect right = commands[2].solid_rect.rect;
  const cgpui::Rect bottom = commands[3].solid_rect.rect;
  const cgpui::Rect left = commands[4].solid_rect.rect;
  if (top.origin.x != 0.0F || top.origin.y != 0.0F ||
      top.size.width != 80.0F || top.size.height != 2.0F) {
    return 122;
  }
  if (right.origin.x != 77.0F || right.origin.y != 2.0F ||
      right.size.width != 3.0F || right.size.height != 34.0F) {
    return 123;
  }
  if (bottom.origin.x != 0.0F || bottom.origin.y != 36.0F ||
      bottom.size.width != 80.0F || bottom.size.height != 4.0F) {
    return 124;
  }
  return left.origin.x == 0.0F && left.origin.y == 2.0F &&
                 left.size.width == 5.0F && left.size.height == 34.0F
             ? 0
             : 125;
}

int test_styled_element_skips_border_without_color() {
  std::unique_ptr<cgpui::Element> element =
      cgpui::ElementBuilder::box()
          .style(cgpui::Style{}
                     .with_border_width(cgpui::EdgeSizes::all(4.0F))
                     .with_preferred_size(
                         cgpui::Size{.width = 30.0F, .height = 20.0F}))
          .build();
  (void)element->layout(cgpui::LayoutInput{});

  cgpui::PaintList paint_list;
  element->paint(paint_list);
  return paint_list.commands().empty() ? 0 : 126;
}

int test_paint_list_attaches_current_clip_to_commands() {
  cgpui::PaintList paint_list;
  const cgpui::Rect clip{
      .origin = {.x = 2.0F, .y = 3.0F},
      .size = {.width = 40.0F, .height = 20.0F},
  };

  paint_list.fill_rect(
      cgpui::Rect{.size = {.width = 10.0F, .height = 10.0F}},
      cgpui::Color{.r = 0.1F, .a = 1.0F});
  paint_list.push_clip(clip);
  paint_list.fill_rect(
      cgpui::Rect{.size = {.width = 20.0F, .height = 10.0F}},
      cgpui::Color{.g = 0.2F, .a = 1.0F});
  paint_list.pop_clip();
  paint_list.fill_rect(
      cgpui::Rect{.size = {.width = 30.0F, .height = 10.0F}},
      cgpui::Color{.b = 0.3F, .a = 1.0F});

  const std::span<const cgpui::PaintCommand> commands = paint_list.commands();
  if (commands.size() != 3) {
    return 127;
  }
  if (commands[0].clip_rect.has_value() || commands[2].clip_rect.has_value()) {
    return 128;
  }
  return commands[1].clip_rect.has_value() &&
                 commands[1].clip_rect->origin.x == 2.0F &&
                 commands[1].clip_rect->origin.y == 3.0F &&
                 commands[1].clip_rect->size.width == 40.0F &&
                 commands[1].clip_rect->size.height == 20.0F
             ? 0
             : 129;
}

int test_styled_element_hidden_overflow_attaches_bounds_clip_metadata() {
  const cgpui::Color background{.r = 0.1F, .a = 1.0F};
  const cgpui::Color border{.g = 0.2F, .a = 1.0F};
  std::unique_ptr<cgpui::Element> element =
      cgpui::ElementBuilder::box()
          .style(cgpui::Style{}
                     .with_background_color(background)
                     .with_border_color(border)
                     .with_border_width(cgpui::EdgeSizes::all(2.0F))
                     .with_overflow(cgpui::Overflow::hidden)
                     .with_preferred_size(
                         cgpui::Size{.width = 50.0F, .height = 30.0F}))
          .child(cgpui::ElementBuilder::box()
                     .style(cgpui::Style{}
                                .with_background_color(
                                    cgpui::Color{.b = 0.3F, .a = 1.0F})
                                .with_preferred_size(
                                    cgpui::Size{.width = 20.0F,
                                                .height = 10.0F}))
                     .build())
          .build();

  const cgpui::LayoutOutput output = element->layout(cgpui::LayoutInput{});
  if (output.size.width != 20.0F || output.size.height != 10.0F) {
    return 130;
  }
  cgpui::PaintList paint_list;
  element->paint(paint_list);

  const std::span<const cgpui::PaintCommand> commands = paint_list.commands();
  if (commands.size() != 6) {
    return 131;
  }
  for (const cgpui::PaintCommand& command : commands) {
    if (!command.clip_rect.has_value() ||
        command.clip_rect->origin.x != 0.0F ||
        command.clip_rect->origin.y != 0.0F ||
        command.clip_rect->size.width != 20.0F ||
        command.clip_rect->size.height != 10.0F) {
      return 132;
    }
  }

  return commands[5].solid_rect.color.b == 0.3F ? 0 : 133;
}

int test_styled_element_hidden_overflow_uses_explicit_clip_rect_metadata() {
  const cgpui::Rect clip{
      .origin = {.x = 4.0F, .y = 5.0F},
      .size = {.width = 12.0F, .height = 8.0F},
  };
  std::unique_ptr<cgpui::Element> element =
      cgpui::ElementBuilder::box()
          .style(cgpui::Style{}
                     .with_background_color(cgpui::Color{.r = 0.1F, .a = 1.0F})
                     .with_overflow(cgpui::Overflow::hidden)
                     .with_clip_rect(clip)
                     .with_preferred_size(
                         cgpui::Size{.width = 30.0F, .height = 20.0F}))
          .build();

  (void)element->layout(cgpui::LayoutInput{});
  cgpui::PaintList paint_list;
  element->paint(paint_list);

  const std::span<const cgpui::PaintCommand> commands = paint_list.commands();
  return commands.size() == 1 && commands[0].clip_rect.has_value() &&
                 commands[0].clip_rect->origin.x == 4.0F &&
                 commands[0].clip_rect->origin.y == 5.0F &&
                 commands[0].clip_rect->size.width == 12.0F &&
                 commands[0].clip_rect->size.height == 8.0F
             ? 0
             : 134;
}

int test_text_element_binds_text_model_and_lays_out_skeleton() {
  cgpui::TextModel model("hello");
  cgpui::TextElement element(&model);
  element.assign_id(cgpui::ElementId{44});

  if (element.model() != &model) {
    return 135;
  }
  if (element.text() != model.text()) {
    return 136;
  }

  const cgpui::LayoutOutput output = element.layout(cgpui::LayoutInput{});
  if (output.size.width != 40.0F || output.size.height != 16.0F) {
    return 137;
  }

  const std::optional<cgpui::Rect> bounds = element.layout_bounds();
  if (!bounds.has_value() || bounds->size.width != 40.0F ||
      bounds->size.height != 16.0F) {
    return 138;
  }

  return element.hit_test(cgpui::Point{.x = 4.0F, .y = 8.0F}) ==
                 cgpui::ElementId{44}
             ? 0
             : 139;
}

int test_text_element_paints_text_placeholder_from_layout_bounds() {
  cgpui::TextModel model("hi");
  cgpui::TextElement element(&model);
  (void)element.layout(cgpui::LayoutInput{});

  cgpui::PaintList paint_list;
  element.paint(paint_list);
  const std::span<const cgpui::PaintCommand> commands = paint_list.commands();
  if (commands.size() != 1) {
    return 140;
  }
  if (commands[0].solid_rect.rect.size.width != 16.0F ||
      commands[0].solid_rect.rect.size.height != 16.0F) {
    return 141;
  }

  return commands[0].solid_rect.color.a > 0.0F ? 0 : 142;
}

int test_element_builder_builds_text_leaf() {
  cgpui::TextModel model("builder");
  std::unique_ptr<cgpui::Element> element =
      cgpui::ElementBuilder::text(model).build();

  auto* text = dynamic_cast<cgpui::TextElement*>(element.get());
  if (text == nullptr || text->model() != &model ||
      text->text() != model.text()) {
    return 143;
  }

  const cgpui::LayoutOutput output = text->layout(cgpui::LayoutInput{});
  return output.size.width == 56.0F && output.size.height == 16.0F ? 0 : 144;
}

int test_styled_element_layout_includes_padding_without_child() {
  std::unique_ptr<cgpui::Element> element =
      cgpui::ElementBuilder::box()
          .style(cgpui::Style{}
                     .with_preferred_size(
                         cgpui::Size{.width = 20.0F, .height = 10.0F})
                     .with_padding(
                         cgpui::EdgeSizes::trbl(1.0F, 2.0F, 3.0F, 4.0F)))
          .build();

  const cgpui::LayoutOutput output = element->layout(cgpui::LayoutInput{});
  if (output.size.width != 26.0F || output.size.height != 14.0F) {
    return 67;
  }

  const std::optional<cgpui::Rect> bounds = element->layout_bounds();
  return bounds.has_value() && bounds->size.width == 26.0F &&
                 bounds->size.height == 14.0F
             ? 0
             : 68;
}

int test_styled_element_layout_offsets_child_by_padding() {
  std::unique_ptr<cgpui::Element> element =
      cgpui::ElementBuilder::box()
          .style(cgpui::Style{}.with_padding(
              cgpui::EdgeSizes::trbl(2.0F, 3.0F, 4.0F, 5.0F)))
          .child(std::make_unique<cgpui::FixedSizeElement>(
              cgpui::Size{.width = 30.0F, .height = 15.0F}))
          .build();

  const cgpui::LayoutOutput output = element->layout(cgpui::LayoutInput{});
  if (output.size.width != 38.0F || output.size.height != 21.0F) {
    return 69;
  }

  const auto* styled = dynamic_cast<const cgpui::StyledElement*>(element.get());
  if (styled == nullptr || styled->child() == nullptr) {
    return 70;
  }

  const std::optional<cgpui::Rect> child_bounds =
      styled->child()->layout_bounds();
  if (!child_bounds.has_value()) {
    return 71;
  }

  return child_bounds->origin.x == 5.0F && child_bounds->origin.y == 2.0F &&
                 child_bounds->size.width == 30.0F &&
                 child_bounds->size.height == 15.0F
             ? 0
             : 72;
}

int test_styled_element_layout_includes_margin_in_outer_size() {
  std::unique_ptr<cgpui::Element> element =
      cgpui::ElementBuilder::box()
          .style(cgpui::Style{}
                     .with_preferred_size(
                         cgpui::Size{.width = 20.0F, .height = 10.0F})
                     .with_padding(
                         cgpui::EdgeSizes::trbl(1.0F, 2.0F, 3.0F, 4.0F))
                     .with_margin(
                         cgpui::EdgeSizes::trbl(5.0F, 6.0F, 7.0F, 8.0F)))
          .build();

  const cgpui::LayoutOutput output = element->layout(cgpui::LayoutInput{});
  if (output.size.width != 40.0F || output.size.height != 26.0F) {
    return 114;
  }

  const std::optional<cgpui::Rect> bounds = element->layout_bounds();
  return bounds.has_value() && bounds->size.width == 40.0F &&
                 bounds->size.height == 26.0F
             ? 0
             : 115;
}

int test_styled_element_margin_offsets_child_outside_padding() {
  std::unique_ptr<cgpui::Element> element =
      cgpui::ElementBuilder::box()
          .style(cgpui::Style{}
                     .with_padding(
                         cgpui::EdgeSizes::trbl(2.0F, 3.0F, 4.0F, 5.0F))
                     .with_margin(
                         cgpui::EdgeSizes::trbl(7.0F, 11.0F, 13.0F, 17.0F)))
          .child(std::make_unique<cgpui::FixedSizeElement>(
              cgpui::Size{.width = 30.0F, .height = 15.0F}))
          .build();

  const cgpui::LayoutOutput output = element->layout(cgpui::LayoutInput{});
  if (output.size.width != 66.0F || output.size.height != 41.0F) {
    return 116;
  }

  const auto* styled = dynamic_cast<const cgpui::StyledElement*>(element.get());
  if (styled == nullptr || styled->child() == nullptr) {
    return 117;
  }

  const std::optional<cgpui::Rect> child_bounds =
      styled->child()->layout_bounds();
  return child_bounds.has_value() && child_bounds->origin.x == 22.0F &&
                 child_bounds->origin.y == 9.0F &&
                 child_bounds->size.width == 30.0F &&
                 child_bounds->size.height == 15.0F
             ? 0
             : 118;
}

int test_element_tree_paints_root_and_children_in_tree_order() {
  cgpui::ElementTree tree;
  const cgpui::Color root_color{
      .r = 0.1F,
      .g = 0.0F,
      .b = 0.0F,
      .a = 1.0F,
  };
  const cgpui::Color first_color{
      .r = 0.0F,
      .g = 0.2F,
      .b = 0.0F,
      .a = 1.0F,
  };
  const cgpui::Color second_color{
      .r = 0.0F,
      .g = 0.0F,
      .b = 0.3F,
      .a = 1.0F,
  };

  const cgpui::ElementId root_id = tree.set_root(
      cgpui::ElementBuilder::box()
          .style(cgpui::Style{}
                     .with_background_color(root_color)
                     .with_preferred_size(
                         cgpui::Size{.width = 80.0F, .height = 40.0F}))
          .build());
  const cgpui::ElementId first_child_id = tree.append_child(
      root_id,
      cgpui::ElementBuilder::box()
          .style(cgpui::Style{}
                     .with_background_color(first_color)
                     .with_preferred_size(
                         cgpui::Size{.width = 20.0F, .height = 10.0F}))
          .build());
  const cgpui::ElementId second_child_id = tree.append_child(
      root_id,
      cgpui::ElementBuilder::box()
          .style(cgpui::Style{}
                     .with_background_color(second_color)
                     .with_preferred_size(
                         cgpui::Size{.width = 30.0F, .height = 15.0F}))
          .build());

  tree.get(root_id)->set_layout_bounds(cgpui::Rect{
      .origin = {.x = 0.0F, .y = 0.0F},
      .size = {.width = 80.0F, .height = 40.0F},
  });
  tree.get(first_child_id)->set_layout_bounds(cgpui::Rect{
      .origin = {.x = 4.0F, .y = 5.0F},
      .size = {.width = 20.0F, .height = 10.0F},
  });
  tree.get(second_child_id)->set_layout_bounds(cgpui::Rect{
      .origin = {.x = 6.0F, .y = 7.0F},
      .size = {.width = 30.0F, .height = 15.0F},
  });

  cgpui::PaintList paint_list;
  tree.paint(paint_list);
  const std::span<const cgpui::PaintCommand> commands = paint_list.commands();
  if (commands.size() != 3) {
    return 73;
  }
  if (commands[0].solid_rect.color.r != root_color.r ||
      commands[1].solid_rect.color.g != first_color.g ||
      commands[2].solid_rect.color.b != second_color.b) {
    return 74;
  }
  if (commands[1].solid_rect.rect.origin.x != 4.0F ||
      commands[2].solid_rect.rect.origin.y != 7.0F) {
    return 75;
  }

  return 0;
}

int test_element_tree_paints_siblings_by_stable_z_order() {
  cgpui::ElementTree tree;
  const cgpui::Color root_color{
      .r = 0.1F,
      .g = 0.0F,
      .b = 0.0F,
      .a = 1.0F,
  };
  const cgpui::Color low_color{
      .r = 0.2F,
      .g = 0.0F,
      .b = 0.0F,
      .a = 1.0F,
  };
  const cgpui::Color middle_first_color{
      .r = 0.3F,
      .g = 0.0F,
      .b = 0.0F,
      .a = 1.0F,
  };
  const cgpui::Color middle_second_color{
      .r = 0.4F,
      .g = 0.0F,
      .b = 0.0F,
      .a = 1.0F,
  };
  const cgpui::Color high_color{
      .r = 0.5F,
      .g = 0.0F,
      .b = 0.0F,
      .a = 1.0F,
  };

  const cgpui::ElementId root_id = tree.set_root(
      cgpui::ElementBuilder::box()
          .style(cgpui::Style{}
                     .with_background_color(root_color)
                     .with_preferred_size(
                         cgpui::Size{.width = 80.0F, .height = 40.0F}))
          .build());
  const cgpui::ElementId high_id = tree.append_child(
      root_id,
      cgpui::ElementBuilder::box()
          .style(cgpui::Style{}
                     .with_background_color(high_color)
                     .with_z_index(10)
                     .with_preferred_size(
                         cgpui::Size{.width = 20.0F, .height = 10.0F}))
          .build());
  const cgpui::ElementId middle_first_id = tree.append_child(
      root_id,
      cgpui::ElementBuilder::box()
          .style(cgpui::Style{}
                     .with_background_color(middle_first_color)
                     .with_z_index(5)
                     .with_preferred_size(
                         cgpui::Size{.width = 20.0F, .height = 10.0F}))
          .build());
  const cgpui::ElementId low_id = tree.append_child(
      root_id,
      cgpui::ElementBuilder::box()
          .style(cgpui::Style{}
                     .with_background_color(low_color)
                     .with_z_index(-1)
                     .with_preferred_size(
                         cgpui::Size{.width = 20.0F, .height = 10.0F}))
          .build());
  const cgpui::ElementId middle_second_id = tree.append_child(
      root_id,
      cgpui::ElementBuilder::box()
          .style(cgpui::Style{}
                     .with_background_color(middle_second_color)
                     .with_z_index(5)
                     .with_preferred_size(
                         cgpui::Size{.width = 20.0F, .height = 10.0F}))
          .build());

  for (const cgpui::ElementId id :
       {root_id, high_id, middle_first_id, low_id, middle_second_id}) {
    tree.get(id)->set_layout_bounds(cgpui::Rect{
        .origin = {.x = 0.0F, .y = 0.0F},
        .size = {.width = 20.0F, .height = 10.0F},
    });
  }

  cgpui::PaintList paint_list;
  tree.paint(paint_list);
  const std::span<const cgpui::PaintCommand> commands = paint_list.commands();
  if (commands.size() != 5) {
    return 76;
  }
  if (commands[0].solid_rect.color.r != root_color.r) {
    return 77;
  }
  if (commands[1].solid_rect.color.r != low_color.r ||
      commands[2].solid_rect.color.r != middle_first_color.r ||
      commands[3].solid_rect.color.r != middle_second_color.r ||
      commands[4].solid_rect.color.r != high_color.r) {
    return 78;
  }

  return 0;
}

int test_element_tree_lays_out_root_element() {
  cgpui::ElementTree tree;
  const cgpui::ElementId root_id = tree.set_root(
      std::make_unique<cgpui::FixedSizeElement>(
          cgpui::Size{.width = 120.0F, .height = 30.0F}));

  const cgpui::LayoutOutput output = tree.layout_root(cgpui::LayoutInput{
      .constraints =
          {
              .min_size = {.width = 20.0F, .height = 10.0F},
              .max_size = {.width = 80.0F, .height = 60.0F},
          },
  });

  if (output.size.width != 80.0F || output.size.height != 30.0F) {
    return 90;
  }

  const cgpui::Element* root = tree.get(root_id);
  if (root == nullptr || !root->layout_bounds().has_value()) {
    return 91;
  }
  return root->layout_bounds()->size.width == 80.0F &&
                 root->layout_bounds()->size.height == 30.0F
             ? 0
             : 92;
}

int test_element_tree_layout_root_handles_empty_tree() {
  cgpui::ElementTree tree;
  const cgpui::LayoutOutput output = tree.layout_root(cgpui::LayoutInput{
      .constraints =
          {
              .min_size = {.width = 12.0F, .height = 8.0F},
              .max_size = {.width = 80.0F, .height = 60.0F},
          },
  });

  return output.size.width == 12.0F && output.size.height == 8.0F ? 0 : 93;
}

int test_element_tree_hit_tests_from_root() {
  cgpui::ElementTree tree;
  const cgpui::ElementId root_id =
      tree.set_root(std::make_unique<cgpui::FixedSizeElement>(
          cgpui::Size{.width = 50.0F, .height = 30.0F}));
  (void)tree.layout_root(cgpui::LayoutInput{});

  if (tree.hit_test_root(cgpui::Point{.x = 10.0F, .y = 10.0F}) != root_id) {
    return 94;
  }
  return tree.hit_test_root(cgpui::Point{.x = 60.0F, .y = 10.0F}).value == 0
      ? 0
      : 95;
}

int test_element_tree_hit_test_root_handles_empty_tree() {
  cgpui::ElementTree tree;
  return tree.hit_test_root(cgpui::Point{.x = 1.0F, .y = 1.0F}).value == 0
      ? 0
      : 96;
}

int test_base_element_event_handler_defaults_to_unhandled() {
  TestElement element;
  element.assign_id(cgpui::ElementId{30});

  const cgpui::EventResult result = element.handle_event(
      cgpui::PointerMoved{.position = {.x = 1.0F, .y = 2.0F}},
      cgpui::ElementEventContext{
          .target_element_id = element.id(),
      });

  return !result.consumed && !result.cancelled ? 0 : 97;
}

int test_styled_element_forwards_events_to_child() {
  auto child = std::make_unique<EventCountingElement>();
  EventCountingElement* child_ptr = child.get();
  child_ptr->assign_id(cgpui::ElementId{32});
  child_ptr->result = cgpui::EventResult::consumed_event();

  cgpui::StyledElement element(cgpui::Style{}, std::move(child));
  element.assign_id(cgpui::ElementId{31});

  const cgpui::EventResult result = element.handle_event(
      cgpui::PointerMoved{.position = {.x = 2.0F, .y = 3.0F}},
      cgpui::ElementEventContext{
          .target_element_id = child_ptr->id(),
      });

  if (!result.consumed || result.cancelled) {
    return 98;
  }
  if (child_ptr->event_count != 1 || !child_ptr->saw_pointer ||
      child_ptr->last_target != cgpui::ElementId{32}) {
    return 99;
  }

  return 0;
}

} // namespace

int main() {
  if (const int result = test_element_id_defaults_to_invalid(); result != 0) {
    return result;
  }
  if (const int result = test_element_stores_assigned_id(); result != 0) {
    return result;
  }
  if (const int result = test_element_is_polymorphic(); result != 0) {
    return result;
  }
  if (const int result = test_base_element_lays_out_zero_size(); result != 0) {
    return result;
  }
  if (const int result = test_fixed_size_element_lays_out_preferred_size();
      result != 0) {
    return result;
  }
  if (const int result = test_fixed_size_element_layout_applies_constraints();
      result != 0) {
    return result;
  }
  if (const int result = test_fixed_size_element_records_layout_bounds();
      result != 0) {
    return result;
  }
  if (const int result = test_fixed_size_element_hit_tests_layout_bounds();
      result != 0) {
    return result;
  }
  if (const int result =
          test_vertical_stack_defaults_to_empty_constrained_zero_size();
      result != 0) {
    return result;
  }
  if (const int result = test_vertical_stack_lays_out_children_top_to_bottom();
      result != 0) {
    return result;
  }
  if (const int result = test_vertical_stack_layout_applies_stack_constraints();
      result != 0) {
    return result;
  }
  if (const int result =
          test_vertical_stack_records_child_bounds_top_to_bottom();
      result != 0) {
    return result;
  }
  if (const int result =
          test_vertical_stack_gap_spaces_children_only_between_items();
      result != 0) {
    return result;
  }
  if (const int result = test_vertical_stack_hit_tests_children_before_self();
      result != 0) {
    return result;
  }
  if (const int result = test_flex_row_lays_out_children_left_to_right();
      result != 0) {
    return result;
  }
  if (const int result = test_flex_row_gap_spaces_children_on_main_axis();
      result != 0) {
    return result;
  }
  if (const int result = test_flex_column_lays_out_children_top_to_bottom();
      result != 0) {
    return result;
  }
  if (const int result = test_flex_column_gap_spaces_children_on_main_axis();
      result != 0) {
    return result;
  }
  if (const int result = test_flex_hit_tests_children_before_self();
      result != 0) {
    return result;
  }
  if (const int result = test_element_tree_stores_root_and_children();
      result != 0) {
    return result;
  }
  if (const int result = test_element_tree_rejects_unknown_parent();
      result != 0) {
    return result;
  }
  if (const int result = test_element_tree_replaces_root_with_fresh_tree();
      result != 0) {
    return result;
  }
  if (const int result = test_element_tree_reconciles_root_in_place();
      result != 0) {
    return result;
  }
  if (const int result = test_element_tree_reconciles_children_by_index();
      result != 0) {
    return result;
  }
  if (const int result = test_element_tree_reconcile_appends_new_child_index();
      result != 0) {
    return result;
  }
  if (const int result = test_element_tree_reconcile_rejects_unknown_parent();
      result != 0) {
    return result;
  }
  if (const int result = test_element_builder_creates_styled_box();
      result != 0) {
    return result;
  }
  if (const int result = test_element_builder_wraps_child(); result != 0) {
    return result;
  }
  if (const int result = test_element_builder_builds_flex_row_with_children();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_builder_builds_flex_column_with_children();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_builder_builds_vertical_stack_with_children();
      result != 0) {
    return result;
  }
  if (const int result = test_element_builder_builds_fixed_size_leaf();
      result != 0) {
    return result;
  }
  if (const int result =
          test_styled_element_paints_background_rect_from_layout_bounds();
      result != 0) {
    return result;
  }
  if (const int result = test_styled_element_without_background_paints_nothing();
      result != 0) {
    return result;
  }
  if (const int result =
          test_styled_element_paints_border_rects_after_background();
      result != 0) {
    return result;
  }
  if (const int result = test_styled_element_skips_border_without_color();
      result != 0) {
    return result;
  }
  if (const int result = test_paint_list_attaches_current_clip_to_commands();
      result != 0) {
    return result;
  }
  if (const int result =
          test_styled_element_hidden_overflow_attaches_bounds_clip_metadata();
      result != 0) {
    return result;
  }
  if (const int result =
          test_styled_element_hidden_overflow_uses_explicit_clip_rect_metadata();
      result != 0) {
    return result;
  }
  if (const int result =
          test_text_element_binds_text_model_and_lays_out_skeleton();
      result != 0) {
    return result;
  }
  if (const int result =
          test_text_element_paints_text_placeholder_from_layout_bounds();
      result != 0) {
    return result;
  }
  if (const int result = test_element_builder_builds_text_leaf();
      result != 0) {
    return result;
  }
  if (const int result =
          test_styled_element_layout_includes_padding_without_child();
      result != 0) {
    return result;
  }
  if (const int result = test_styled_element_layout_offsets_child_by_padding();
      result != 0) {
    return result;
  }
  if (const int result =
          test_styled_element_layout_includes_margin_in_outer_size();
      result != 0) {
    return result;
  }
  if (const int result =
          test_styled_element_margin_offsets_child_outside_padding();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_tree_paints_root_and_children_in_tree_order();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_tree_paints_siblings_by_stable_z_order();
      result != 0) {
    return result;
  }
  if (const int result = test_element_tree_lays_out_root_element();
      result != 0) {
    return result;
  }
  if (const int result = test_element_tree_layout_root_handles_empty_tree();
      result != 0) {
    return result;
  }
  if (const int result = test_element_tree_hit_tests_from_root();
      result != 0) {
    return result;
  }
  if (const int result = test_element_tree_hit_test_root_handles_empty_tree();
      result != 0) {
    return result;
  }
  if (const int result = test_base_element_event_handler_defaults_to_unhandled();
      result != 0) {
    return result;
  }
  if (const int result = test_styled_element_forwards_events_to_child();
      result != 0) {
    return result;
  }
  return 0;
}
