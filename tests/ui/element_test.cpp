#include "cgpui/ui/element.hpp"
#include "cgpui/ui/scroll.hpp"
#include "cgpui/ui/style.hpp"
#include "cgpui/ui/text.hpp"
#include "cgpui/ui/ui.hpp"
#include "paint_snapshot.hpp"

#include <concepts>
#include <memory>
#include <span>
#include <string>
#include <vector>

namespace {

bool same_transform(cgpui::AffineTransform lhs, cgpui::AffineTransform rhs) {
  return lhs.scale_x == rhs.scale_x && lhs.skew_y == rhs.skew_y &&
         lhs.skew_x == rhs.skew_x && lhs.scale_y == rhs.scale_y &&
         lhs.translate_x == rhs.translate_x &&
         lhs.translate_y == rhs.translate_y;
}

bool same_shadow(const cgpui::BoxShadow& lhs, const cgpui::BoxShadow& rhs) {
  return lhs.color.r == rhs.color.r && lhs.color.g == rhs.color.g &&
         lhs.color.b == rhs.color.b && lhs.color.a == rhs.color.a &&
         lhs.offset.x == rhs.offset.x && lhs.offset.y == rhs.offset.y &&
         lhs.blur_radius == rhs.blur_radius &&
         lhs.spread_radius == rhs.spread_radius;
}

bool same_rect(cgpui::Rect lhs, cgpui::Rect rhs) {
  return lhs.origin.x == rhs.origin.x && lhs.origin.y == rhs.origin.y &&
         lhs.size.width == rhs.size.width &&
         lhs.size.height == rhs.size.height;
}

class TestElement final : public cgpui::Element {};

class MutableSizeElement final : public cgpui::Element {
 public:
  explicit MutableSizeElement(cgpui::Size size) : size_(size) {}

  void set_size(cgpui::Size size) {
    size_ = size;
  }

  [[nodiscard]] cgpui::LayoutOutput layout(
      cgpui::LayoutInput input) const override {
    (void)input;
    return cgpui::LayoutOutput{.size = size_};
  }

 private:
  cgpui::Size size_;
};

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

class FocusableCountingElement final : public cgpui::Element {
 public:
  [[nodiscard]] bool focusable() const override {
    return true;
  }

  void focus(const cgpui::ElementFocusContext& context) override {
    focus_count += 1;
    last_focused_element_id = context.element_id;
  }

  int focus_count = 0;
  cgpui::ElementId last_focused_element_id;
};

struct LifecycleCounters {
  int mounts = 0;
  int updates = 0;
  int unmounts = 0;
  cgpui::ElementId last_mounted;
  cgpui::ElementId last_updated;
  cgpui::ElementId last_unmounted;
};

class LifecycleCountingElement final : public cgpui::Element {
 public:
  explicit LifecycleCountingElement(LifecycleCounters& counters)
      : counters_(&counters) {}

  void on_mount(const cgpui::ElementLifecycleContext& context) override {
    counters_->mounts += 1;
    counters_->last_mounted = context.element_id;
  }

  void on_update(const cgpui::ElementLifecycleContext& context) override {
    counters_->updates += 1;
    counters_->last_updated = context.element_id;
  }

  void on_unmount(const cgpui::ElementLifecycleContext& context) override {
    counters_->unmounts += 1;
    counters_->last_unmounted = context.element_id;
  }

 private:
  LifecycleCounters* counters_ = nullptr;
};

struct WidgetState {
  WidgetState(int value, std::string name)
      : value(value),
        name(std::move(name)) {}

  int value = 0;
  std::string name;
};

struct AlternateWidgetState {
  int value = 0;
};

static_assert(std::same_as<decltype(cgpui::ElementId{}.value), std::uint64_t>);
static_assert(std::equality_comparable<cgpui::ElementId>);
static_assert(std::equality_comparable<cgpui::ElementKey>);
static_assert(std::same_as<cgpui::AnyElement, std::unique_ptr<cgpui::Element>>);

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

int test_any_element_into_element_preserves_owned_element() {
  cgpui::AnyElement element =
      cgpui::into_element(std::make_unique<NamedElement>(9));
  auto* named = dynamic_cast<NamedElement*>(element.get());

  if (named == nullptr || named->value() != 9) {
    return 184;
  }

  element->assign_id(cgpui::ElementId{44});
  return element->id() == cgpui::ElementId{44} ? 0 : 185;
}

int test_element_builder_into_element_builds_owned_element() {
  cgpui::AnyElement element = cgpui::into_element(
      cgpui::ElementBuilder::fixed_size(
          cgpui::Size{.width = 12.0F, .height = 34.0F}));
  auto* fixed = dynamic_cast<cgpui::FixedSizeElement*>(element.get());

  if (fixed == nullptr) {
    return 186;
  }

  const cgpui::LayoutOutput output = fixed->layout(cgpui::LayoutInput{});
  return output.size.width == 12.0F && output.size.height == 34.0F ? 0 : 187;
}

int test_free_authoring_factories_create_builders() {
  cgpui::TextModel model("factory");
  cgpui::AnyElement div = cgpui::into_element(cgpui::div());
  cgpui::AnyElement row = cgpui::into_element(cgpui::h_flex());
  cgpui::AnyElement column = cgpui::into_element(cgpui::v_flex());
  cgpui::AnyElement stack = cgpui::into_element(cgpui::v_stack());
  cgpui::AnyElement text = cgpui::into_element(cgpui::text(model));

  auto* styled = dynamic_cast<cgpui::StyledElement*>(div.get());
  auto* row_flex = dynamic_cast<cgpui::FlexElement*>(row.get());
  auto* column_flex = dynamic_cast<cgpui::FlexElement*>(column.get());
  auto* vertical_stack = dynamic_cast<cgpui::VerticalStackElement*>(stack.get());
  auto* text_element = dynamic_cast<cgpui::TextElement*>(text.get());

  if (styled == nullptr || styled->child() != nullptr) {
    return 188;
  }
  if (row_flex == nullptr ||
      row_flex->direction() != cgpui::FlexDirection::row) {
    return 189;
  }
  if (column_flex == nullptr ||
      column_flex->direction() != cgpui::FlexDirection::column) {
    return 190;
  }
  if (vertical_stack == nullptr || vertical_stack->children().size() != 0) {
    return 191;
  }
  return text_element != nullptr && text_element->model() == &model &&
                 text_element->text() == "factory"
             ? 0
             : 192;
}

int test_free_authoring_factories_compose_with_existing_builder_methods() {
  cgpui::AnyElement element =
      cgpui::into_element(cgpui::h_flex()
                              .style(cgpui::Style{}.with_gap(5.0F))
                              .child(cgpui::into_element(
                                  cgpui::ElementBuilder::fixed_size(
                                      cgpui::Size{.width = 10.0F,
                                                  .height = 4.0F})))
                              .child(cgpui::into_element(
                                  cgpui::ElementBuilder::fixed_size(
                                      cgpui::Size{.width = 20.0F,
                                                  .height = 8.0F}))));
  auto* flex = dynamic_cast<cgpui::FlexElement*>(element.get());
  if (flex == nullptr || flex->gap() != 5.0F) {
    return 193;
  }

  const cgpui::LayoutOutput output = flex->layout(cgpui::LayoutInput{});
  if (output.size.width != 35.0F || output.size.height != 8.0F) {
    return 194;
  }

  const std::optional<cgpui::Rect> second_bounds =
      flex->children()[1]->layout_bounds();
  return second_bounds.has_value() && second_bounds->origin.x == 15.0F &&
                 second_bounds->size.width == 20.0F
             ? 0
             : 195;
}

int test_element_builder_child_accepts_builder() {
  cgpui::AnyElement element =
      cgpui::into_element(cgpui::h_flex()
                              .style(cgpui::Style{}.with_gap(3.0F))
                              .child(cgpui::div().style(
                                  cgpui::Style{}.with_preferred_size(
                                      cgpui::Size{.width = 6.0F,
                                                  .height = 4.0F})))
                              .child(cgpui::div().style(
                                  cgpui::Style{}.with_preferred_size(
                                      cgpui::Size{.width = 8.0F,
                                                  .height = 5.0F}))));
  auto* flex = dynamic_cast<cgpui::FlexElement*>(element.get());
  if (flex == nullptr || flex->children().size() != 2) {
    return 196;
  }

  const cgpui::LayoutOutput output = flex->layout(cgpui::LayoutInput{});
  if (output.size.width != 17.0F || output.size.height != 5.0F) {
    return 197;
  }

  const std::optional<cgpui::Rect> second_bounds =
      flex->children()[1]->layout_bounds();
  return second_bounds.has_value() && second_bounds->origin.x == 9.0F &&
                 second_bounds->size.width == 8.0F
             ? 0
             : 198;
}

int test_element_builder_child_accepts_any_element() {
  cgpui::AnyElement child =
      cgpui::into_element(cgpui::div().style(
          cgpui::Style{}.with_preferred_size(
              cgpui::Size{.width = 13.0F, .height = 7.0F})));
  cgpui::AnyElement element =
      cgpui::into_element(cgpui::v_stack().child(std::move(child)));

  auto* stack = dynamic_cast<cgpui::VerticalStackElement*>(element.get());
  if (stack == nullptr || stack->children().size() != 1) {
    return 199;
  }

  const cgpui::LayoutOutput output = stack->layout(cgpui::LayoutInput{});
  return output.size.width == 13.0F && output.size.height == 7.0F ? 0 : 200;
}

int test_element_builder_child_accepts_typed_element_ownership() {
  cgpui::AnyElement element =
      cgpui::into_element(cgpui::v_stack().child(
          std::make_unique<cgpui::FixedSizeElement>(
              cgpui::Size{.width = 21.0F, .height = 9.0F})));

  auto* stack = dynamic_cast<cgpui::VerticalStackElement*>(element.get());
  if (stack == nullptr || stack->children().size() != 1) {
    return 201;
  }

  const cgpui::LayoutOutput output = stack->layout(cgpui::LayoutInput{});
  return output.size.width == 21.0F && output.size.height == 9.0F ? 0 : 202;
}

int test_element_builder_fluent_style_shortcuts_mutate_box_style() {
  cgpui::AnyElement element =
      cgpui::into_element(cgpui::div()
                              .size(cgpui::Size{.width = cgpui::px(64.0F),
                                                 .height = cgpui::px(32.0F)})
                              .padding(cgpui::edges(1.0F, 2.0F, 3.0F, 4.0F))
                              .margin(cgpui::edges(5.0F, 6.0F))
                              .background(cgpui::rgb(10, 20, 30))
                              .foreground(cgpui::rgba(200, 210, 220, 0.5F))
                              .border_width(cgpui::edges(7.0F))
                              .border_color(cgpui::rgb(40, 50, 60))
                              .border_radius(cgpui::BorderRadii::corners(
                                  1.0F,
                                  2.0F,
                                  3.0F,
                                  4.0F)));
  const auto* styled = dynamic_cast<const cgpui::StyledElement*>(element.get());
  if (styled == nullptr) {
    return 203;
  }

  const cgpui::Style& style = styled->style();
  if (style.preferred_size.width != 64.0F ||
      style.preferred_size.height != 32.0F) {
    return 204;
  }
  if (style.padding.top != 1.0F || style.padding.right != 2.0F ||
      style.padding.bottom != 3.0F || style.padding.left != 4.0F) {
    return 205;
  }
  if (style.margin.left != 5.0F || style.margin.right != 5.0F ||
      style.margin.top != 6.0F || style.margin.bottom != 6.0F) {
    return 206;
  }
  if (!style.background_color.has_value() ||
      style.background_color->r != 10.0F / 255.0F ||
      !style.foreground_color.has_value() ||
      style.foreground_color->a != 0.5F) {
    return 207;
  }
  if (style.border_width.top != 7.0F || !style.border_color.has_value() ||
      style.border_color->g != 50.0F / 255.0F) {
    return 208;
  }
  return style.border_radius.top_left == 1.0F &&
                 style.border_radius.top_right == 2.0F &&
                 style.border_radius.bottom_right == 3.0F &&
                 style.border_radius.bottom_left == 4.0F
             ? 0
             : 209;
}

int test_element_builder_fluent_size_shortcut_accepts_dimensions() {
  cgpui::AnyElement styled_element =
      cgpui::into_element(cgpui::div().size(cgpui::px(20.0F), cgpui::px(10.0F)));
  const auto* styled =
      dynamic_cast<const cgpui::StyledElement*>(styled_element.get());
  if (styled == nullptr ||
      styled->style().preferred_size.width != 20.0F ||
      styled->style().preferred_size.height != 10.0F) {
    return 210;
  }

  cgpui::AnyElement fixed_element = cgpui::into_element(
      cgpui::ElementBuilder::fixed_size(
          cgpui::Size{.width = 1.0F, .height = 2.0F})
          .size(cgpui::px(30.0F), cgpui::px(40.0F)));
  const auto* fixed =
      dynamic_cast<const cgpui::FixedSizeElement*>(fixed_element.get());
  return fixed != nullptr && fixed->preferred_size().width == 30.0F &&
                 fixed->preferred_size().height == 40.0F
             ? 0
             : 211;
}

int test_element_builder_fluent_gap_shortcut_applies_to_stack_and_flex() {
  cgpui::AnyElement row =
      cgpui::into_element(cgpui::h_flex()
                              .gap(cgpui::px(4.0F))
                              .child(cgpui::div().size(10.0F, 3.0F))
                              .child(cgpui::div().size(20.0F, 5.0F)));
  auto* flex = dynamic_cast<cgpui::FlexElement*>(row.get());
  if (flex == nullptr || flex->gap() != 4.0F) {
    return 212;
  }
  const cgpui::LayoutOutput row_output = flex->layout(cgpui::LayoutInput{});
  if (row_output.size.width != 34.0F || row_output.size.height != 5.0F) {
    return 213;
  }

  cgpui::AnyElement stack =
      cgpui::into_element(cgpui::v_stack()
                              .gap(cgpui::px(6.0F))
                              .child(cgpui::div().size(10.0F, 3.0F))
                              .child(cgpui::div().size(20.0F, 5.0F)));
  auto* vertical_stack = dynamic_cast<cgpui::VerticalStackElement*>(stack.get());
  if (vertical_stack == nullptr || vertical_stack->gap() != 6.0F) {
    return 214;
  }
  const cgpui::LayoutOutput stack_output =
      vertical_stack->layout(cgpui::LayoutInput{});
  return stack_output.size.width == 20.0F &&
                 stack_output.size.height == 14.0F
             ? 0
             : 215;
}

int test_element_builder_flex_vocabulary_helpers_map_to_existing_style() {
  cgpui::AnyElement start_row =
      cgpui::into_element(cgpui::h_flex()
                              .align_items(cgpui::AlignItems::end)
                              .items_start()
                              .justify_content(cgpui::JustifyContent::end)
                              .justify_start());
  const auto* start_flex =
      dynamic_cast<const cgpui::FlexElement*>(start_row.get());
  if (start_flex == nullptr ||
      start_flex->align_items() != cgpui::AlignItems::start ||
      start_flex->justify_content() != cgpui::JustifyContent::start) {
    return 216;
  }

  cgpui::AnyElement centered_row =
      cgpui::into_element(cgpui::h_flex().items_center().justify_center());
  const auto* centered_flex =
      dynamic_cast<const cgpui::FlexElement*>(centered_row.get());
  if (centered_flex == nullptr ||
      centered_flex->align_items() != cgpui::AlignItems::center ||
      centered_flex->justify_content() != cgpui::JustifyContent::center) {
    return 217;
  }

  cgpui::AnyElement end_column =
      cgpui::into_element(cgpui::v_flex().items_end().justify_end());
  const auto* end_flex =
      dynamic_cast<const cgpui::FlexElement*>(end_column.get());
  if (end_flex == nullptr ||
      end_flex->align_items() != cgpui::AlignItems::end ||
      end_flex->justify_content() != cgpui::JustifyContent::end) {
    return 218;
  }

  cgpui::AnyElement styled_element =
      cgpui::into_element(cgpui::div()
                              .items_center()
                              .justify_between()
                              .flex_1());
  const auto* styled =
      dynamic_cast<const cgpui::StyledElement*>(styled_element.get());
  if (styled == nullptr ||
      styled->style().align_items != cgpui::AlignItems::center ||
      styled->style().justify_content !=
          cgpui::JustifyContent::space_between) {
    return 219;
  }
  return styled->flex_grow() == 1.0F && styled->flex_shrink() == 1.0F ? 0
                                                                      : 220;
}

int test_element_builder_sizing_color_border_vocabulary_helpers_map_to_existing_style() {
  cgpui::AnyElement element =
      cgpui::into_element(cgpui::div()
                              .w(cgpui::px(64.0F))
                              .h(cgpui::px(32.0F))
                              .bg(cgpui::rgb(10, 20, 30))
                              .text_color(cgpui::rgba(200, 210, 220, 0.5F))
                              .border_1()
                              .border_color(cgpui::rgb(40, 50, 60))
                              .rounded(cgpui::px(6.0F)));
  const auto* styled = dynamic_cast<const cgpui::StyledElement*>(element.get());
  if (styled == nullptr) {
    return 571;
  }

  const cgpui::Style& style = styled->style();
  if (style.preferred_size.width != 64.0F ||
      style.preferred_size.height != 32.0F ||
      style.border_width.top != 1.0F || style.border_width.right != 1.0F ||
      style.border_width.bottom != 1.0F || style.border_width.left != 1.0F) {
    return 572;
  }
  if (!style.background_color.has_value() ||
      style.background_color->r != 10.0F / 255.0F ||
      !style.foreground_color.has_value() ||
      style.foreground_color->a != 0.5F ||
      !style.border_color.has_value() ||
      style.border_color->g != 50.0F / 255.0F ||
      style.border_radius.top_left != 6.0F ||
      style.border_radius.top_right != 6.0F ||
      style.border_radius.bottom_right != 6.0F ||
      style.border_radius.bottom_left != 6.0F) {
    return 573;
  }

  cgpui::AnyElement fixed_element = cgpui::into_element(
      cgpui::ElementBuilder::fixed_size(
          cgpui::Size{.width = 11.0F, .height = 12.0F})
          .w(cgpui::px(13.0F))
          .h(cgpui::px(14.0F)));
  const auto* fixed =
      dynamic_cast<const cgpui::FixedSizeElement*>(fixed_element.get());
  return fixed != nullptr && fixed->preferred_size().width == 13.0F &&
                 fixed->preferred_size().height == 14.0F
             ? 0
             : 574;
}

int test_element_builder_layout_constraint_vocabulary_helpers_map_to_existing_style() {
  cgpui::AnyElement element =
      cgpui::into_element(cgpui::div()
                              .min_size(cgpui::Size{.width = 10.0F,
                                                     .height = 20.0F})
                              .max_size(cgpui::Size{.width = 200.0F,
                                                     .height = 220.0F})
                              .min_w(cgpui::px(30.0F))
                              .min_h(cgpui::px(40.0F))
                              .max_w(cgpui::px(100.0F))
                              .max_h(cgpui::px(120.0F)));
  const auto* styled = dynamic_cast<const cgpui::StyledElement*>(element.get());
  if (styled == nullptr) {
    return 587;
  }

  const cgpui::Style& style = styled->style();
  if (style.min_size.width != 30.0F ||
      style.min_size.height != 40.0F ||
      style.max_size.width != 100.0F ||
      style.max_size.height != 120.0F) {
    return 588;
  }

  cgpui::AnyElement constrained =
      cgpui::into_element(cgpui::div()
                              .size(5.0F, 200.0F)
                              .min_w(cgpui::px(30.0F))
                              .max_h(cgpui::px(80.0F)));
  const cgpui::LayoutOutput output =
      constrained->layout(cgpui::LayoutInput{});
  return output.size.width == 30.0F && output.size.height == 80.0F ? 0
                                                                   : 589;
}

int test_element_builder_percentage_sizing_helpers_map_to_existing_style() {
  cgpui::AnyElement element =
      cgpui::into_element(cgpui::div()
                              .size_pct(25.0F, 50.0F)
                              .w_pct(75.0F)
                              .h_pct(80.0F));
  const auto* styled = dynamic_cast<const cgpui::StyledElement*>(element.get());
  if (styled == nullptr) {
    return 592;
  }

  const cgpui::PercentageSize& percentage_size =
      styled->style().percentage_size;
  if (!percentage_size.width.has_value() ||
      !percentage_size.height.has_value() ||
      *percentage_size.width != 75.0F ||
      *percentage_size.height != 80.0F) {
    return 593;
  }

  const cgpui::LayoutOutput output = element->layout(cgpui::LayoutInput{
      .constraints =
          {
              .max_size = {.width = 200.0F, .height = 300.0F},
          },
  });
  return output.size.width == 150.0F && output.size.height == 240.0F ? 0
                                                                    : 594;
}

int test_element_builder_margin_padding_gap_shorthands_map_to_existing_layout() {
  cgpui::AnyElement axis_element =
      cgpui::into_element(cgpui::div()
                              .p(1.0F)
                              .px(2.0F)
                              .py(3.0F)
                              .m(4.0F)
                              .mx(5.0F)
                              .my(6.0F));
  const auto* axis_styled =
      dynamic_cast<const cgpui::StyledElement*>(axis_element.get());
  if (axis_styled == nullptr) {
    return 597;
  }
  const cgpui::Style& axis_style = axis_styled->style();
  if (axis_style.padding.top != 3.0F ||
      axis_style.padding.right != 2.0F ||
      axis_style.padding.bottom != 3.0F ||
      axis_style.padding.left != 2.0F ||
      axis_style.margin.top != 6.0F || axis_style.margin.right != 5.0F ||
      axis_style.margin.bottom != 6.0F ||
      axis_style.margin.left != 5.0F) {
    return 598;
  }

  cgpui::AnyElement edge_element =
      cgpui::into_element(cgpui::div()
                              .p(1.0F)
                              .pt(3.0F)
                              .pr(6.0F)
                              .pb(4.0F)
                              .pl(5.0F)
                              .m(7.0F)
                              .mt(9.0F)
                              .mr(8.0F)
                              .mb(10.0F)
                              .ml(8.0F)
                              .gap(11.0F)
                              .child(cgpui::div().size(20.0F, 5.0F))
                              .child(cgpui::div().size(10.0F, 7.0F)));
  const auto* edge_styled =
      dynamic_cast<const cgpui::StyledElement*>(edge_element.get());
  if (edge_styled == nullptr || edge_styled->children().size() != 2) {
    return 599;
  }

  const cgpui::LayoutOutput output = edge_element->layout(cgpui::LayoutInput{});
  if (output.size.width != 47.0F || output.size.height != 49.0F) {
    return 600;
  }

  const std::optional<cgpui::Rect> first_bounds =
      edge_styled->children()[0]->layout_bounds();
  const std::optional<cgpui::Rect> second_bounds =
      edge_styled->children()[1]->layout_bounds();
  return first_bounds.has_value() && second_bounds.has_value() &&
                 first_bounds->origin.x == 13.0F &&
                 first_bounds->origin.y == 12.0F &&
                 second_bounds->origin.x == 13.0F &&
                 second_bounds->origin.y == 28.0F
             ? 0
             : 601;
}

int test_element_builder_overflow_opacity_position_vocabulary_helpers_map_to_existing_style() {
  cgpui::AnyElement positioned_element =
      cgpui::into_element(cgpui::div()
                              .overflow_hidden()
                              .opacity(0.625F)
                              .z_index(7)
                              .absolute()
                              .top(cgpui::px(1.0F))
                              .right(cgpui::px(2.0F))
                              .bottom(cgpui::px(3.0F))
                              .left(cgpui::px(4.0F)));
  const auto* positioned =
      dynamic_cast<const cgpui::StyledElement*>(positioned_element.get());
  if (positioned == nullptr) {
    return 575;
  }

  const cgpui::Style& style = positioned->style();
  if (style.overflow != cgpui::Overflow::hidden ||
      style.opacity != 0.625F || style.z_index != 7 ||
      style.position != cgpui::Position::absolute ||
      style.inset.top != 1.0F || style.inset.right != 2.0F ||
      style.inset.bottom != 3.0F || style.inset.left != 4.0F) {
    return 576;
  }

  cgpui::AnyElement visible_element = cgpui::into_element(
      cgpui::div().overflow_hidden().overflow_visible().absolute().relative());
  const auto* visible =
      dynamic_cast<const cgpui::StyledElement*>(visible_element.get());
  return visible != nullptr &&
                 visible->style().overflow == cgpui::Overflow::visible &&
                 visible->style().position == cgpui::Position::relative
              ? 0
              : 577;
}

int test_element_builder_text_vocabulary_helpers_map_to_existing_style() {
  cgpui::AnyElement element =
      cgpui::into_element(cgpui::div()
                              .text_size(cgpui::px(19.0F))
                              .font_family("Step323Mono")
                              .text_color(cgpui::rgb(30, 40, 50)));
  const auto* styled = dynamic_cast<const cgpui::StyledElement*>(element.get());
  if (styled == nullptr) {
    return 578;
  }

  const cgpui::Style& style = styled->style();
  if (style.font_size != 19.0F || style.font.family != "Step323Mono" ||
      !style.foreground_color.has_value() ||
      style.foreground_color->g != 40.0F / 255.0F) {
    return 579;
  }

  cgpui::TextModel model{"text"};
  cgpui::AnyElement text_element =
      cgpui::into_element(cgpui::text(model)
                              .font_family("Step323Text")
                              .text_size(cgpui::px(21.0F)));
  const auto* text = dynamic_cast<const cgpui::TextElement*>(text_element.get());
  return text != nullptr && text->font().family == "Step323Text" &&
                 text->font_size() == 21.0F
            ? 0
            : 580;
}

int test_element_builder_shadow_vocabulary_maps_to_style_storage_and_paint() {
  const cgpui::BoxShadow custom_shadow{
      .color = cgpui::rgba(0, 0, 0, 0.3F),
      .offset = {.x = 3.0F, .y = 4.0F},
      .blur_radius = 12.0F,
      .spread_radius = 2.0F,
  };
  cgpui::AnyElement element =
      cgpui::into_element(cgpui::div().shadow(custom_shadow));
  const auto* styled = dynamic_cast<const cgpui::StyledElement*>(element.get());
  if (styled == nullptr || !styled->style().box_shadow.has_value() ||
      !same_shadow(*styled->style().box_shadow, custom_shadow)) {
    return 581;
  }

  cgpui::AnyElement preset_element = cgpui::into_element(cgpui::div().shadow_sm());
  const auto* preset =
      dynamic_cast<const cgpui::StyledElement*>(preset_element.get());
  const cgpui::BoxShadow expected_sm{
      .color = cgpui::rgba(0, 0, 0, 0.18F),
      .offset = {.x = 0.0F, .y = 2.0F},
      .blur_radius = 8.0F,
      .spread_radius = 0.0F,
  };
  if (preset == nullptr || !preset->style().box_shadow.has_value() ||
      !same_shadow(*preset->style().box_shadow, expected_sm)) {
    return 582;
  }

  cgpui::AnyElement painted_element =
      cgpui::into_element(cgpui::div()
                              .shadow(custom_shadow)
                              .bg(cgpui::rgb(240, 240, 240))
                              .rounded(cgpui::px(6.0F))
                              .child(cgpui::div()
                                         .size(cgpui::px(2.0F), cgpui::px(2.0F))
                                         .bg(cgpui::rgb(10, 20, 30))));
  auto* painted = dynamic_cast<cgpui::StyledElement*>(painted_element.get());
  if (painted == nullptr) {
    return 583;
  }
  const cgpui::LayoutOutput painted_layout =
      painted->layout(cgpui::LayoutInput{});
  if (painted_layout.size.width != 2.0F ||
      painted_layout.size.height != 2.0F) {
    return 586;
  }
  cgpui::PaintList paint_list;
  painted->paint(paint_list);
  const std::span<const cgpui::PaintCommand> commands = paint_list.commands();
  if (commands.size() != 3 ||
      commands[0].kind != cgpui::PaintCommandKind::box_shadow ||
      commands[1].kind != cgpui::PaintCommandKind::rounded_rect ||
      commands[2].kind != cgpui::PaintCommandKind::solid_rect) {
    return 584;
  }
  if (!same_shadow(commands[0].box_shadow.shadow, custom_shadow) ||
      commands[0].box_shadow.bounds.size.width != 2.0F ||
      commands[0].box_shadow.bounds.size.height != 2.0F ||
      commands[0].box_shadow.radius.top_left != 6.0F) {
    return 585;
  }

  return 0;
}

int test_element_builder_style_state_overlays_are_stored_on_styled_box() {
  cgpui::AnyElement element =
      cgpui::into_element(cgpui::div()
                              .background(cgpui::rgb(10, 10, 10))
                              .hover_style(cgpui::StyleOverlay{}
                                               .with_background_color(
                                                   cgpui::rgb(20, 20, 20))
                                               .with_padding(
                                                   cgpui::edges(2.0F)))
                              .focus_style(cgpui::StyleOverlay{}
                                               .with_foreground_color(
                                                   cgpui::rgb(30, 30, 30)))
                              .active_style(cgpui::StyleOverlay{}
                                                .with_foreground_color(
                                                    cgpui::rgb(35, 35, 35))
                                                .with_gap(7.0F))
                              .disabled_style(cgpui::StyleOverlay{}
                                                  .with_background_color(
                                                      cgpui::rgb(40, 40, 40))
                                                  .with_border_width(
                                                      cgpui::edges(3.0F))));
  const auto* styled = dynamic_cast<const cgpui::StyledElement*>(element.get());
  if (styled == nullptr) {
    return 224;
  }

  if (!styled->style().background_color.has_value() ||
      styled->style().background_color->r != 10.0F / 255.0F) {
    return 225;
  }

  const cgpui::Style resolved = cgpui::resolved_style(
      styled->style_state(),
      cgpui::StyleStateFlags{
          .hovered = true,
          .focused = true,
          .active = true,
          .disabled = true,
      });
  if (!resolved.background_color.has_value() ||
      resolved.background_color->r != 40.0F / 255.0F ||
      !resolved.foreground_color.has_value() ||
      resolved.foreground_color->r != 35.0F / 255.0F ||
      resolved.padding.top != 2.0F ||
      resolved.gap != 7.0F ||
      resolved.border_width.left != 3.0F) {
    return 226;
  }

  return 0;
}

int test_element_builder_key_applies_to_built_element() {
  cgpui::ElementKey key{.value = "profile-row"};
  cgpui::AnyElement element =
      cgpui::into_element(cgpui::div().key(key).size(12.0F, 4.0F));

  if (!element->key().has_value() || element->key()->value != "profile-row") {
    return 229;
  }

  cgpui::AnyElement wrapped =
      cgpui::into_element(cgpui::div()
                              .key("clickable-row")
                              .on_click([](const cgpui::ElementEventContext&) {
                                return cgpui::EventResult::consumed_event();
                              }));
  return wrapped->key().has_value() && wrapped->key()->value == "clickable-row"
             ? 0
             : 230;
}

int test_element_builder_stores_style_classes_and_inline_style() {
  const cgpui::StyleClassId card = cgpui::style_class("card");
  const cgpui::StyleClassId raised = cgpui::style_class("raised");
  cgpui::AnyElement element =
      cgpui::into_element(cgpui::div()
                              .class_name(card)
                              .class_name(raised)
                              .class_name(card)
                              .inline_style(cgpui::StyleOverlay{}
                                                .with_background_color(
                                                    cgpui::rgb(9, 8, 7))
                                                .with_padding(
                                                    cgpui::edges(5.0F))));

  const auto* styled = dynamic_cast<const cgpui::StyledElement*>(element.get());
  if (styled == nullptr) {
    return 231;
  }

  const cgpui::StyleClasses& classes = styled->style_classes();
  if (classes.size() != 2 || !classes.contains(card) ||
      !classes.contains(raised)) {
    return 232;
  }

  const cgpui::StyleOverlay& inline_style = styled->inline_style();
  if (!inline_style.background_color.has_value() ||
      inline_style.background_color->r != 9.0F / 255.0F ||
      !inline_style.padding.has_value() || inline_style.padding->left != 5.0F) {
    return 233;
  }

  cgpui::StyleCascade cascade;
  cascade.set_class_style(
      card,
      cgpui::StyleState{
          .base = cgpui::Style{}.with_background_color(cgpui::rgb(1, 2, 3)),
          .hover = cgpui::StyleOverlay{}.with_gap(4.0F),
          .active = cgpui::StyleOverlay{}.with_foreground_color(
              cgpui::rgb(2, 3, 4)),
      });
  const cgpui::Style resolved = styled->resolved_style(
      cascade,
      cgpui::StyleStateFlags{.hovered = true, .active = true});
  if (!resolved.background_color.has_value() ||
      resolved.background_color->r != 9.0F / 255.0F ||
      !resolved.foreground_color.has_value() ||
      resolved.foreground_color->r != 2.0F / 255.0F ||
      resolved.padding.left != 5.0F || resolved.gap != 4.0F) {
    return 234;
  }

  cgpui::StyleClasses raised_reuse;
  raised_reuse.add(card);
  cascade.set_class_rule(
      raised,
      cgpui::StyleClassRule{
          .reused_classes = raised_reuse,
          .style = cgpui::StyleState{
              .base = cgpui::Style{}.with_border_width(cgpui::edges(2.0F)),
              .active = cgpui::StyleOverlay{}.with_gap(9.0F),
          },
      });
  cgpui::AnyElement reused_element =
      cgpui::into_element(cgpui::div().class_name(raised));
  const auto* reused_styled =
      dynamic_cast<const cgpui::StyledElement*>(reused_element.get());
  if (reused_styled == nullptr) {
    return 235;
  }
  const cgpui::Style reused_resolved = reused_styled->resolved_style(
      cascade,
      cgpui::StyleStateFlags{.active = true});
  return reused_resolved.background_color.has_value() &&
                 reused_resolved.background_color->r == 1.0F / 255.0F &&
                 reused_resolved.foreground_color.has_value() &&
                 reused_resolved.foreground_color->r == 2.0F / 255.0F &&
                 reused_resolved.border_width.left == 2.0F &&
                 reused_resolved.gap == 9.0F
             ? 0
             : 236;
}

int test_styled_element_resolves_theme_token_fallbacks() {
  const cgpui::ThemeTokenId surface_color =
      cgpui::theme_token("color.surface");
  const cgpui::ThemeTokenId inline_color =
      cgpui::theme_token("color.inline");
  const cgpui::ThemeTokenId compact_spacing =
      cgpui::theme_token("space.compact");
  const cgpui::ThemeTokenId missing_spacing =
      cgpui::theme_token("space.missing");

  cgpui::Theme theme;
  theme.set_color(surface_color, cgpui::rgb(10, 20, 30))
      .set_color(inline_color, cgpui::rgb(40, 50, 60))
      .set_spacing(compact_spacing, cgpui::px(8.0F));

  const cgpui::StyleClassId card = cgpui::style_class("card.theme");
  cgpui::StyleCascade cascade;
  cascade.set_class_style(
      card,
      cgpui::StyleState{
          .base = cgpui::Style{}
                      .with_background_color_token(surface_color)
                      .with_gap_token(compact_spacing),
          .hover =
              cgpui::StyleOverlay{}.with_padding_token(compact_spacing),
      });

  cgpui::AnyElement element =
      cgpui::into_element(cgpui::div()
                              .class_name(card)
                              .style(cgpui::Style{}
                                         .with_margin_token(compact_spacing))
                              .inline_style(cgpui::StyleOverlay{}
                                                .with_background_color_token(
                                                    inline_color)
                                                .with_gap(3.0F)
                                                .with_gap_token(
                                                    missing_spacing)));
  const auto* styled = dynamic_cast<const cgpui::StyledElement*>(element.get());
  if (styled == nullptr) {
    return 237;
  }

  const cgpui::Style resolved =
      styled->resolved_style(cascade, cgpui::StyleStateFlags{.hovered = true},
                             theme);
  return resolved.background_color.has_value() &&
                 resolved.background_color->r == 40.0F / 255.0F &&
                 resolved.gap == 3.0F && resolved.padding.left == 8.0F &&
                 resolved.margin.left == 8.0F
             ? 0
             : 238;
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

int test_flex_row_justify_content_positions_children_on_main_axis() {
  auto make_row = [](cgpui::JustifyContent justify_content) {
    auto element =
        cgpui::ElementBuilder::row()
            .gap(5.0F)
            .justify_content(justify_content)
            .child(std::make_unique<cgpui::FixedSizeElement>(
                cgpui::Size{.width = 10.0F, .height = 10.0F}))
            .child(std::make_unique<cgpui::FixedSizeElement>(
                cgpui::Size{.width = 20.0F, .height = 10.0F}))
            .build();
    return std::unique_ptr<cgpui::FlexElement>(
        dynamic_cast<cgpui::FlexElement*>(element.release()));
  };

  std::unique_ptr<cgpui::FlexElement> centered =
      make_row(cgpui::JustifyContent::center);
  if (centered == nullptr ||
      centered->justify_content() != cgpui::JustifyContent::center) {
    return 238;
  }
  const cgpui::LayoutOutput centered_output = centered->layout(
      cgpui::LayoutInput{.constraints = {.min_size = {.width = 100.0F}}});
  if (centered_output.size.width != 100.0F ||
      centered_output.size.height != 10.0F) {
    return 239;
  }
  const std::optional<cgpui::Rect> centered_first =
      centered->children()[0]->layout_bounds();
  const std::optional<cgpui::Rect> centered_second =
      centered->children()[1]->layout_bounds();
  if (!centered_first.has_value() || !centered_second.has_value() ||
      centered_first->origin.x != 32.5F ||
      centered_second->origin.x != 47.5F) {
    return 240;
  }

  std::unique_ptr<cgpui::FlexElement> ended =
      make_row(cgpui::JustifyContent::end);
  const cgpui::LayoutOutput ended_output = ended->layout(
      cgpui::LayoutInput{.constraints = {.min_size = {.width = 100.0F}}});
  const std::optional<cgpui::Rect> ended_first =
      ended->children()[0]->layout_bounds();
  if (ended_output.size.width != 100.0F || !ended_first.has_value() ||
      ended_first->origin.x != 65.0F) {
    return 241;
  }

  std::unique_ptr<cgpui::FlexElement> spaced =
      make_row(cgpui::JustifyContent::space_between);
  const cgpui::LayoutOutput spaced_output = spaced->layout(
      cgpui::LayoutInput{.constraints = {.min_size = {.width = 100.0F}}});
  const std::optional<cgpui::Rect> spaced_first =
      spaced->children()[0]->layout_bounds();
  const std::optional<cgpui::Rect> spaced_second =
      spaced->children()[1]->layout_bounds();
  return spaced_output.size.width == 100.0F && spaced_first.has_value() &&
                 spaced_second.has_value() &&
                 spaced_first->origin.x == 0.0F &&
                 spaced_second->origin.x == 80.0F
             ? 0
             : 242;
}

int test_flex_row_align_items_positions_children_on_cross_axis() {
  auto element =
      cgpui::ElementBuilder::row()
          .align_items(cgpui::AlignItems::center)
          .child(std::make_unique<cgpui::FixedSizeElement>(
              cgpui::Size{.width = 10.0F, .height = 10.0F}))
          .build();
  auto* flex = dynamic_cast<cgpui::FlexElement*>(element.get());
  if (flex == nullptr || flex->align_items() != cgpui::AlignItems::center) {
    return 243;
  }
  const cgpui::LayoutOutput centered_output = flex->layout(
      cgpui::LayoutInput{.constraints = {.min_size = {.height = 40.0F}}});
  const std::optional<cgpui::Rect> centered_child =
      flex->children()[0]->layout_bounds();
  if (centered_output.size.height != 40.0F || !centered_child.has_value() ||
      centered_child->origin.y != 15.0F) {
    return 244;
  }

  flex->set_align_items(cgpui::AlignItems::end);
  const cgpui::LayoutOutput ended_output = flex->layout(
      cgpui::LayoutInput{.constraints = {.min_size = {.height = 40.0F}}});
  const std::optional<cgpui::Rect> ended_child =
      flex->children()[0]->layout_bounds();
  return ended_output.size.height == 40.0F && ended_child.has_value() &&
                 ended_child->origin.y == 30.0F
             ? 0
             : 245;
}

int test_element_builder_flex_grow_and_shrink_shortcuts_apply_to_children() {
  cgpui::AnyElement element =
      cgpui::into_element(cgpui::h_flex()
                              .child(cgpui::div()
                                         .size(10.0F, 4.0F)
                                         .flex_grow(2.0F)
                                         .flex_shrink(3.0F))
                              .child(cgpui::div()
                                         .size(20.0F, 4.0F)
                                         .flex_grow(1.0F)
                                         .flex_shrink(1.0F)));
  auto* flex = dynamic_cast<cgpui::FlexElement*>(element.get());
  if (flex == nullptr || flex->children().size() != 2) {
    return 251;
  }

  const cgpui::Element& first = *flex->children()[0];
  const cgpui::Element& second = *flex->children()[1];
  if (first.flex_grow() != 2.0F || first.flex_shrink() != 3.0F ||
      second.flex_grow() != 1.0F || second.flex_shrink() != 1.0F) {
    return 252;
  }
  return 0;
}

int test_flex_row_grow_expands_children_on_main_axis() {
  cgpui::AnyElement element =
      cgpui::into_element(cgpui::h_flex()
                              .gap(4.0F)
                              .child(cgpui::div()
                                         .size(10.0F, 5.0F)
                                         .flex_grow(1.0F))
                              .child(cgpui::div()
                                         .size(20.0F, 5.0F)
                                         .flex_grow(2.0F)));
  auto* flex = dynamic_cast<cgpui::FlexElement*>(element.get());
  if (flex == nullptr) {
    return 253;
  }

  const cgpui::LayoutOutput output = flex->layout(cgpui::LayoutInput{
      .constraints = {.min_size = {.width = 64.0F}},
  });
  if (output.size.width != 64.0F || output.size.height != 5.0F) {
    return 254;
  }

  const std::optional<cgpui::Rect> first_bounds =
      flex->children()[0]->layout_bounds();
  const std::optional<cgpui::Rect> second_bounds =
      flex->children()[1]->layout_bounds();
  return first_bounds.has_value() && second_bounds.has_value() &&
                 first_bounds->origin.x == 0.0F &&
                 first_bounds->size.width == 20.0F &&
                 second_bounds->origin.x == 24.0F &&
                 second_bounds->size.width == 40.0F
             ? 0
             : 255;
}

int test_flex_column_shrink_reduces_children_on_main_axis() {
  cgpui::AnyElement element =
      cgpui::into_element(cgpui::v_flex()
                              .gap(2.0F)
                              .child(cgpui::div()
                                         .size(4.0F, 30.0F)
                                         .flex_shrink(1.0F))
                              .child(cgpui::div()
                                         .size(4.0F, 20.0F)
                                         .flex_shrink(3.0F)));
  auto* flex = dynamic_cast<cgpui::FlexElement*>(element.get());
  if (flex == nullptr) {
    return 256;
  }

  const cgpui::LayoutOutput output = flex->layout(cgpui::LayoutInput{
      .constraints = {.max_size = {.width = 100.0F, .height = 42.0F}},
  });
  if (output.size.width != 4.0F || output.size.height != 42.0F) {
    return 257;
  }

  const std::optional<cgpui::Rect> first_bounds =
      flex->children()[0]->layout_bounds();
  const std::optional<cgpui::Rect> second_bounds =
      flex->children()[1]->layout_bounds();
  return first_bounds.has_value() && second_bounds.has_value() &&
                 first_bounds->origin.y == 0.0F &&
                 first_bounds->size.height == 27.5F &&
                 second_bounds->origin.y == 29.5F &&
                 second_bounds->size.height == 12.5F
             ? 0
             : 258;
}

int test_element_builder_absolute_and_inset_shortcuts_apply_to_child() {
  cgpui::AnyElement element =
      cgpui::into_element(cgpui::h_flex()
                              .child(cgpui::div()
                                         .size(10.0F, 4.0F)
                                         .absolute()
                                         .inset(cgpui::edges(2.0F, 3.0F))));
  auto* flex = dynamic_cast<cgpui::FlexElement*>(element.get());
  if (flex == nullptr || flex->children().size() != 1) {
    return 259;
  }

  const cgpui::Element& child = *flex->children()[0];
  return child.position() == cgpui::Position::absolute &&
                 child.inset().top == 3.0F && child.inset().left == 2.0F
             ? 0
             : 260;
}

int test_flex_row_absolute_children_do_not_affect_main_layout() {
  cgpui::AnyElement element =
      cgpui::into_element(cgpui::h_flex()
                              .gap(5.0F)
                              .child(cgpui::div().size(10.0F, 4.0F))
                              .child(cgpui::div()
                                         .size(20.0F, 6.0F)
                                         .absolute()
                                         .inset(cgpui::edges(
                                             2.0F,
                                             0.0F,
                                             0.0F,
                                             7.0F)))
                              .child(cgpui::div()
                                         .size(50.0F, 9.0F)
                                         .fixed()
                                         .top(4.0F)
                                         .left(17.0F))
                              .child(cgpui::div().size(30.0F, 5.0F)));
  auto* flex = dynamic_cast<cgpui::FlexElement*>(element.get());
  if (flex == nullptr || flex->children().size() != 4) {
    return 261;
  }

  const cgpui::LayoutOutput output = flex->layout(cgpui::LayoutInput{
      .constraints = {.min_size = {.width = 80.0F, .height = 40.0F}},
  });
  if (output.size.width != 80.0F || output.size.height != 40.0F) {
    return 262;
  }

  const std::optional<cgpui::Rect> first_bounds =
      flex->children()[0]->layout_bounds();
  const std::optional<cgpui::Rect> absolute_bounds =
      flex->children()[1]->layout_bounds();
  const std::optional<cgpui::Rect> fixed_bounds =
      flex->children()[2]->layout_bounds();
  const std::optional<cgpui::Rect> third_bounds =
      flex->children()[3]->layout_bounds();
  return first_bounds.has_value() && absolute_bounds.has_value() &&
                 fixed_bounds.has_value() && third_bounds.has_value() &&
                 first_bounds->origin.x == 0.0F &&
                 third_bounds->origin.x == 15.0F &&
                 absolute_bounds->origin.x == 7.0F &&
                 absolute_bounds->origin.y == 2.0F &&
                 absolute_bounds->size.width == 20.0F &&
                 absolute_bounds->size.height == 6.0F &&
                 fixed_bounds->origin.x == 17.0F &&
                 fixed_bounds->origin.y == 4.0F &&
                 fixed_bounds->size.width == 50.0F &&
                 fixed_bounds->size.height == 9.0F
             ? 0
             : 263;
}

int test_element_builder_fixed_shortcut_maps_to_position() {
  cgpui::AnyElement element = cgpui::into_element(
      cgpui::div().size(8.0F, 4.0F).fixed().top(6.0F).left(9.0F));
  const auto* styled = dynamic_cast<const cgpui::StyledElement*>(element.get());
  if (styled == nullptr) {
    return 602;
  }

  const cgpui::Style& style = styled->style();
  return style.position == cgpui::Position::fixed &&
                 element->position() == cgpui::Position::fixed &&
                 style.inset.top == 6.0F && style.inset.left == 9.0F &&
                 element->inset().top == 6.0F &&
                 element->inset().left == 9.0F
             ? 0
             : 603;
}

int test_styled_element_positioned_children_do_not_affect_vertical_flow() {
  cgpui::AnyElement element =
      cgpui::into_element(cgpui::div()
                              .gap(5.0F)
                              .p(1.0F)
                              .m(2.0F)
                              .child(cgpui::div().size(10.0F, 4.0F))
                              .child(cgpui::div()
                                         .size(20.0F, 6.0F)
                                         .absolute()
                                         .top(7.0F)
                                         .left(9.0F))
                              .child(cgpui::div()
                                         .size(30.0F, 8.0F)
                                         .fixed()
                                         .top(11.0F)
                                         .left(13.0F))
                              .child(cgpui::div().size(12.0F, 3.0F)));
  auto* styled = dynamic_cast<cgpui::StyledElement*>(element.get());
  if (styled == nullptr || styled->children().size() != 4) {
    return 604;
  }

  const cgpui::LayoutOutput output = styled->layout(cgpui::LayoutInput{});
  if (output.size.width != 18.0F || output.size.height != 18.0F) {
    return 605;
  }

  const std::optional<cgpui::Rect> first_bounds =
      styled->children()[0]->layout_bounds();
  const std::optional<cgpui::Rect> absolute_bounds =
      styled->children()[1]->layout_bounds();
  const std::optional<cgpui::Rect> fixed_bounds =
      styled->children()[2]->layout_bounds();
  const std::optional<cgpui::Rect> last_bounds =
      styled->children()[3]->layout_bounds();
  return first_bounds.has_value() && absolute_bounds.has_value() &&
                 fixed_bounds.has_value() && last_bounds.has_value() &&
                 first_bounds->origin.x == 3.0F &&
                 first_bounds->origin.y == 3.0F &&
                 last_bounds->origin.x == 3.0F &&
                 last_bounds->origin.y == 12.0F &&
                 absolute_bounds->origin.x == 9.0F &&
                 absolute_bounds->origin.y == 7.0F &&
                 fixed_bounds->origin.x == 13.0F &&
                 fixed_bounds->origin.y == 11.0F &&
                 fixed_bounds->size.width == 30.0F &&
                 fixed_bounds->size.height == 8.0F
             ? 0
             : 606;
}

int test_element_builder_layer_shortcut_applies_to_child() {
  cgpui::AnyElement element =
      cgpui::into_element(cgpui::h_flex()
                              .child(cgpui::div()
                                         .size(10.0F, 4.0F)
                                         .layer(3)));
  auto* flex = dynamic_cast<cgpui::FlexElement*>(element.get());
  if (flex == nullptr || flex->children().size() != 1) {
    return 264;
  }

  return flex->children()[0]->layer() == 3 &&
                 flex->children()[0]->z_order() == 3
             ? 0
             : 265;
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

int test_flex_column_justify_content_and_align_items_position_children() {
  auto element =
      cgpui::ElementBuilder::column()
          .gap(5.0F)
          .justify_content(cgpui::JustifyContent::center)
          .align_items(cgpui::AlignItems::end)
          .child(std::make_unique<cgpui::FixedSizeElement>(
              cgpui::Size{.width = 10.0F, .height = 10.0F}))
          .child(std::make_unique<cgpui::FixedSizeElement>(
              cgpui::Size{.width = 20.0F, .height = 20.0F}))
          .build();
  auto* flex = dynamic_cast<cgpui::FlexElement*>(element.get());
  if (flex == nullptr ||
      flex->justify_content() != cgpui::JustifyContent::center ||
      flex->align_items() != cgpui::AlignItems::end) {
    return 246;
  }

  const cgpui::LayoutOutput output = flex->layout(cgpui::LayoutInput{
      .constraints = {.min_size = {.width = 50.0F, .height = 80.0F}},
  });
  if (output.size.width != 50.0F || output.size.height != 80.0F) {
    return 247;
  }
  const std::optional<cgpui::Rect> first_bounds =
      flex->children()[0]->layout_bounds();
  const std::optional<cgpui::Rect> second_bounds =
      flex->children()[1]->layout_bounds();
  if (!first_bounds.has_value() || !second_bounds.has_value()) {
    return 248;
  }
  if (first_bounds->origin.x != 40.0F || first_bounds->origin.y != 22.5F ||
      second_bounds->origin.x != 30.0F ||
      second_bounds->origin.y != 37.5F) {
    return 249;
  }

  flex->set_justify_content(cgpui::JustifyContent::end);
  (void)flex->layout(cgpui::LayoutInput{
      .constraints = {.min_size = {.width = 50.0F, .height = 80.0F}},
  });
  const std::optional<cgpui::Rect> end_first_bounds =
      flex->children()[0]->layout_bounds();
  return end_first_bounds.has_value() && end_first_bounds->origin.y == 45.0F
             ? 0
             : 250;
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

int test_keyed_children_preserve_ids_across_reorder_insert_and_removal() {
  cgpui::ElementTree tree;
  const cgpui::ElementId root_id =
      tree.reconcile_root(std::make_unique<NamedElement>(1));

  std::vector<cgpui::AnyElement> first_pass;
  first_pass.push_back(
      cgpui::into_element(cgpui::div().key("alpha").size(10.0F, 1.0F)));
  first_pass.push_back(
      cgpui::into_element(cgpui::div().key("beta").size(20.0F, 2.0F)));
  first_pass.push_back(
      cgpui::into_element(cgpui::div().key("gamma").size(30.0F, 3.0F)));
  const std::vector<cgpui::ElementId> first_ids =
      tree.reconcile_children(root_id, std::move(first_pass));
  if (first_ids.size() != 3 || first_ids[0].value == 0 ||
      first_ids[1].value == 0 || first_ids[2].value == 0) {
    return 231;
  }

  const cgpui::ElementId alpha_id = first_ids[0];
  const cgpui::ElementId beta_id = first_ids[1];
  const cgpui::ElementId gamma_id = first_ids[2];
  const cgpui::ElementId gamma_child_id =
      tree.append_child(gamma_id, std::make_unique<NamedElement>(99));
  if (gamma_child_id.value == 0) {
    return 232;
  }

  std::vector<cgpui::AnyElement> second_pass;
  second_pass.push_back(
      cgpui::into_element(cgpui::div().key("gamma").size(33.0F, 3.0F)));
  second_pass.push_back(
      cgpui::into_element(cgpui::div().key("delta").size(40.0F, 4.0F)));
  second_pass.push_back(
      cgpui::into_element(cgpui::div().key("alpha").size(11.0F, 1.0F)));
  const std::vector<cgpui::ElementId> second_ids =
      tree.reconcile_children(root_id, std::move(second_pass));
  if (second_ids.size() != 3) {
    return 233;
  }
  if (second_ids[0] != gamma_id || second_ids[2] != alpha_id) {
    return 234;
  }
  if (second_ids[1].value == 0 || second_ids[1] == alpha_id ||
      second_ids[1] == beta_id || second_ids[1] == gamma_id) {
    return 235;
  }

  const std::span<const cgpui::ElementId> children = tree.children(root_id);
  if (children.size() != 3 || children[0] != gamma_id ||
      children[1] != second_ids[1] || children[2] != alpha_id) {
    return 236;
  }

  const auto* gamma = dynamic_cast<const cgpui::StyledElement*>(
      tree.get(gamma_id));
  const auto* delta = dynamic_cast<const cgpui::StyledElement*>(
      tree.get(second_ids[1]));
  const auto* alpha = dynamic_cast<const cgpui::StyledElement*>(
      tree.get(alpha_id));
  if (gamma == nullptr || delta == nullptr || alpha == nullptr) {
    return 237;
  }
  if (gamma->style().preferred_size.width != 33.0F ||
      delta->style().preferred_size.width != 40.0F ||
      alpha->style().preferred_size.width != 11.0F) {
    return 238;
  }
  if (!tree.parent(gamma_id).has_value() || *tree.parent(gamma_id) != root_id ||
      !tree.parent(alpha_id).has_value() || *tree.parent(alpha_id) != root_id ||
      !tree.parent(second_ids[1]).has_value() ||
      *tree.parent(second_ids[1]) != root_id) {
    return 239;
  }
  if (tree.get(beta_id) != nullptr || tree.parent(beta_id).has_value()) {
    return 240;
  }
  return tree.get(gamma_child_id) != nullptr &&
                 tree.parent(gamma_child_id).has_value() &&
                 *tree.parent(gamma_child_id) == gamma_id
             ? 0
             : 241;
}

int test_element_lifecycle_root_mount_and_update_callbacks() {
  cgpui::ElementTree tree;
  LifecycleCounters first;
  const cgpui::ElementId root_id =
      tree.reconcile_root(std::make_unique<LifecycleCountingElement>(first));
  if (root_id.value == 0 || first.mounts != 1 || first.updates != 0 ||
      first.unmounts != 0 || first.last_mounted != root_id) {
    return 242;
  }

  LifecycleCounters second;
  const cgpui::ElementId reconciled_root_id =
      tree.reconcile_root(std::make_unique<LifecycleCountingElement>(second));
  if (reconciled_root_id != root_id) {
    return 243;
  }
  if (first.unmounts != 0 || second.mounts != 0 || second.updates != 1 ||
      second.unmounts != 0 || second.last_updated != root_id) {
    return 244;
  }
  return 0;
}

int test_element_lifecycle_keyed_children_mount_update_and_unmount() {
  cgpui::ElementTree tree;
  const cgpui::ElementId root_id =
      tree.reconcile_root(std::make_unique<NamedElement>(1));

  LifecycleCounters alpha_first;
  LifecycleCounters beta_first;
  std::vector<cgpui::AnyElement> first_pass;
  auto alpha = std::make_unique<LifecycleCountingElement>(alpha_first);
  alpha->set_key(cgpui::ElementKey{.value = "alpha"});
  first_pass.push_back(std::move(alpha));
  auto beta = std::make_unique<LifecycleCountingElement>(beta_first);
  beta->set_key(cgpui::ElementKey{.value = "beta"});
  first_pass.push_back(std::move(beta));

  const std::vector<cgpui::ElementId> first_ids =
      tree.reconcile_children(root_id, std::move(first_pass));
  if (first_ids.size() != 2 || alpha_first.mounts != 1 ||
      beta_first.mounts != 1 || alpha_first.updates != 0 ||
      beta_first.updates != 0 || alpha_first.last_mounted != first_ids[0] ||
      beta_first.last_mounted != first_ids[1]) {
    return 245;
  }

  const cgpui::ElementId alpha_id = first_ids[0];
  const cgpui::ElementId beta_id = first_ids[1];
  LifecycleCounters alpha_second;
  LifecycleCounters gamma_second;
  std::vector<cgpui::AnyElement> second_pass;
  auto retained_alpha =
      std::make_unique<LifecycleCountingElement>(alpha_second);
  retained_alpha->set_key(cgpui::ElementKey{.value = "alpha"});
  second_pass.push_back(std::move(retained_alpha));
  auto gamma = std::make_unique<LifecycleCountingElement>(gamma_second);
  gamma->set_key(cgpui::ElementKey{.value = "gamma"});
  second_pass.push_back(std::move(gamma));

  const std::vector<cgpui::ElementId> second_ids =
      tree.reconcile_children(root_id, std::move(second_pass));
  if (second_ids.size() != 2 || second_ids[0] != alpha_id ||
      second_ids[1].value == 0 || second_ids[1] == beta_id) {
    return 246;
  }
  if (alpha_first.unmounts != 0 || beta_first.unmounts != 1 ||
      beta_first.last_unmounted != beta_id) {
    return 247;
  }
  if (alpha_second.mounts != 0 || alpha_second.updates != 1 ||
      alpha_second.unmounts != 0 || alpha_second.last_updated != alpha_id) {
    return 248;
  }
  if (gamma_second.mounts != 1 || gamma_second.updates != 0 ||
      gamma_second.unmounts != 0 ||
      gamma_second.last_mounted != second_ids[1]) {
    return 249;
  }
  return 0;
}

int test_element_lifecycle_set_root_unmounts_previous_tree() {
  cgpui::ElementTree tree;
  LifecycleCounters root_first;
  LifecycleCounters child_first;
  const cgpui::ElementId root_id =
      tree.set_root(std::make_unique<LifecycleCountingElement>(root_first));
  const cgpui::ElementId child_id =
      tree.append_child(root_id, std::make_unique<LifecycleCountingElement>(
                                     child_first));
  if (root_first.mounts != 1 || child_first.mounts != 1 ||
      root_first.last_mounted != root_id ||
      child_first.last_mounted != child_id) {
    return 250;
  }

  LifecycleCounters root_second;
  const cgpui::ElementId second_root_id =
      tree.set_root(std::make_unique<LifecycleCountingElement>(root_second));
  if (second_root_id.value == 0 || second_root_id == root_id) {
    return 251;
  }
  if (child_first.unmounts != 1 || child_first.last_unmounted != child_id ||
      root_first.unmounts != 1 || root_first.last_unmounted != root_id) {
    return 252;
  }
  if (root_second.mounts != 1 || root_second.updates != 0 ||
      root_second.last_mounted != second_root_id) {
    return 253;
  }
  return 0;
}

int test_element_tree_state_survives_keyed_reconcile_and_prunes_removed_nodes() {
  cgpui::ElementTree tree;
  const cgpui::ElementId root_id =
      tree.reconcile_root(std::make_unique<NamedElement>(1));

  std::vector<cgpui::AnyElement> first_pass;
  first_pass.push_back(cgpui::into_element(cgpui::div().key("alpha")));
  first_pass.push_back(cgpui::into_element(cgpui::div().key("beta")));
  const std::vector<cgpui::ElementId> first_ids =
      tree.reconcile_children(root_id, std::move(first_pass));
  if (first_ids.size() != 2) {
    return 254;
  }

  const cgpui::ElementId alpha_id = first_ids[0];
  const cgpui::ElementId beta_id = first_ids[1];
  WidgetState* alpha_state =
      tree.state_or_init<WidgetState>(alpha_id, 7, std::string{"alpha"});
  WidgetState* beta_state =
      tree.emplace_state<WidgetState>(beta_id, 3, std::string{"beta"});
  if (alpha_state == nullptr || beta_state == nullptr ||
      alpha_state->value != 7 || beta_state->name != "beta") {
    return 255;
  }

  alpha_state->value = 8;
  if (tree.state<AlternateWidgetState>(alpha_id) != nullptr ||
      tree.state<WidgetState>(cgpui::ElementId{999}) != nullptr ||
      tree.state_or_init<WidgetState>(
          cgpui::ElementId{999},
          1,
          std::string{"missing"}) != nullptr ||
      tree.emplace_state<WidgetState>(
          cgpui::ElementId{999},
          1,
          std::string{"missing"}) != nullptr) {
    return 256;
  }

  std::vector<cgpui::AnyElement> second_pass;
  second_pass.push_back(cgpui::into_element(cgpui::div().key("gamma")));
  second_pass.push_back(cgpui::into_element(cgpui::div().key("alpha")));
  const std::vector<cgpui::ElementId> second_ids =
      tree.reconcile_children(root_id, std::move(second_pass));
  if (second_ids.size() != 2 || second_ids[1] != alpha_id ||
      second_ids[0] == beta_id) {
    return 257;
  }

  const WidgetState* retained_alpha_state =
      static_cast<const cgpui::ElementTree&>(tree).state<WidgetState>(alpha_id);
  if (retained_alpha_state == nullptr || retained_alpha_state->value != 8 ||
      retained_alpha_state->name != "alpha") {
    return 258;
  }

  WidgetState* existing_alpha_state =
      tree.state_or_init<WidgetState>(alpha_id, 99, std::string{"ignored"});
  if (existing_alpha_state != retained_alpha_state ||
      existing_alpha_state->value != 8 ||
      existing_alpha_state->name != "alpha") {
    return 259;
  }

  if (tree.state<WidgetState>(beta_id) != nullptr ||
      tree.state<AlternateWidgetState>(alpha_id) != nullptr) {
    return 260;
  }

  WidgetState* replaced_alpha_state =
      tree.emplace_state<WidgetState>(alpha_id, 42, std::string{"replaced"});
  if (replaced_alpha_state == nullptr || replaced_alpha_state->value != 42 ||
      replaced_alpha_state->name != "replaced") {
    return 261;
  }

  return 0;
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

int test_div_builder_preserves_multiple_children_in_author_order() {
  cgpui::AnyElement element =
      cgpui::into_element(cgpui::div()
                              .padding(cgpui::edges(1.0F))
                              .gap(cgpui::px(2.0F))
                              .child(cgpui::div()
                                         .size(10.0F, 4.0F)
                                         .background(cgpui::rgb(10, 20, 30)))
                              .child(cgpui::div()
                                         .size(6.0F, 5.0F)
                                         .background(cgpui::rgb(40, 50, 60))));
  auto* styled = dynamic_cast<cgpui::StyledElement*>(element.get());
  if (styled == nullptr || styled->children().size() != 2 ||
      styled->child() != styled->children()[0].get()) {
    return 901;
  }

  const cgpui::LayoutOutput output = styled->layout(cgpui::LayoutInput{});
  if (output.size.width != 12.0F || output.size.height != 13.0F) {
    return 902;
  }

  const std::optional<cgpui::Rect> first_bounds =
      styled->children()[0]->layout_bounds();
  const std::optional<cgpui::Rect> second_bounds =
      styled->children()[1]->layout_bounds();
  if (!first_bounds.has_value() || !second_bounds.has_value() ||
      first_bounds->origin.x != 1.0F || first_bounds->origin.y != 1.0F ||
      second_bounds->origin.x != 1.0F || second_bounds->origin.y != 7.0F) {
    return 903;
  }

  cgpui::PaintList paint_list;
  styled->paint(paint_list);
  const std::span<const cgpui::PaintCommand> commands = paint_list.commands();
  if (commands.size() != 2 ||
      commands[0].kind != cgpui::PaintCommandKind::solid_rect ||
      commands[1].kind != cgpui::PaintCommandKind::solid_rect) {
    return 904;
  }

  return commands[0].solid_rect.rect.origin.y == 1.0F &&
                 commands[1].solid_rect.rect.origin.y == 7.0F
             ? 0
             : 905;
}

int test_element_builder_children_accepts_owned_collection() {
  std::vector<cgpui::AnyElement> children;
  children.push_back(cgpui::into_element(
      cgpui::div().size(3.0F, 4.0F).background(cgpui::rgb(1, 2, 3))));
  children.push_back(cgpui::into_element(
      cgpui::div().size(5.0F, 6.0F).background(cgpui::rgb(4, 5, 6))));

  cgpui::AnyElement element = cgpui::into_element(
      cgpui::div().gap(1.0F).children(std::move(children)));
  auto* styled = dynamic_cast<cgpui::StyledElement*>(element.get());
  if (styled == nullptr || styled->children().size() != 2) {
    return 906;
  }

  const cgpui::LayoutOutput output = styled->layout(cgpui::LayoutInput{});
  if (output.size.width != 5.0F || output.size.height != 11.0F) {
    return 907;
  }

  const std::optional<cgpui::Rect> second_bounds =
      styled->children()[1]->layout_bounds();
  return second_bounds.has_value() && second_bounds->origin.y == 5.0F
             ? 0
             : 908;
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
  if (commands[0].kind != cgpui::PaintCommandKind::solid_rect) {
    return 269;
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

int test_styled_element_paints_rounded_background_rect_metadata() {
  const cgpui::Color color{
      .r = 0.9F,
      .g = 0.4F,
      .b = 0.2F,
      .a = 1.0F,
  };
  const cgpui::BorderRadii radius =
      cgpui::BorderRadii::corners(4.0F, 5.0F, 6.0F, 7.0F);
  std::unique_ptr<cgpui::Element> element =
      cgpui::ElementBuilder::box()
          .style(cgpui::Style{}
                     .with_background_color(color)
                     .with_border_radius(radius)
                     .with_preferred_size(
                         cgpui::Size{.width = 44.0F, .height = 22.0F}))
          .build();

  const cgpui::LayoutOutput output = element->layout(cgpui::LayoutInput{});
  if (output.size.width != 44.0F || output.size.height != 22.0F) {
    return 270;
  }

  cgpui::PaintList paint_list;
  element->paint(paint_list);

  const std::span<const cgpui::PaintCommand> commands = paint_list.commands();
  if (commands.size() != 1) {
    return 271;
  }
  if (commands[0].kind != cgpui::PaintCommandKind::rounded_rect) {
    return 272;
  }

  const cgpui::RoundedRect& rounded = commands[0].rounded_rect;
  if (rounded.rect.origin.x != 0.0F || rounded.rect.origin.y != 0.0F ||
      rounded.rect.size.width != 44.0F ||
      rounded.rect.size.height != 22.0F) {
    return 273;
  }
  if (rounded.color.r != color.r || rounded.color.g != color.g ||
      rounded.color.b != color.b || rounded.color.a != color.a) {
    return 274;
  }
  return rounded.radius.top_left == 4.0F &&
                 rounded.radius.top_right == 5.0F &&
                 rounded.radius.bottom_right == 6.0F &&
                 rounded.radius.bottom_left == 7.0F
             ? 0
             : 275;
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
  paint_list.push_clip(clip);
  paint_list.fill_rounded_rect(
      cgpui::Rect{.size = {.width = 25.0F, .height = 12.0F}},
      cgpui::Color{.r = 0.4F, .a = 1.0F},
      cgpui::BorderRadii::all(3.0F));
  paint_list.pop_clip();
  paint_list.push_clip(clip);
  paint_list.fill_text(
      cgpui::Rect{.size = {.width = 35.0F, .height = 16.0F}},
      cgpui::Color{.g = 0.6F, .a = 1.0F},
      "clip text");
  paint_list.pop_clip();
  paint_list.fill_rect(
      cgpui::Rect{.size = {.width = 30.0F, .height = 10.0F}},
      cgpui::Color{.b = 0.3F, .a = 1.0F});

  const std::span<const cgpui::PaintCommand> commands = paint_list.commands();
  if (commands.size() != 5) {
    return 127;
  }
  if (commands[0].clip_rect.has_value() || commands[4].clip_rect.has_value()) {
    return 128;
  }
  return commands[1].kind == cgpui::PaintCommandKind::solid_rect &&
                 commands[2].kind == cgpui::PaintCommandKind::rounded_rect &&
                 commands[3].kind == cgpui::PaintCommandKind::text &&
                 commands[1].clip_rect.has_value() &&
                 commands[2].clip_rect.has_value() &&
                 commands[3].clip_rect.has_value() &&
                 commands[1].clip_rect->origin.x == 2.0F &&
                 commands[2].clip_rect->origin.y == 3.0F &&
                 commands[2].rounded_rect.radius.top_left == 3.0F &&
                 commands[1].clip_rect->size.width == 40.0F &&
                 commands[2].clip_rect->size.height == 20.0F &&
                 commands[3].text.content == "clip text" &&
                 commands[3].text.byte_length == 9
             ? 0
             : 129;
}

int test_paint_list_intersects_nested_clips() {
  cgpui::PaintList paint_list;
  const cgpui::Rect outer{
      .origin = {.x = 10.0F, .y = 10.0F},
      .size = {.width = 40.0F, .height = 30.0F},
  };
  const cgpui::Rect inner{
      .origin = {.x = 30.0F, .y = 0.0F},
      .size = {.width = 50.0F, .height = 50.0F},
  };
  const cgpui::Rect disjoint{
      .origin = {.x = 100.0F, .y = 120.0F},
      .size = {.width = 5.0F, .height = 6.0F},
  };
  const cgpui::Rect disjoint_before{
      .origin = {.x = -20.0F, .y = -30.0F},
      .size = {.width = 5.0F, .height = 6.0F},
  };
  const cgpui::Rect expected_intersection{
      .origin = {.x = 30.0F, .y = 10.0F},
      .size = {.width = 20.0F, .height = 30.0F},
  };
  const cgpui::Rect expected_empty_intersection{
      .origin = {.x = 100.0F, .y = 120.0F},
      .size = {.width = 0.0F, .height = 0.0F},
  };
  const cgpui::Rect expected_empty_before_intersection{
      .origin = {.x = -20.0F, .y = -30.0F},
      .size = {.width = 0.0F, .height = 0.0F},
  };

  paint_list.push_clip(outer);
  paint_list.fill_rect(
      cgpui::Rect{.size = {.width = 10.0F, .height = 10.0F}},
      cgpui::Color{.r = 0.1F, .a = 1.0F});
  paint_list.push_clip(inner);
  paint_list.fill_rect(
      cgpui::Rect{.size = {.width = 20.0F, .height = 10.0F}},
      cgpui::Color{.g = 0.2F, .a = 1.0F});
  paint_list.push_clip(disjoint);
  paint_list.fill_rect(
      cgpui::Rect{.size = {.width = 30.0F, .height = 10.0F}},
      cgpui::Color{.b = 0.3F, .a = 1.0F});

  const std::span<const cgpui::PaintCommand> commands = paint_list.commands();
  if (commands.size() != 3) {
    return 600;
  }
  if (!commands[0].clip_rect.has_value() ||
      !same_rect(*commands[0].clip_rect, outer) ||
      commands[0].clip_stack.full_depth != 1 ||
      commands[0].clip_stack.clips.size() != 1 ||
      !commands[0].clip_stack.current_clip_rect.has_value() ||
      !same_rect(*commands[0].clip_stack.current_clip_rect, outer)) {
    return 601;
  }
  if (!commands[1].clip_rect.has_value() ||
      !same_rect(*commands[1].clip_rect, expected_intersection) ||
      commands[1].clip_stack.full_depth != 2 ||
      commands[1].clip_stack.clips.size() != 2 ||
      !same_rect(commands[1].clip_stack.clips.back(), expected_intersection) ||
      !commands[1].clip_stack.current_clip_rect.has_value() ||
      !same_rect(
          *commands[1].clip_stack.current_clip_rect,
          expected_intersection)) {
    return 602;
  }
  if (!commands[2].clip_rect.has_value() ||
      !same_rect(*commands[2].clip_rect, expected_empty_intersection)) {
    return 603;
  }

  cgpui::PaintList before_list;
  before_list.push_clip(outer);
  before_list.push_clip(disjoint_before);
  before_list.fill_rect(
      cgpui::Rect{.size = {.width = 10.0F, .height = 10.0F}},
      cgpui::Color{.a = 1.0F});
  const std::span<const cgpui::PaintCommand> before_commands =
      before_list.commands();
  return before_commands.size() == 1 &&
                 before_commands[0].clip_rect.has_value() &&
                 same_rect(
                     *before_commands[0].clip_rect,
                     expected_empty_before_intersection)
             ? 0
             : 604;
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

int test_styled_element_paint_metadata_composes_opacity_transform_in_order() {
  const cgpui::Color parent_color{.r = 0.1F, .g = 0.0F, .b = 0.0F, .a = 1.0F};
  const cgpui::Color child_color{.r = 0.0F, .g = 0.2F, .b = 0.0F, .a = 1.0F};
  std::unique_ptr<cgpui::Element> element =
      cgpui::ElementBuilder::box()
          .style(cgpui::Style{}
                     .with_background_color(parent_color)
                     .with_opacity(0.8F)
                     .with_transform(
                         cgpui::AffineTransform::translation(2.0F, 3.0F)))
          .child(cgpui::ElementBuilder::box()
                     .style(cgpui::Style{}
                                .with_background_color(child_color)
                                .with_preferred_size(
                                    cgpui::Size{.width = 8.0F, .height = 6.0F})
                                .with_opacity(0.5F)
                                .with_transform(
                                    cgpui::AffineTransform::translation(
                                        4.0F,
                                        5.0F))))
          .build();

  (void)element->layout(cgpui::LayoutInput{});
  cgpui::PaintList paint_list;
  element->paint(paint_list);
  const std::span<const cgpui::PaintCommand> commands = paint_list.commands();
  if (commands.size() != 2) {
    return 393;
  }
  if (commands[0].kind != cgpui::PaintCommandKind::solid_rect ||
      commands[1].kind != cgpui::PaintCommandKind::solid_rect ||
      commands[0].solid_rect.color.r != parent_color.r ||
      commands[1].solid_rect.color.g != child_color.g) {
    return 394;
  }
  if (commands[0].metadata.opacity != 0.8F ||
      !same_transform(
          commands[0].metadata.transform,
          cgpui::AffineTransform::translation(2.0F, 3.0F))) {
    return 395;
  }
  return commands[1].metadata.opacity == 0.4F &&
                 same_transform(
                     commands[1].metadata.transform,
                     cgpui::AffineTransform::translation(6.0F, 8.0F))
             ? 0
             : 396;
}

int test_styled_element_hidden_overflow_clips_hit_testing_to_bounds() {
  auto child = std::make_unique<cgpui::FixedSizeElement>(
      cgpui::Size{.width = 50.0F, .height = 30.0F});
  child->assign_id(cgpui::ElementId{136});

  cgpui::StyledElement element(
      cgpui::Style{}.with_overflow(cgpui::Overflow::hidden),
      std::move(child));
  element.assign_id(cgpui::ElementId{135});
  (void)element.layout(cgpui::LayoutInput{});
  element.set_layout_bounds(cgpui::Rect{
      .origin = {.x = 0.0F, .y = 0.0F},
      .size = {.width = 20.0F, .height = 10.0F},
  });

  if (element.hit_test(cgpui::Point{.x = 5.0F, .y = 5.0F}) !=
      cgpui::ElementId{136}) {
    return 232;
  }
  return element.hit_test(cgpui::Point{.x = 25.0F, .y = 5.0F}).value == 0
             ? 0
             : 233;
}

int test_styled_element_visible_overflow_preserves_child_hit_testing() {
  auto child = std::make_unique<cgpui::FixedSizeElement>(
      cgpui::Size{.width = 50.0F, .height = 30.0F});
  child->assign_id(cgpui::ElementId{138});

  cgpui::StyledElement element(
      cgpui::Style{}.with_overflow(cgpui::Overflow::visible),
      std::move(child));
  element.assign_id(cgpui::ElementId{137});
  (void)element.layout(cgpui::LayoutInput{});
  element.set_layout_bounds(cgpui::Rect{
      .origin = {.x = 0.0F, .y = 0.0F},
      .size = {.width = 20.0F, .height = 10.0F},
  });

  if (element.hit_test(cgpui::Point{.x = 5.0F, .y = 5.0F}) !=
      cgpui::ElementId{138}) {
    return 234;
  }
  return element.hit_test(cgpui::Point{.x = 25.0F, .y = 5.0F}) ==
                 cgpui::ElementId{138}
             ? 0
             : 235;
}

int test_styled_element_hidden_overflow_clips_hit_testing_to_explicit_clip_rect() {
  auto child = std::make_unique<cgpui::FixedSizeElement>(
      cgpui::Size{.width = 50.0F, .height = 30.0F});
  child->assign_id(cgpui::ElementId{140});

  const cgpui::Rect clip{
      .origin = {.x = 4.0F, .y = 5.0F},
      .size = {.width = 12.0F, .height = 8.0F},
  };
  cgpui::StyledElement element(
      cgpui::Style{}
          .with_overflow(cgpui::Overflow::hidden)
          .with_clip_rect(clip),
      std::move(child));
  element.assign_id(cgpui::ElementId{139});
  (void)element.layout(cgpui::LayoutInput{});

  if (element.hit_test(cgpui::Point{.x = 6.0F, .y = 6.0F}) !=
      cgpui::ElementId{140}) {
    return 236;
  }
  if (element.hit_test(cgpui::Point{.x = 2.0F, .y = 6.0F}).value != 0 ||
      element.hit_test(cgpui::Point{.x = 16.0F, .y = 6.0F}).value != 0) {
    return 237;
  }
  return 0;
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

int test_text_element_uses_font_size_for_deterministic_metrics() {
  cgpui::TextModel model("abcd");
  cgpui::TextElement element(
      &model,
      cgpui::Style{}
          .with_font(cgpui::FontDescriptor{.family = "Inter"})
          .with_font_size(20.0F));

  if (element.font().family != "Inter" || element.font_size() != 20.0F) {
    return 280;
  }

  const cgpui::LayoutOutput output = element.layout(cgpui::LayoutInput{});
  if (output.size.width != 40.0F || output.size.height != 20.0F) {
    return 281;
  }

  const std::optional<cgpui::Rect> bounds = element.layout_bounds();
  return bounds.has_value() && bounds->size.width == 40.0F &&
                 bounds->size.height == 20.0F
             ? 0
             : 282;
}

int test_text_element_uses_shaping_run_for_utf8_layout_metrics() {
  cgpui::TextModel model("A\xE4\xB8\xAD");
  cgpui::TextElement element(
      &model,
      cgpui::Style{}
          .with_font(cgpui::FontDescriptor{.family = "Fallback"})
          .with_font_size(20.0F));

  const cgpui::TextShapeRun run = element.shape_run();
  if (run.glyph_count() != 2 || run.total_advance != 20.0F ||
      run.byte_length != model.text().size()) {
    return 283;
  }

  const cgpui::LayoutOutput output = element.layout(cgpui::LayoutInput{});
  return output.size.width == 20.0F && output.size.height == 20.0F ? 0 : 284;
}

int test_label_widget_paints_text_without_editing_metadata() {
  cgpui::AnyElement element =
      cgpui::label("status")
          .foreground(cgpui::rgb(20, 40, 60))
          .font(cgpui::FontDescriptor{.family = "Inter"})
          .font_size(18.0F)
          .key("status-label")
          .build();

  const auto* label = dynamic_cast<const cgpui::LabelElement*>(element.get());
  if (label == nullptr || label->text() != "status") {
    return 353;
  }
  if (label->focusable() || !label->key().has_value() ||
      label->key()->value != "status-label") {
    return 354;
  }
  if (label->font().family != "Inter" || label->font_size() != 18.0F ||
      !label->style().foreground_color.has_value()) {
    return 355;
  }

  const cgpui::LayoutOutput output = element->layout(cgpui::LayoutInput{});
  if (output.size.width != 54.0F || output.size.height != 18.0F) {
    return 356;
  }

  cgpui::PaintList paint_list;
  element->paint(paint_list);
  const std::span<const cgpui::PaintCommand> commands = paint_list.commands();
  if (commands.size() != 1 ||
      commands[0].kind != cgpui::PaintCommandKind::text) {
    return 357;
  }

  const cgpui::TextPaint& text = commands[0].text;
  return text.content == "status" && text.byte_length == 6 &&
                 text.font.family == "Inter" && text.font_size == 18.0F &&
                 text.color.r == 20.0F / 255.0F &&
                 text.bounds.size.width == 54.0F &&
                 text.bounds.size.height == 18.0F
             ? 0
             : 358;
}

int test_text_input_widget_is_focusable_editable_text_element() {
  cgpui::TextModel model("input");
  cgpui::AnyElement element =
      cgpui::text_input(model)
          .foreground(cgpui::rgb(30, 60, 90))
          .font(cgpui::FontDescriptor{.family = "Input"})
          .font_size(18.0F)
          .key("primary-input")
          .build();

  const auto* input =
      dynamic_cast<const cgpui::TextInputElement*>(element.get());
  if (input == nullptr || input->model() != &model ||
      input->text() != "input") {
    return 359;
  }
  if (!input->focusable() || !input->key().has_value() ||
      input->key()->value != "primary-input") {
    return 360;
  }
  if (input->font().family != "Input" || input->font_size() != 18.0F ||
      !input->style().foreground_color.has_value()) {
    return 361;
  }

  model.set_selection(1, 4);
  const cgpui::LayoutOutput output = element->layout(cgpui::LayoutInput{});
  if (output.size.width != 45.0F || output.size.height != 18.0F) {
    return 362;
  }

  cgpui::PaintList paint_list;
  element->paint(paint_list);
  const std::span<const cgpui::PaintCommand> commands = paint_list.commands();
  if (commands.size() != 3 ||
      commands[0].kind != cgpui::PaintCommandKind::text_selection ||
      commands[1].kind != cgpui::PaintCommandKind::text ||
      commands[2].kind != cgpui::PaintCommandKind::text_caret) {
    return 363;
  }

  const cgpui::TextSelectionPaint& selection = commands[0].text_selection;
  const cgpui::TextPaint& text = commands[1].text;
  const cgpui::TextCaretPaint& caret = commands[2].text_caret;
  return selection.range.start == 1 && selection.range.end == 4 &&
                 selection.rect.size.width == 27.0F &&
                 text.content == "input" && text.font.family == "Input" &&
                 text.font_size == 18.0F && caret.byte_offset == 4 &&
                 caret.rect.origin.x == 36.0F
             ? 0
             : 364;
}

int test_styled_element_inherits_text_style_into_label_descendant() {
  cgpui::AnyElement element =
      cgpui::into_element(cgpui::div()
                              .text_color(cgpui::rgb(12, 34, 56))
                              .font_family("InheritedParent")
                              .text_size(20.0F)
                              .child(cgpui::label("copy").build()));

  const cgpui::LayoutOutput output = element->layout(cgpui::LayoutInput{});
  if (output.size.width != 40.0F || output.size.height != 20.0F) {
    return 608;
  }

  cgpui::PaintList paint_list;
  element->paint(paint_list);
  const std::span<const cgpui::PaintCommand> commands = paint_list.commands();
  if (commands.size() != 1 ||
      commands[0].kind != cgpui::PaintCommandKind::text) {
    return 609;
  }

  const cgpui::TextPaint& text = commands[0].text;
  if (text.font.family != "InheritedParent" || text.font_size != 20.0F ||
      text.color.r != 12.0F / 255.0F ||
      text.bounds.size.width != 40.0F ||
      text.bounds.size.height != 20.0F) {
    return 610;
  }
  return 0;
}

int test_child_label_text_style_overrides_inherited_parent_style() {
  cgpui::AnyElement element =
      cgpui::into_element(cgpui::div()
                              .text_color(cgpui::rgb(12, 34, 56))
                              .font_family("InheritedParent")
                              .text_size(24.0F)
                              .child(cgpui::label("child")
                                         .foreground(cgpui::rgb(200, 100, 50))
                                         .font(cgpui::FontDescriptor{
                                             .family = "ExplicitChild"})
                                         .font_size(12.0F)
                                         .build()));

  const cgpui::LayoutOutput output = element->layout(cgpui::LayoutInput{});
  if (output.size.width != 30.0F || output.size.height != 12.0F) {
    return 611;
  }

  cgpui::PaintList paint_list;
  element->paint(paint_list);
  const std::span<const cgpui::PaintCommand> commands = paint_list.commands();
  if (commands.size() != 1 ||
      commands[0].kind != cgpui::PaintCommandKind::text) {
    return 612;
  }

  const cgpui::TextPaint& text = commands[0].text;
  if (text.font.family != "ExplicitChild" || text.font_size != 12.0F ||
      text.color.r != 200.0F / 255.0F ||
      text.bounds.size.width != 30.0F ||
      text.bounds.size.height != 12.0F) {
    return 613;
  }
  return 0;
}

int test_inherited_text_style_flows_through_flex_container() {
  cgpui::AnyElement element =
      cgpui::into_element(cgpui::div()
                              .text_color(cgpui::rgb(80, 90, 100))
                              .font_family("FlexInherited")
                              .text_size(22.0F)
                              .child(cgpui::h_flex().child(
                                  cgpui::label("xy").build())));

  const cgpui::LayoutOutput output = element->layout(cgpui::LayoutInput{});
  if (output.size.width != 22.0F || output.size.height != 22.0F) {
    return 614;
  }

  const auto* styled = dynamic_cast<const cgpui::StyledElement*>(element.get());
  const auto* flex = styled == nullptr
                         ? nullptr
                         : dynamic_cast<const cgpui::FlexElement*>(
                               styled->child());
  if (flex == nullptr || flex->children().empty()) {
    return 615;
  }

  const auto* label =
      dynamic_cast<const cgpui::LabelElement*>(flex->children()[0].get());
  if (label == nullptr) {
    return 616;
  }
  const std::optional<cgpui::Rect> bounds = label->layout_bounds();
  return label->font().family == "FlexInherited" &&
                 label->font_size() == 22.0F &&
                 label->effective_style().foreground_color.has_value() &&
                 label->effective_style().foreground_color->r ==
                     80.0F / 255.0F &&
                 bounds.has_value() && bounds->size.width == 22.0F &&
                 bounds->size.height == 22.0F
             ? 0
             : 617;
}

int test_child_view_element_references_view_and_lays_out_placeholder() {
  cgpui::ChildViewElement element(
      cgpui::ViewId{42},
      cgpui::Size{.width = 120.0F, .height = 80.0F});
  element.assign_id(cgpui::ElementId{45});

  if (element.view_id() != cgpui::ViewId{42} ||
      element.placeholder_size().width != 120.0F ||
      element.placeholder_size().height != 80.0F) {
    return 227;
  }

  const cgpui::LayoutOutput output = element.layout(cgpui::LayoutInput{
      .constraints =
          {
              .min_size = {.width = 40.0F, .height = 20.0F},
              .max_size = {.width = 100.0F, .height = 90.0F},
          },
  });
  if (output.size.width != 100.0F || output.size.height != 80.0F) {
    return 228;
  }

  const std::optional<cgpui::Rect> bounds = element.layout_bounds();
  if (!bounds.has_value() || bounds->origin.x != 0.0F ||
      bounds->origin.y != 0.0F || bounds->size.width != 100.0F ||
      bounds->size.height != 80.0F) {
    return 229;
  }

  if (element.hit_test(cgpui::Point{.x = 99.0F, .y = 79.0F}) !=
      cgpui::ElementId{45}) {
    return 230;
  }
  return element.hit_test(cgpui::Point{.x = 100.0F, .y = 79.0F}).value == 0 &&
                 element.hit_test(cgpui::Point{.x = 99.0F, .y = 80.0F})
                         .value == 0
             ? 0
             : 231;
}

int test_text_element_paints_text_command_from_layout_bounds() {
  cgpui::TextModel model("hi");
  cgpui::TextElement element(
      &model,
      cgpui::Style{}
          .with_foreground_color(cgpui::rgb(10, 20, 30))
          .with_font(cgpui::FontDescriptor{.family = "Mono"})
          .with_font_size(20.0F));
  (void)element.layout(cgpui::LayoutInput{});

  cgpui::PaintList paint_list;
  element.paint(paint_list);
  const std::span<const cgpui::PaintCommand> commands = paint_list.commands();
  if (commands.size() != 2) {
    return 140;
  }
  if (commands[0].kind != cgpui::PaintCommandKind::text) {
    return 141;
  }

  const cgpui::TextPaint& text = commands[0].text;
  if (text.bounds.size.width != 20.0F || text.bounds.size.height != 20.0F) {
    return 142;
  }
  if (text.content != "hi" || text.byte_length != model.text().size()) {
    return 143;
  }
  if (text.font.family != "Mono" || text.font_size != 20.0F ||
      text.color.r != 10.0F / 255.0F) {
    return 283;
  }

  const cgpui::TextCaretPaint& caret = commands[1].text_caret;
  return commands[1].kind == cgpui::PaintCommandKind::text_caret &&
                 caret.byte_offset == model.cursor() &&
                 caret.rect.origin.x == 20.0F &&
                 caret.rect.size.height == 20.0F && text.color.a > 0.0F
             ? 0
             : 144;
}

int test_text_element_text_paint_includes_glyph_metadata_for_cache() {
  cgpui::TextModel model("A\xE4\xB8\xAD");
  cgpui::TextElement element(
      &model,
      cgpui::Style{}
          .with_font(cgpui::FontDescriptor{.family = "CacheFont"})
          .with_font_size(20.0F));
  (void)element.layout(cgpui::LayoutInput{});

  cgpui::PaintList paint_list;
  element.paint(paint_list);
  const std::span<const cgpui::PaintCommand> commands = paint_list.commands();
  if (commands.size() != 2 ||
      commands[0].kind != cgpui::PaintCommandKind::text) {
    return 402;
  }

  const cgpui::TextPaint& text = commands[0].text;
  if (text.glyphs.size() != 2) {
    return 403;
  }

  const cgpui::TextGlyphPaint& first = text.glyphs[0];
  const cgpui::TextGlyphPaint& second = text.glyphs[1];
  if (first.key.font_family != "CacheFont" || first.key.font_size != 20.0F ||
      first.key.glyph_index != 0 || first.key.byte_offset != 0 ||
      first.key.byte_length != 1 || first.origin.x != 0.0F ||
      first.advance != 10.0F) {
    return 404;
  }
  return second.key.font_family == "CacheFont" &&
                 second.key.font_size == 20.0F &&
                 second.key.glyph_index == 1 &&
                 second.key.byte_offset == 1 &&
                 second.key.byte_length == 3 &&
                 second.origin.x == 10.0F &&
                 second.advance == 10.0F
             ? 0
             : 405;
}

int test_text_element_paints_caret_and_selection_metadata() {
  cgpui::TextModel model("abcd");
  model.set_selection(1, 3);
  cgpui::TextElement element(
      &model,
      cgpui::Style{}
          .with_font(cgpui::FontDescriptor{.family = "Mono"})
          .with_font_size(20.0F));
  (void)element.layout(cgpui::LayoutInput{});

  cgpui::PaintList paint_list;
  element.paint(paint_list);
  const std::span<const cgpui::PaintCommand> commands = paint_list.commands();
  if (commands.size() != 3) {
    return 286;
  }
  if (commands[0].kind != cgpui::PaintCommandKind::text_selection ||
      commands[1].kind != cgpui::PaintCommandKind::text ||
      commands[2].kind != cgpui::PaintCommandKind::text_caret) {
    return 287;
  }

  const cgpui::TextSelectionPaint& selection = commands[0].text_selection;
  if (selection.range.start != 1 || selection.range.end != 3 ||
      selection.rect.origin.x != 10.0F || selection.rect.origin.y != 0.0F ||
      selection.rect.size.width != 20.0F ||
      selection.rect.size.height != 20.0F ||
      selection.font_size != 20.0F) {
    return 288;
  }

  const cgpui::TextCaretPaint& caret = commands[2].text_caret;
  if (caret.byte_offset != 3 || caret.rect.origin.x != 30.0F ||
      caret.rect.origin.y != 0.0F || caret.rect.size.width != 1.0F ||
      caret.rect.size.height != 20.0F || caret.font_size != 20.0F) {
    return 289;
  }

  return commands[1].text.font.family == "Mono" ? 0 : 290;
}

int test_text_element_paints_multiline_selection_and_caret_geometry() {
  cgpui::TextModel model("ab\ncd");
  model.set_selection(1, 4);
  cgpui::TextElement element(
      &model,
      cgpui::Style{}
          .with_font(cgpui::FontDescriptor{.family = "Mono"})
          .with_font_size(20.0F));
  (void)element.layout(cgpui::LayoutInput{});

  cgpui::PaintList paint_list;
  element.paint(paint_list);
  const std::span<const cgpui::PaintCommand> commands = paint_list.commands();
  if (commands.size() != 4 ||
      commands[0].kind != cgpui::PaintCommandKind::text_selection ||
      commands[1].kind != cgpui::PaintCommandKind::text_selection ||
      commands[2].kind != cgpui::PaintCommandKind::text ||
      commands[3].kind != cgpui::PaintCommandKind::text_caret) {
    return 710;
  }

  const cgpui::TextSelectionPaint& first = commands[0].text_selection;
  const cgpui::TextSelectionPaint& second = commands[1].text_selection;
  if (first.range.start != 1 || first.range.end != 2 ||
      first.rect.origin.x != 10.0F || first.rect.origin.y != 0.0F ||
      first.rect.size.width != 10.0F || first.rect.size.height != 20.0F) {
    return 711;
  }
  if (second.range.start != 3 || second.range.end != 4 ||
      second.rect.origin.x != 0.0F || second.rect.origin.y != 20.0F ||
      second.rect.size.width != 10.0F || second.rect.size.height != 20.0F) {
    return 712;
  }

  const cgpui::TextCaretPaint& caret = commands[3].text_caret;
  return caret.byte_offset == 4 && caret.rect.origin.x == 10.0F &&
                 caret.rect.origin.y == 20.0F &&
                 caret.rect.size.height == 20.0F
             ? 0
             : 713;
}

int test_text_element_scrolls_caret_into_view() {
  cgpui::TextModel model("ab\ncd\nef");
  model.set_selection(model.text().size(), model.text().size());
  cgpui::TextElement element(
      &model,
      cgpui::Style{}
          .with_font(cgpui::FontDescriptor{.family = "Mono"})
          .with_font_size(20.0F));
  (void)element.layout(cgpui::LayoutInput{});

  const std::optional<cgpui::Rect> caret =
      element.caret_rect(cgpui::DpiScale{});
  if (!caret.has_value() || caret->origin.y != 40.0F ||
      caret->size.height != 20.0F) {
    return 714;
  }

  cgpui::ScrollState state;
  state.set_viewport_size(cgpui::Size{.width = 20.0F, .height = 20.0F});
  state.set_content_size(cgpui::Size{.width = 20.0F, .height = 60.0F});
  if (!element.scroll_caret_into_view(state, cgpui::DpiScale{}) ||
      state.offset().y != 40.0F) {
    return 715;
  }

  if (element.scroll_caret_into_view(state, cgpui::DpiScale{})) {
    return 716;
  }
  return state.offset().x == 0.0F && state.offset().y == 40.0F ? 0 : 717;
}

int test_empty_text_element_still_paints_caret_metadata() {
  cgpui::TextModel model;
  cgpui::TextElement element(
      &model,
      cgpui::Style{}.with_font_size(18.0F));
  (void)element.layout(cgpui::LayoutInput{});

  cgpui::PaintList paint_list;
  element.paint(paint_list);
  const std::span<const cgpui::PaintCommand> commands = paint_list.commands();
  if (commands.size() != 1 ||
      commands[0].kind != cgpui::PaintCommandKind::text_caret) {
    return 291;
  }

  const cgpui::TextCaretPaint& caret = commands[0].text_caret;
  return caret.byte_offset == 0 && caret.rect.origin.x == 0.0F &&
                 caret.rect.size.width == 1.0F &&
                 caret.rect.size.height == 18.0F
             ? 0
             : 292;
}

int test_element_builder_builds_text_leaf() {
  cgpui::TextModel model("builder");
  std::unique_ptr<cgpui::Element> element =
      cgpui::ElementBuilder::text(model)
          .font(cgpui::FontDescriptor{.family = "Builder"})
          .font_size(18.0F)
          .foreground(cgpui::rgb(100, 110, 120))
          .build();

  auto* text = dynamic_cast<cgpui::TextElement*>(element.get());
  if (text == nullptr || text->model() != &model ||
      text->text() != model.text()) {
    return 143;
  }

  const cgpui::LayoutOutput output = text->layout(cgpui::LayoutInput{});
  if (output.size.width != 63.0F || output.size.height != 18.0F) {
    return 284;
  }
  return text->font().family == "Builder" && text->font_size() == 18.0F &&
                 text->style().foreground_color.has_value()
             ? 0
             : 285;
}

int test_element_builder_builds_child_view_placeholder() {
  cgpui::AnyElement element =
      cgpui::into_element(cgpui::child_view(cgpui::ViewId{84})
                              .size(cgpui::Size{.width = 48.0F,
                                                 .height = 36.0F}));
  auto* child_view = dynamic_cast<cgpui::ChildViewElement*>(element.get());
  if (child_view == nullptr || child_view->view_id() != cgpui::ViewId{84}) {
    return 232;
  }

  const cgpui::LayoutOutput output = child_view->layout(cgpui::LayoutInput{});
  return output.size.width == 48.0F && output.size.height == 36.0F ? 0 : 233;
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

int test_styled_element_layout_combines_author_and_external_constraints() {
  std::unique_ptr<cgpui::Element> element =
      cgpui::div()
          .size(20.0F, 200.0F)
          .padding(cgpui::edges(5.0F))
          .min_size(cgpui::Size{.width = 40.0F, .height = 50.0F})
          .max_size(cgpui::Size{.width = 90.0F, .height = 120.0F})
          .build();

  const cgpui::LayoutOutput output = element->layout(cgpui::LayoutInput{
      .constraints =
          {
              .min_size = {.width = 60.0F, .height = 30.0F},
              .max_size = {.width = 80.0F, .height = 150.0F},
          },
  });
  if (output.size.width != 60.0F || output.size.height != 120.0F) {
    return 590;
  }

  const std::optional<cgpui::Rect> bounds = element->layout_bounds();
  return bounds.has_value() && bounds->size.width == 60.0F &&
                 bounds->size.height == 120.0F
             ? 0
             : 591;
}

int test_styled_element_percentage_size_falls_back_without_finite_parent() {
  std::unique_ptr<cgpui::Element> element =
      cgpui::div()
          .w_pct(50.0F)
          .h_pct(25.0F)
          .child(std::make_unique<cgpui::FixedSizeElement>(
              cgpui::Size{.width = 40.0F, .height = 20.0F}))
          .build();

  const cgpui::LayoutOutput unconstrained =
      element->layout(cgpui::LayoutInput{});
  if (unconstrained.size.width != 40.0F ||
      unconstrained.size.height != 20.0F) {
    return 595;
  }

  const cgpui::LayoutOutput constrained = element->layout(cgpui::LayoutInput{
      .constraints =
          {
              .max_size = {.width = 200.0F, .height = 400.0F},
          },
  });
  return constrained.size.width == 100.0F &&
                 constrained.size.height == 100.0F
             ? 0
             : 596;
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

int test_element_tree_paints_layers_with_explicit_z_index_precedence() {
  cgpui::ElementTree tree;
  const cgpui::Color root_color{
      .r = 0.1F,
      .g = 0.0F,
      .b = 0.0F,
      .a = 1.0F,
  };
  const cgpui::Color low_layer_color{
      .r = 0.2F,
      .g = 0.0F,
      .b = 0.0F,
      .a = 1.0F,
  };
  const cgpui::Color high_layer_color{
      .r = 0.3F,
      .g = 0.0F,
      .b = 0.0F,
      .a = 1.0F,
  };
  const cgpui::Color explicit_z_color{
      .r = 0.4F,
      .g = 0.0F,
      .b = 0.0F,
      .a = 1.0F,
  };
  const cgpui::Color stable_first_color{
      .r = 0.5F,
      .g = 0.0F,
      .b = 0.0F,
      .a = 1.0F,
  };
  const cgpui::Color stable_second_color{
      .r = 0.6F,
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
  const cgpui::ElementId high_layer_id = tree.append_child(
      root_id,
      cgpui::div()
          .background(high_layer_color)
          .size(20.0F, 10.0F)
          .layer(5)
          .build());
  const cgpui::ElementId low_layer_id = tree.append_child(
      root_id,
      cgpui::div()
          .background(low_layer_color)
          .size(20.0F, 10.0F)
          .layer(-2)
          .build());
  const cgpui::ElementId explicit_z_id = tree.append_child(
      root_id,
      cgpui::ElementBuilder::box()
          .style(cgpui::Style{}
                     .with_background_color(explicit_z_color)
                     .with_preferred_size(
                         cgpui::Size{.width = 20.0F, .height = 10.0F})
                     .with_layer(-10)
                     .with_z_index(2))
          .build());
  const cgpui::ElementId stable_first_id = tree.append_child(
      root_id,
      cgpui::div()
          .background(stable_first_color)
          .size(20.0F, 10.0F)
          .layer(5)
          .build());
  const cgpui::ElementId stable_second_id = tree.append_child(
      root_id,
      cgpui::div()
          .background(stable_second_color)
          .size(20.0F, 10.0F)
          .layer(5)
          .build());

  for (const cgpui::ElementId id :
       {root_id,
        high_layer_id,
        low_layer_id,
        explicit_z_id,
        stable_first_id,
        stable_second_id}) {
    tree.get(id)->set_layout_bounds(cgpui::Rect{
        .origin = {.x = 0.0F, .y = 0.0F},
        .size = {.width = 20.0F, .height = 10.0F},
    });
  }

  cgpui::PaintList paint_list;
  tree.paint(paint_list);
  const std::span<const cgpui::PaintCommand> commands = paint_list.commands();
  if (commands.size() != 6) {
    return 266;
  }
  if (commands[0].solid_rect.color.r != root_color.r) {
    return 267;
  }
  if (commands[1].solid_rect.color.r != low_layer_color.r ||
      commands[2].solid_rect.color.r != explicit_z_color.r ||
      commands[3].solid_rect.color.r != high_layer_color.r ||
      commands[4].solid_rect.color.r != stable_first_color.r ||
      commands[5].solid_rect.color.r != stable_second_color.r) {
    return 268;
  }

  return 0;
}

int test_styled_element_orders_direct_overlay_children_by_z_order() {
  const cgpui::Color high_layer_color{
      .r = 0.1F,
      .g = 0.0F,
      .b = 0.0F,
      .a = 1.0F,
  };
  const cgpui::Color explicit_z_color{
      .r = 0.2F,
      .g = 0.0F,
      .b = 0.0F,
      .a = 1.0F,
  };
  const cgpui::Color stable_high_layer_color{
      .r = 0.3F,
      .g = 0.0F,
      .b = 0.0F,
      .a = 1.0F,
  };
  const cgpui::Color low_layer_color{
      .r = 0.4F,
      .g = 0.0F,
      .b = 0.0F,
      .a = 1.0F,
  };

  std::unique_ptr<cgpui::Element> element =
      cgpui::div()
          .size(20.0F, 20.0F)
          .child(cgpui::div()
                     .background(high_layer_color)
                     .size(10.0F, 10.0F)
                     .absolute()
                     .top(0.0F)
                     .left(0.0F)
                     .layer(5)
                     .build())
          .child(cgpui::div()
                     .background(explicit_z_color)
                     .size(10.0F, 10.0F)
                     .absolute()
                     .top(0.0F)
                     .left(0.0F)
                     .layer(-10)
                     .z_index(2)
                     .build())
          .child(cgpui::div()
                     .background(stable_high_layer_color)
                     .size(10.0F, 10.0F)
                     .absolute()
                     .top(0.0F)
                     .left(0.0F)
                     .layer(5)
                     .build())
          .child(cgpui::div()
                     .background(low_layer_color)
                     .size(10.0F, 10.0F)
                     .absolute()
                     .top(0.0F)
                     .left(0.0F)
                     .layer(-2)
                     .build())
          .build();

  auto* root = dynamic_cast<cgpui::StyledElement*>(element.get());
  if (root == nullptr || root->children().size() != 4) {
    return 373;
  }
  root->children()[0]->assign_id(cgpui::ElementId{11});
  root->children()[1]->assign_id(cgpui::ElementId{22});
  root->children()[2]->assign_id(cgpui::ElementId{33});
  root->children()[3]->assign_id(cgpui::ElementId{44});

  const cgpui::LayoutOutput output = root->layout(cgpui::LayoutInput{});
  if (output.size.width != 20.0F || output.size.height != 20.0F) {
    return 374;
  }

  cgpui::PaintList paint_list;
  root->paint(paint_list);
  const std::span<const cgpui::PaintCommand> commands = paint_list.commands();
  if (commands.size() != 4) {
    return 375;
  }
  if (commands[0].solid_rect.color.r != low_layer_color.r ||
      commands[1].solid_rect.color.r != explicit_z_color.r ||
      commands[2].solid_rect.color.r != high_layer_color.r ||
      commands[3].solid_rect.color.r != stable_high_layer_color.r) {
    return 376;
  }

  const cgpui::ElementId hit = root->hit_test(cgpui::Point{.x = 1.0F, .y = 1.0F});
  return hit == cgpui::ElementId{33} ? 0 : 377;
}

int test_styled_element_dispatches_direct_overlay_events_by_z_order() {
  auto high_layer_child = std::make_unique<EventCountingElement>();
  high_layer_child->set_layer(5);
  high_layer_child->result = cgpui::EventResult::consumed_event();
  EventCountingElement* high_layer_ptr = high_layer_child.get();

  auto low_layer_child = std::make_unique<EventCountingElement>();
  low_layer_child->set_layer(-2);
  low_layer_child->result = cgpui::EventResult::consumed_event();
  EventCountingElement* low_layer_ptr = low_layer_child.get();

  std::vector<std::unique_ptr<cgpui::Element>> children;
  children.push_back(std::move(high_layer_child));
  children.push_back(std::move(low_layer_child));
  cgpui::StyledElement root(cgpui::Style{}, std::move(children));

  const cgpui::PlatformEvent event =
      cgpui::PointerMoved{.position = {.x = 1.0F, .y = 1.0F}};
  const cgpui::EventResult result =
      root.handle_event(event, cgpui::ElementEventContext{});
  if (!result.consumed || result.cancelled) {
    return 378;
  }
  if (high_layer_ptr->event_count != 1 || low_layer_ptr->event_count != 0) {
    return 379;
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

int test_element_tree_reports_preorder_ids_in_structure_order() {
  cgpui::ElementTree tree;
  const cgpui::ElementId root_id =
      tree.set_root(std::make_unique<NamedElement>(1));
  const cgpui::ElementId first_child_id =
      tree.append_child(root_id, std::make_unique<NamedElement>(2));
  const cgpui::ElementId second_child_id =
      tree.append_child(root_id, std::make_unique<NamedElement>(3));
  const cgpui::ElementId grandchild_id =
      tree.append_child(first_child_id, std::make_unique<NamedElement>(4));

  const std::vector<cgpui::ElementId> ids = tree.preorder_ids();
  const std::vector<cgpui::ElementId> expected{
      root_id,
      first_child_id,
      grandchild_id,
      second_child_id,
  };

  return ids == expected ? 0 : 155;
}

int test_element_tree_preorder_ids_handles_empty_tree() {
  cgpui::ElementTree tree;
  return tree.preorder_ids().empty() ? 0 : 156;
}

int test_element_tree_reports_enabled_preorder_ids() {
  cgpui::ElementTree tree;
  const cgpui::ElementId root_id =
      tree.set_root(std::make_unique<NamedElement>(1));
  const cgpui::ElementId disabled_child_id =
      tree.append_child(root_id, std::make_unique<NamedElement>(2));
  const cgpui::ElementId grandchild_id =
      tree.append_child(disabled_child_id, std::make_unique<NamedElement>(3));
  const cgpui::ElementId enabled_child_id =
      tree.append_child(root_id, std::make_unique<NamedElement>(4));

  tree.get(disabled_child_id)->set_enabled(false);

  const std::vector<cgpui::ElementId> ids = tree.enabled_preorder_ids();
  const std::vector<cgpui::ElementId> expected{
      root_id,
      grandchild_id,
      enabled_child_id,
  };

  return ids == expected ? 0 : 184;
}

int test_accessibility_tree_reports_roles_names_and_focus_state() {
  cgpui::ElementTree tree;
  const cgpui::ElementId root_id =
      tree.set_root(cgpui::into_element(cgpui::v_stack()));
  const cgpui::ElementId label_id =
      tree.append_child(root_id, cgpui::label("Name").build());
  const cgpui::ElementId button_id = tree.append_child(
      root_id,
      cgpui::button("dialog.save")
          .child(cgpui::label("Save").build())
          .build());
  cgpui::TextModel input_model("Ada");
  const cgpui::ElementId input_id =
      tree.append_child(root_id, cgpui::text_input(input_model).build());

  (void)tree.layout_root(cgpui::LayoutInput{});
  tree.get(label_id)->set_layout_bounds(cgpui::Rect{
      .origin = {.x = 0.0F, .y = 0.0F},
      .size = {.width = 32.0F, .height = 16.0F}});
  tree.get(button_id)->set_layout_bounds(cgpui::Rect{
      .origin = {.x = 0.0F, .y = 18.0F},
      .size = {.width = 48.0F, .height = 20.0F}});
  tree.get(input_id)->set_layout_bounds(cgpui::Rect{
      .origin = {.x = 0.0F, .y = 40.0F},
      .size = {.width = 64.0F, .height = 20.0F}});

  const cgpui::AccessibilityTreeSnapshot snapshot =
      tree.accessibility_snapshot(cgpui::AccessibilitySnapshotOptions{
          .focused_element_id = input_id,
      });

  if (snapshot.root_element_id != root_id || snapshot.nodes.size() != 4) {
    return 285;
  }

  const cgpui::AccessibilityNode* root = snapshot.node(root_id);
  const cgpui::AccessibilityNode* label = snapshot.node(label_id);
  const cgpui::AccessibilityNode* button = snapshot.node(button_id);
  const cgpui::AccessibilityNode* input = snapshot.node(input_id);
  if (root == nullptr || label == nullptr || button == nullptr ||
      input == nullptr) {
    return 286;
  }

  if (root->role != cgpui::AccessibilityRole::generic ||
      root->parent_element_id.has_value() || root->children.size() != 3 ||
      root->children[0] != label_id || root->children[1] != button_id ||
      root->children[2] != input_id) {
    return 287;
  }
  if (label->role != cgpui::AccessibilityRole::label ||
      label->name != "Name" || label->text != "Name" ||
      label->parent_element_id != root_id || label->focusable ||
      label->focused || !label->enabled || !label->bounds.has_value()) {
    return 288;
  }
  if (button->role != cgpui::AccessibilityRole::button ||
      button->name != "Save" || button->text != "" ||
      button->parent_element_id != root_id || !button->focusable ||
      button->focused || !button->enabled || !button->bounds.has_value()) {
    return 289;
  }
  if (input->role != cgpui::AccessibilityRole::text_input ||
      input->name != "Ada" || input->text != "Ada" ||
      input->parent_element_id != root_id || !input->focusable ||
      !input->focused || !input->enabled || !input->bounds.has_value()) {
    return 290;
  }

  return 0;
}

int test_accessibility_tree_reports_focus_metadata() {
  cgpui::ElementTree tree;
  std::unique_ptr<cgpui::Element> root =
      cgpui::ElementBuilder::box()
          .focusable()
          .tab_index(2)
          .focus_ring(cgpui::FocusRingVisibility::hidden)
          .build();
  const cgpui::ElementId root_id = tree.set_root(std::move(root));

  const cgpui::AccessibilityTreeSnapshot snapshot =
      tree.accessibility_snapshot();
  const cgpui::AccessibilityNode* root_node = snapshot.node(root_id);
  if (root_node == nullptr || root_node->tab_index != 2 ||
      root_node->focus_ring != cgpui::FocusRingVisibility::hidden) {
    return 415;
  }
  return 0;
}

int test_element_tree_enabled_preorder_ids_handles_empty_tree() {
  cgpui::ElementTree tree;
  return tree.enabled_preorder_ids().empty() ? 0 : 185;
}

int test_element_tree_finds_element_by_id_and_type() {
  cgpui::ElementTree tree;
  const cgpui::ElementId root_id =
      tree.set_root(std::make_unique<NamedElement>(42));
  const cgpui::ElementId child_id =
      tree.append_child(root_id, std::make_unique<cgpui::FixedSizeElement>(
                                     cgpui::Size{.width = 8.0F,
                                                 .height = 9.0F}));

  NamedElement* root = tree.find_as<NamedElement>(root_id);
  const cgpui::FixedSizeElement* child =
      std::as_const(tree).find_as<cgpui::FixedSizeElement>(child_id);
  if (root == nullptr || root->value() != 42 || root->id() != root_id) {
    return 157;
  }
  if (child == nullptr || child->preferred_size().width != 8.0F ||
      child->preferred_size().height != 9.0F || child->id() != child_id) {
    return 158;
  }
  if (tree.find_as<cgpui::FixedSizeElement>(root_id) != nullptr ||
      tree.find_as<NamedElement>(cgpui::ElementId{999}) != nullptr) {
    return 159;
  }

  return 0;
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

int test_base_element_is_not_focusable_by_default() {
  TestElement element;
  element.assign_id(cgpui::ElementId{33});

  element.focus(cgpui::ElementFocusContext{.element_id = element.id()});

  return !element.focusable() ? 0 : 145;
}

int test_focusable_element_activation_hook_observes_element_id() {
  FocusableCountingElement element;
  element.assign_id(cgpui::ElementId{34});

  if (!element.focusable()) {
    return 146;
  }

  element.focus(cgpui::ElementFocusContext{.element_id = element.id()});

  if (element.focus_count != 1 ||
      element.last_focused_element_id != cgpui::ElementId{34}) {
    return 147;
  }
  return 0;
}

int test_base_element_enabled_state_defaults_to_enabled_and_can_toggle() {
  TestElement element;

  if (!element.enabled()) {
    return 148;
  }

  element.set_enabled(false);
  if (element.enabled()) {
    return 149;
  }

  element.set_enabled(true);
  return element.enabled() ? 0 : 150;
}

int test_element_builder_applies_enabled_state_to_built_element() {
  std::unique_ptr<cgpui::Element> disabled =
      cgpui::ElementBuilder::box().enabled(false).build();
  std::unique_ptr<cgpui::Element> enabled =
      cgpui::ElementBuilder::box().enabled(true).build();

  if (disabled == nullptr || enabled == nullptr) {
    return 151;
  }
  if (disabled->enabled() || !enabled->enabled()) {
    return 152;
  }
  return 0;
}

int test_element_builder_disabled_helper_disables_built_element() {
  std::unique_ptr<cgpui::Element> element =
      cgpui::ElementBuilder::box().disabled().build();

  return element != nullptr && !element->enabled() ? 0 : 179;
}

int test_element_builder_disabled_helper_composes_with_wrappers() {
  int click_count = 0;
  std::unique_ptr<cgpui::Element> element =
      cgpui::ElementBuilder::box()
          .focusable()
          .on_click([&](const cgpui::ElementEventContext&) {
            click_count += 1;
            return cgpui::EventResult::consumed_event();
          })
          .disabled()
          .build();

  cgpui::ElementEventContext click_context{
      .target_element_id = element->id()};
  click_context.gesture = cgpui::ElementGestureKind::click;
  const cgpui::EventResult result = element->handle_event(
      cgpui::PointerButton{
          .button = cgpui::MouseButton::left,
          .pressed = false,
          .position = {.x = 1.0F, .y = 2.0F}},
      click_context);

  if (element->enabled()) {
    return 180;
  }
  if (!element->focusable()) {
    return 181;
  }
  if (click_count != 0) {
    return 182;
  }
  return !result.consumed && !result.cancelled ? 0 : 183;
}

int test_element_builder_focusable_helper_marks_built_element_focusable() {
  std::unique_ptr<cgpui::Element> element =
      cgpui::ElementBuilder::box().focusable().build();
  element->assign_id(cgpui::ElementId{37});

  if (!element->focusable()) {
    return 166;
  }

  element->focus(cgpui::ElementFocusContext{.element_id = element->id()});
  return element->enabled() ? 0 : 167;
}

int test_element_builder_focusable_helper_preserves_disabled_state() {
  std::unique_ptr<cgpui::Element> element =
      cgpui::ElementBuilder::box().enabled(false).focusable().build();

  if (element->enabled()) {
    return 168;
  }
  return element->focusable() ? 0 : 169;
}

int test_element_builder_records_focus_metadata() {
  std::unique_ptr<cgpui::Element> element =
      cgpui::ElementBuilder::box()
          .tab_index(4)
          .focus_ring(cgpui::FocusRingVisibility::visible)
          .focusable()
          .build();

  if (element == nullptr || !element->focusable()) {
    return 413;
  }

  const cgpui::FocusMetadata metadata = element->focus_metadata();
  if (!metadata.tab_index.has_value() || *metadata.tab_index != 4 ||
      metadata.focus_ring != cgpui::FocusRingVisibility::visible) {
    return 414;
  }
  return 0;
}

int test_element_builder_focusable_helper_composes_with_click_handler() {
  int click_count = 0;
  std::unique_ptr<cgpui::Element> element =
      cgpui::ElementBuilder::box()
          .focusable()
          .on_click([&](const cgpui::ElementEventContext&) {
            click_count += 1;
            return cgpui::EventResult::consumed_event();
          })
          .build();
  element->assign_id(cgpui::ElementId{38});

  if (!element->focusable()) {
    return 170;
  }

  const cgpui::EventResult press_result = element->handle_event(
      cgpui::PointerButton{
          .button = cgpui::MouseButton::left,
          .pressed = true,
          .position = {.x = 1.0F, .y = 2.0F}},
      cgpui::ElementEventContext{.target_element_id = element->id()});

  if (click_count != 0) {
    return 171;
  }

  cgpui::ElementEventContext click_context{
      .target_element_id = element->id()};
  click_context.gesture = cgpui::ElementGestureKind::click;
  const cgpui::EventResult click_result = element->handle_event(
      cgpui::PointerButton{
          .button = cgpui::MouseButton::left,
          .pressed = false,
          .position = {.x = 1.0F, .y = 2.0F}},
      click_context);

  if (click_count != 1) {
    return 172;
  }
  if (press_result.consumed || press_result.cancelled) {
    return 173;
  }
  return click_result.consumed && !click_result.cancelled ? 0 : 174;
}

int test_element_builder_key_handler_runs_on_keyboard_event() {
  int key_count = 0;
  std::uint32_t last_key_code = 0;
  cgpui::ElementId handled_element_id{};
  std::unique_ptr<cgpui::Element> element =
      cgpui::ElementBuilder::box()
          .on_key([&](
                      const cgpui::KeyboardKey& key,
                      const cgpui::ElementEventContext& context) {
            key_count += 1;
            last_key_code = key.key_code;
            handled_element_id = context.target_element_id;
            return cgpui::EventResult::consumed_event();
          })
          .build();
  element->assign_id(cgpui::ElementId{39});

  const cgpui::EventResult key_result = element->handle_event(
      cgpui::KeyboardKey{
          .key_code = 65,
          .action = cgpui::KeyAction::pressed,
          .modifiers = {.control = true}},
      cgpui::ElementEventContext{.target_element_id = element->id()});
  const cgpui::EventResult pointer_result = element->handle_event(
      cgpui::PointerMoved{.position = {.x = 1.0F, .y = 2.0F}},
      cgpui::ElementEventContext{.target_element_id = element->id()});

  if (key_count != 1 || last_key_code != 65 ||
      handled_element_id != cgpui::ElementId{39}) {
    return 173;
  }
  if (!key_result.consumed || key_result.cancelled) {
    return 174;
  }
  return !pointer_result.consumed && !pointer_result.cancelled ? 0 : 175;
}

int test_element_builder_key_handler_respects_disabled_state() {
  int key_count = 0;
  std::unique_ptr<cgpui::Element> element =
      cgpui::ElementBuilder::box()
          .enabled(false)
          .on_key([&](
                      const cgpui::KeyboardKey&,
                      const cgpui::ElementEventContext&) {
            key_count += 1;
            return cgpui::EventResult::consumed_event();
          })
          .build();
  element->assign_id(cgpui::ElementId{40});

  const cgpui::EventResult result = element->handle_event(
      cgpui::KeyboardKey{.key_code = 65, .action = cgpui::KeyAction::pressed},
      cgpui::ElementEventContext{.target_element_id = element->id()});

  if (element->enabled()) {
    return 176;
  }
  if (key_count != 0) {
    return 177;
  }
  return !result.consumed && !result.cancelled ? 0 : 178;
}

int test_element_builder_click_handler_runs_on_synthesized_click() {
  int click_count = 0;
  cgpui::ElementId clicked_element_id{};
  std::unique_ptr<cgpui::Element> element =
      cgpui::ElementBuilder::box()
          .on_click([&](const cgpui::ElementEventContext& context) {
            click_count += 1;
            clicked_element_id = context.target_element_id;
            return cgpui::EventResult::consumed_event();
          })
          .build();
  element->assign_id(cgpui::ElementId{35});

  const cgpui::EventResult press_result = element->handle_event(
      cgpui::PointerButton{
          .button = cgpui::MouseButton::left,
          .pressed = true,
          .position = {.x = 1.0F, .y = 2.0F}},
      cgpui::ElementEventContext{.target_element_id = element->id()});
  cgpui::ElementEventContext click_context{
      .target_element_id = element->id()};
  click_context.gesture = cgpui::ElementGestureKind::click;
  const cgpui::EventResult release_result = element->handle_event(
      cgpui::PointerButton{
          .button = cgpui::MouseButton::left,
          .pressed = false,
          .position = {.x = 1.0F, .y = 2.0F}},
      click_context);

  if (click_count != 1 || clicked_element_id != cgpui::ElementId{35}) {
    return 160;
  }
  if (press_result.consumed || press_result.cancelled) {
    return 161;
  }
  return release_result.consumed && !release_result.cancelled ? 0 : 162;
}

int test_element_builder_click_handler_respects_disabled_state() {
  int click_count = 0;
  std::unique_ptr<cgpui::Element> element =
      cgpui::ElementBuilder::box()
          .enabled(false)
          .on_click([&](const cgpui::ElementEventContext&) {
            click_count += 1;
            return cgpui::EventResult::consumed_event();
          })
          .build();
  element->assign_id(cgpui::ElementId{36});

  const cgpui::EventResult result = element->handle_event(
      cgpui::PointerButton{
          .button = cgpui::MouseButton::left,
          .pressed = true,
          .position = {.x = 1.0F, .y = 2.0F}},
      cgpui::ElementEventContext{.target_element_id = element->id()});

  if (element->enabled()) {
    return 163;
  }
  if (click_count != 0) {
    return 164;
  }
  return !result.consumed && !result.cancelled ? 0 : 165;
}

int test_button_widget_composes_click_focus_disabled_and_style_state() {
  int click_count = 0;
  cgpui::AnyElement element =
      cgpui::button("dialog.accept")
          .on_click([&](const cgpui::ElementEventContext& context) {
            click_count += static_cast<int>(context.target_element_id.value);
            return cgpui::EventResult::unhandled();
          })
          .style(cgpui::Style{}
                     .with_background_color(cgpui::rgb(30, 40, 50))
                     .with_padding(cgpui::edges(6.0F)))
          .hover_style(cgpui::StyleOverlay{}.with_background_color(
              cgpui::rgb(40, 50, 60)))
          .focus_style(cgpui::StyleOverlay{}.with_border_width(
              cgpui::edges(2.0F)))
          .active_style(cgpui::StyleOverlay{}
                            .with_background_color(cgpui::rgb(50, 60, 70))
                            .with_opacity(0.4F))
          .disabled_style(cgpui::StyleOverlay{}.with_foreground_color(
              cgpui::rgb(120, 120, 120)))
          .child(cgpui::div().size(10.0F, 6.0F))
          .build();
  element->assign_id(cgpui::ElementId{39});

  auto* button = dynamic_cast<cgpui::ButtonElement*>(element.get());
  if (button == nullptr) {
    return 340;
  }
  if (!button->focusable() || !button->enabled() ||
      button->action_name() != "dialog.accept") {
    return 341;
  }
  if (button->child() == nullptr) {
    return 342;
  }

  const cgpui::Style resolved = cgpui::resolved_style(
      button->style_state(),
      cgpui::StyleStateFlags{
          .hovered = true,
          .focused = true,
          .active = true,
      });
  if (!resolved.background_color.has_value() ||
      resolved.background_color->r != 50.0F / 255.0F ||
      resolved.border_width.left != 2.0F ||
      resolved.padding.left != 6.0F ||
      resolved.opacity != 0.4F) {
    return 343;
  }

  cgpui::ElementEventContext click_context{
      .target_element_id = button->id()};
  click_context.gesture = cgpui::ElementGestureKind::click;
  const cgpui::EventResult result = button->handle_event(
      cgpui::PointerButton{
          .button = cgpui::MouseButton::left,
          .pressed = false,
          .position = {.x = 1.0F, .y = 2.0F}},
      click_context);
  if (click_count != 39 || result.consumed || result.cancelled) {
    return 344;
  }

  button->set_enabled(false);
  const cgpui::EventResult disabled_result = button->handle_event(
      cgpui::PointerButton{
          .button = cgpui::MouseButton::left,
          .pressed = false,
          .position = {.x = 1.0F, .y = 2.0F}},
      click_context);
  if (click_count != 39 || disabled_result.consumed ||
      disabled_result.cancelled) {
    return 345;
  }

  const cgpui::Style disabled_style = cgpui::resolved_style(
      button->style_state(),
      cgpui::StyleStateFlags{.disabled = true});
  return disabled_style.foreground_color.has_value() &&
                 disabled_style.foreground_color->r == 120.0F / 255.0F
             ? 0
             : 346;
}

int test_button_widget_paints_style_box_before_child() {
  const cgpui::Color background{.r = 0.1F, .g = 0.2F, .b = 0.3F, .a = 1.0F};
  const cgpui::Color border{.r = 0.4F, .g = 0.5F, .b = 0.6F, .a = 1.0F};
  const cgpui::Color child_color{
      .r = 0.7F,
      .g = 0.8F,
      .b = 0.9F,
      .a = 1.0F,
  };
  cgpui::AnyElement element =
      cgpui::button("dialog.paint")
          .style(cgpui::Style{}
                     .with_padding(cgpui::edges(3.0F))
                     .with_background_color(background)
                     .with_border_color(border)
                     .with_border_width(cgpui::edges(2.0F))
                     .with_border_radius(cgpui::BorderRadii::all(5.0F)))
          .child(cgpui::div()
                     .size(cgpui::Size{.width = 8.0F, .height = 6.0F})
                     .background(child_color))
          .build();

  const cgpui::LayoutOutput output = element->layout(cgpui::LayoutInput{});
  if (output.size.width != 14.0F || output.size.height != 12.0F) {
    return 347;
  }

  cgpui::PaintList paint_list;
  element->paint(paint_list);
  const std::span<const cgpui::PaintCommand> commands = paint_list.commands();
  if (commands.size() != 6) {
    return 348;
  }
  if (commands[0].kind != cgpui::PaintCommandKind::rounded_rect) {
    return 349;
  }

  const cgpui::RoundedRect& rounded = commands[0].rounded_rect;
  if (rounded.rect.origin.x != 0.0F || rounded.rect.origin.y != 0.0F ||
      rounded.rect.size.width != 14.0F ||
      rounded.rect.size.height != 12.0F || rounded.color.r != background.r ||
      rounded.radius.top_left != 5.0F) {
    return 350;
  }
  if (commands[1].solid_rect.color.r != border.r ||
      commands[4].solid_rect.color.b != border.b) {
    return 351;
  }

  const cgpui::SolidRect& child_rect = commands[5].solid_rect;
  return child_rect.color.r == child_color.r &&
                 child_rect.rect.origin.x == 3.0F &&
                 child_rect.rect.origin.y == 3.0F &&
                 child_rect.rect.size.width == 8.0F &&
                 child_rect.rect.size.height == 6.0F
             ? 0
             : 352;
}

int test_widget_paint_command_stream_matches_snapshot() {
  cgpui::TextModel input_model("input");
  input_model.set_selection(1, 4);
  cgpui::PaintList paint_list;

  cgpui::AnyElement button =
      cgpui::button("snapshot.accept")
          .style(cgpui::Style{}
                     .with_padding(cgpui::edges(2.0F))
                     .with_background_color(cgpui::rgb(20, 30, 40))
                     .with_border_color(cgpui::rgb(50, 60, 70))
                     .with_border_width(cgpui::edges(1.0F))
                     .with_border_radius(cgpui::BorderRadii::all(3.0F)))
          .child(cgpui::label("OK")
                     .foreground(cgpui::rgb(220, 230, 240))
                     .font(cgpui::FontDescriptor{.family = "Snapshot"})
                     .font_size(12.0F)
                     .build())
          .build();
  (void)button->layout(cgpui::LayoutInput{});
  button->paint(paint_list);

  cgpui::AnyElement input =
      cgpui::text_input(input_model)
          .foreground(cgpui::rgb(10, 20, 30))
          .font(cgpui::FontDescriptor{.family = "SnapshotInput"})
          .font_size(18.0F)
          .build();
  (void)input->layout(cgpui::LayoutInput{});
  input->paint(paint_list);

  const std::string snapshot =
      snapshot_paint_commands(paint_list.commands());
  const std::string expected =
      "0 rounded_rect rect=(0.0,0.0 16.0x16.0) color=0.078,0.118,0.157,1.000 radius=3.0,3.0,3.0,3.0 clip=none opacity=1.000 transform=[1.0,0.0,0.0,1.0,0.0,0.0]\n"
      "1 solid_rect rect=(0.0,0.0 16.0x1.0) color=0.196,0.235,0.275,1.000 clip=none opacity=1.000 transform=[1.0,0.0,0.0,1.0,0.0,0.0]\n"
      "2 solid_rect rect=(15.0,1.0 1.0x14.0) color=0.196,0.235,0.275,1.000 clip=none opacity=1.000 transform=[1.0,0.0,0.0,1.0,0.0,0.0]\n"
      "3 solid_rect rect=(0.0,15.0 16.0x1.0) color=0.196,0.235,0.275,1.000 clip=none opacity=1.000 transform=[1.0,0.0,0.0,1.0,0.0,0.0]\n"
      "4 solid_rect rect=(0.0,1.0 1.0x14.0) color=0.196,0.235,0.275,1.000 clip=none opacity=1.000 transform=[1.0,0.0,0.0,1.0,0.0,0.0]\n"
      "5 text bounds=(2.0,2.0 12.0x12.0) color=0.863,0.902,0.941,1.000 content=\"OK\" font=Snapshot size=12.0 device_size=12.0 glyphs=2 clip=none opacity=1.000 transform=[1.0,0.0,0.0,1.0,0.0,0.0]\n"
      "6 text_selection rect=(9.0,0.0 27.0x18.0) color=0.220,0.420,0.800,0.380 range=1..4 size=18.0 clip=none opacity=1.000 transform=[1.0,0.0,0.0,1.0,0.0,0.0]\n"
      "7 text bounds=(0.0,0.0 45.0x18.0) color=0.039,0.078,0.118,1.000 content=\"input\" font=SnapshotInput size=18.0 device_size=18.0 glyphs=5 clip=none opacity=1.000 transform=[1.0,0.0,0.0,1.0,0.0,0.0]\n"
      "8 text_caret rect=(36.0,0.0 1.0x18.0) color=0.039,0.078,0.118,1.000 byte=4 size=18.0 clip=none opacity=1.000 transform=[1.0,0.0,0.0,1.0,0.0,0.0]\n";

  return snapshot == expected ? 0 : 372;
}

int test_element_builder_pointer_handlers_route_concrete_events() {
  int down_count = 0;
  int up_count = 0;
  int move_count = 0;
  cgpui::Point down_position{};
  cgpui::Point up_position{};
  cgpui::Point move_position{};
  cgpui::ElementId down_target{};
  cgpui::ElementId up_target{};
  cgpui::ElementId move_target{};

  std::unique_ptr<cgpui::Element> element =
      cgpui::ElementBuilder::box()
          .on_pointer_down([&](
                               const cgpui::PointerButton& event,
                               const cgpui::ElementEventContext& context) {
            down_count += 1;
            down_position = event.position;
            down_target = context.target_element_id;
            return cgpui::EventResult::consumed_event();
          })
          .on_pointer_up([&](
                             const cgpui::PointerButton& event,
                             const cgpui::ElementEventContext& context) {
            up_count += 1;
            up_position = event.position;
            up_target = context.target_element_id;
            return cgpui::EventResult::unhandled();
          })
          .on_pointer_move([&](
                               const cgpui::PointerMoved& event,
                               const cgpui::ElementEventContext& context) {
            move_count += 1;
            move_position = event.position;
            move_target = context.target_element_id;
            return cgpui::EventResult::cancelled_event();
          })
          .build();
  element->assign_id(cgpui::ElementId{41});

  const cgpui::ElementEventContext context{.target_element_id = element->id()};
  const cgpui::EventResult down_result = element->handle_event(
      cgpui::PointerButton{
          .button = cgpui::MouseButton::left,
          .pressed = true,
          .position = {.x = 1.0F, .y = 2.0F}},
      context);
  const cgpui::EventResult up_result = element->handle_event(
      cgpui::PointerButton{
          .button = cgpui::MouseButton::left,
          .pressed = false,
          .position = {.x = 3.0F, .y = 4.0F}},
      context);
  const cgpui::EventResult move_result = element->handle_event(
      cgpui::PointerMoved{.position = {.x = 5.0F, .y = 6.0F}},
      context);

  if (down_count != 1 || up_count != 1 || move_count != 1) {
    return 216;
  }
  if (down_position.x != 1.0F || down_position.y != 2.0F ||
      up_position.x != 3.0F || up_position.y != 4.0F ||
      move_position.x != 5.0F || move_position.y != 6.0F) {
    return 217;
  }
  if (down_target != cgpui::ElementId{41} ||
      up_target != cgpui::ElementId{41} ||
      move_target != cgpui::ElementId{41}) {
    return 218;
  }
  if (!down_result.consumed || down_result.cancelled) {
    return 219;
  }
  if (up_result.consumed || up_result.cancelled) {
    return 220;
  }
  return move_result.consumed && move_result.cancelled ? 0 : 221;
}

int test_element_builder_pointer_handlers_respect_disabled_state() {
  int down_count = 0;
  int up_count = 0;
  int move_count = 0;
  std::unique_ptr<cgpui::Element> element =
      cgpui::ElementBuilder::box()
          .disabled()
          .on_pointer_down([&](
                               const cgpui::PointerButton&,
                               const cgpui::ElementEventContext&) {
            down_count += 1;
            return cgpui::EventResult::consumed_event();
          })
          .on_pointer_up([&](
                             const cgpui::PointerButton&,
                             const cgpui::ElementEventContext&) {
            up_count += 1;
            return cgpui::EventResult::consumed_event();
          })
          .on_pointer_move([&](
                               const cgpui::PointerMoved&,
                               const cgpui::ElementEventContext&) {
            move_count += 1;
            return cgpui::EventResult::consumed_event();
          })
          .build();
  element->assign_id(cgpui::ElementId{42});

  const cgpui::ElementEventContext context{.target_element_id = element->id()};
  const cgpui::EventResult down_result = element->handle_event(
      cgpui::PointerButton{
          .button = cgpui::MouseButton::left,
          .pressed = true,
          .position = {.x = 1.0F, .y = 2.0F}},
      context);
  const cgpui::EventResult up_result = element->handle_event(
      cgpui::PointerButton{
          .button = cgpui::MouseButton::left,
          .pressed = false,
          .position = {.x = 3.0F, .y = 4.0F}},
      context);
  const cgpui::EventResult move_result = element->handle_event(
      cgpui::PointerMoved{.position = {.x = 5.0F, .y = 6.0F}},
      context);

  if (down_count != 0 || up_count != 0 || move_count != 0) {
    return 222;
  }
  return !down_result.consumed && !up_result.consumed &&
                 !move_result.consumed
             ? 0
             : 223;
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

int test_styled_element_skips_disabled_child_event_handling() {
  auto child = std::make_unique<EventCountingElement>();
  EventCountingElement* child_ptr = child.get();
  child_ptr->assign_id(cgpui::ElementId{34});
  child_ptr->set_enabled(false);
  child_ptr->result = cgpui::EventResult::consumed_event();

  cgpui::StyledElement element(cgpui::Style{}, std::move(child));
  element.assign_id(cgpui::ElementId{33});

  const cgpui::EventResult result = element.handle_event(
      cgpui::PointerMoved{.position = {.x = 4.0F, .y = 5.0F}},
      cgpui::ElementEventContext{
          .target_element_id = child_ptr->id(),
      });

  if (result.consumed || result.cancelled) {
    return 153;
  }
  if (child_ptr->event_count != 0 || child_ptr->saw_pointer) {
    return 154;
  }

  return 0;
}

int test_scroll_element_binds_state_and_preserves_child_layout() {
  cgpui::ScrollState state;
  cgpui::AnyElement element =
      cgpui::scroll(
          state,
          cgpui::div().size(cgpui::Size{.width = 80.0F, .height = 90.0F}));
  auto* scroll = dynamic_cast<cgpui::ScrollElement*>(element.get());
  if (scroll == nullptr || scroll->state() != &state || scroll->child() == nullptr) {
    return 224;
  }

  scroll->assign_id(cgpui::ElementId{50});
  scroll->child()->assign_id(cgpui::ElementId{51});
  const cgpui::LayoutOutput output = scroll->layout(cgpui::LayoutInput{
      .constraints =
          {
              .max_size = {.width = 40.0F, .height = 30.0F},
          },
  });

  if (output.size.width != 40.0F || output.size.height != 30.0F) {
    return 225;
  }
  if (state.viewport_size().width != 40.0F ||
      state.viewport_size().height != 30.0F ||
      state.content_size().width != 80.0F ||
      state.content_size().height != 90.0F) {
    return 226;
  }
  const std::optional<cgpui::Rect> child_bounds = scroll->child()->layout_bounds();
  if (!child_bounds.has_value() || child_bounds->size.width != 80.0F ||
      child_bounds->size.height != 90.0F) {
    return 227;
  }
  return scroll->hit_test(cgpui::Point{.x = 10.0F, .y = 10.0F}) ==
                 cgpui::ElementId{50}
             ? 0
             : 228;
}

int test_scrollable_list_container_keys_clip_and_scroll_offset() {
  cgpui::ScrollState state;
  cgpui::AnyElement element =
      cgpui::scrollable_list(state)
          .size(40.0F, 30.0F)
          .item("alpha",
                cgpui::div()
                    .size(40.0F, 20.0F)
                    .background(cgpui::rgb(10, 20, 30)))
          .item("beta",
                cgpui::div()
                    .size(40.0F, 20.0F)
                    .background(cgpui::rgb(40, 50, 60)))
          .build();

  auto* list = dynamic_cast<cgpui::ScrollableListElement*>(element.get());
  if (list == nullptr || list->state() != &state || list->item_count() != 2) {
    return 365;
  }
  if (list->content().children()[0]->key()->value != "alpha" ||
      list->content().children()[1]->key()->value != "beta") {
    return 366;
  }

  const cgpui::LayoutOutput first_layout = list->layout(cgpui::LayoutInput{});
  if (first_layout.size.width != 40.0F || first_layout.size.height != 30.0F ||
      state.viewport_size().height != 30.0F ||
      state.content_size().height != 40.0F) {
    return 367;
  }

  state.set_offset(cgpui::Point{.x = 0.0F, .y = 10.0F});
  const cgpui::LayoutOutput scrolled_layout = list->layout(cgpui::LayoutInput{});
  if (scrolled_layout.size.width != 40.0F ||
      scrolled_layout.size.height != 30.0F ||
      state.offset().y != 10.0F) {
    return 368;
  }

  const std::optional<cgpui::Rect> alpha_bounds =
      list->content().children()[0]->layout_bounds();
  const std::optional<cgpui::Rect> beta_bounds =
      list->content().children()[1]->layout_bounds();
  if (!alpha_bounds.has_value() || !beta_bounds.has_value() ||
      alpha_bounds->origin.y != -10.0F || beta_bounds->origin.y != 10.0F) {
    return 369;
  }

  cgpui::PaintList paint_list;
  list->paint(paint_list);
  const std::span<const cgpui::PaintCommand> commands = paint_list.commands();
  if (commands.size() != 2 ||
      commands[0].kind != cgpui::PaintCommandKind::solid_rect ||
      commands[1].kind != cgpui::PaintCommandKind::solid_rect) {
    return 370;
  }
  if (!commands[0].clip_rect.has_value() ||
      !commands[1].clip_rect.has_value() ||
      commands[0].clip_rect->size.width != 40.0F ||
      commands[0].clip_rect->size.height != 30.0F ||
      commands[0].solid_rect.rect.origin.y != -10.0F ||
      commands[1].solid_rect.rect.origin.y != 10.0F) {
    return 371;
  }

  return 0;
}

int test_scrollable_list_records_stable_identity_and_visible_range() {
  cgpui::ScrollState state;
  cgpui::AnyElement element =
      cgpui::scrollable_list(state)
          .size(50.0F, 25.0F)
          .gap(2.0F)
          .item("alpha", cgpui::div().size(50.0F, 10.0F))
          .item("beta", cgpui::div().size(50.0F, 10.0F))
          .item("gamma", cgpui::div().size(50.0F, 10.0F))
          .item("delta", cgpui::div().size(50.0F, 10.0F))
          .build();

  auto* list = dynamic_cast<cgpui::ScrollableListElement*>(element.get());
  if (list == nullptr) {
    return 372;
  }

  (void)list->layout(cgpui::LayoutInput{});
  state.set_offset(cgpui::Point{.x = 0.0F, .y = 13.0F});
  (void)list->layout(cgpui::LayoutInput{});

  const cgpui::UniformListLayoutSnapshot& snapshot =
      list->layout_snapshot();
  if (snapshot.items.size() != 4 ||
      snapshot.visible_range.start_index != 1 ||
      snapshot.visible_range.end_index != 4) {
    return 373;
  }
  if (snapshot.items[0].key.value != "alpha" || snapshot.items[1].index != 1 ||
      snapshot.items[1].key.value != "beta" ||
      snapshot.items[3].key.value != "delta") {
    return 374;
  }
  if (snapshot.items[0].visible || !snapshot.items[1].visible ||
      !snapshot.items[2].visible || !snapshot.items[3].visible) {
    return 375;
  }
  if (snapshot.items[1].content_bounds.origin.y != 12.0F ||
      snapshot.items[3].content_bounds.origin.y != 36.0F ||
      snapshot.items[3].content_bounds.size.height != 10.0F) {
    return 376;
  }

  const std::optional<cgpui::Rect> beta_bounds =
      list->content().children()[1]->layout_bounds();
  return beta_bounds.has_value() && beta_bounds->origin.y == -1.0F ? 0 : 377;
}

int test_scrollable_list_anchors_keyed_item_after_preceding_size_change() {
  cgpui::ScrollState state;
  auto alpha = std::make_unique<MutableSizeElement>(
      cgpui::Size{.width = 50.0F, .height = 10.0F});
  MutableSizeElement* alpha_ptr = alpha.get();
  cgpui::AnyElement element =
      cgpui::scrollable_list(state)
          .size(50.0F, 25.0F)
          .gap(2.0F)
          .item("alpha", std::move(alpha))
          .item("beta",
                std::make_unique<MutableSizeElement>(
                    cgpui::Size{.width = 50.0F, .height = 10.0F}))
          .item("gamma",
                std::make_unique<MutableSizeElement>(
                    cgpui::Size{.width = 50.0F, .height = 10.0F}))
          .item("delta",
                std::make_unique<MutableSizeElement>(
                    cgpui::Size{.width = 50.0F, .height = 10.0F}))
          .build();

  auto* list = dynamic_cast<cgpui::ScrollableListElement*>(element.get());
  if (list == nullptr || alpha_ptr == nullptr) {
    return 378;
  }

  (void)list->layout(cgpui::LayoutInput{});
  state.set_offset(cgpui::Point{.x = 0.0F, .y = 13.0F});
  (void)list->layout(cgpui::LayoutInput{});
  const std::optional<cgpui::Rect> before_beta_bounds =
      list->content().children()[1]->layout_bounds();
  if (!before_beta_bounds.has_value() ||
      before_beta_bounds->origin.y != -1.0F) {
    return 379;
  }

  alpha_ptr->set_size(cgpui::Size{.width = 50.0F, .height = 22.0F});
  (void)list->layout(cgpui::LayoutInput{});

  const std::optional<cgpui::Rect> after_beta_bounds =
      list->content().children()[1]->layout_bounds();
  const cgpui::UniformListLayoutSnapshot& snapshot =
      list->layout_snapshot();
  if (state.offset().y != 25.0F || !after_beta_bounds.has_value() ||
      after_beta_bounds->origin.y != -1.0F) {
    return 380;
  }
  if (snapshot.items.size() != 4 ||
      snapshot.items[1].key.value != "beta" ||
      snapshot.items[1].content_bounds.origin.y != 24.0F ||
      snapshot.visible_range.start_index != 1 ||
      snapshot.visible_range.end_index != 4) {
    return 381;
  }

  return 0;
}

int test_scrollable_list_records_item_measurement_cache_hits() {
  cgpui::ScrollState state;
  cgpui::AnyElement element =
      cgpui::scrollable_list(state)
          .size(50.0F, 25.0F)
          .gap(2.0F)
          .item("alpha", cgpui::div().size(50.0F, 10.0F))
          .item("beta", cgpui::div().size(50.0F, 12.0F))
          .build();

  auto* list = dynamic_cast<cgpui::ScrollableListElement*>(element.get());
  if (list == nullptr) {
    return 382;
  }

  (void)list->layout(cgpui::LayoutInput{});
  const cgpui::UniformListLayoutSnapshot& first_snapshot =
      list->layout_snapshot();
  if (first_snapshot.measurements.size() != 2 ||
      first_snapshot.measurements[0].cache_hit ||
      first_snapshot.measurements[1].cache_hit ||
      first_snapshot.measurements[0].measurement.size.height != 10.0F ||
      first_snapshot.measurements[1].measurement.size.height != 12.0F) {
    return 383;
  }
  if (list->measurement_cache().entry_count() != 2 ||
      list->measurement_cache().lookup_count() != 2 ||
      list->measurement_cache().miss_count() != 2 ||
      list->measurement_cache().hit_count() != 0) {
    return 384;
  }

  state.set_offset(cgpui::Point{.x = 0.0F, .y = 4.0F});
  (void)list->layout(cgpui::LayoutInput{});
  const cgpui::UniformListLayoutSnapshot& second_snapshot =
      list->layout_snapshot();
  if (second_snapshot.measurements.size() != 2 ||
      !second_snapshot.measurements[0].cache_hit ||
      !second_snapshot.measurements[1].cache_hit) {
    return 385;
  }
  return list->measurement_cache().entry_count() == 2 &&
                 list->measurement_cache().lookup_count() == 4 &&
                 list->measurement_cache().miss_count() == 2 &&
                 list->measurement_cache().hit_count() == 2
             ? 0
             : 386;
}

int test_scrollable_list_recycles_large_list_paint_to_retained_window() {
  cgpui::ScrollState state;
  cgpui::ScrollableListBuilder builder =
      cgpui::scrollable_list(state).size(50.0F, 25.0F);
  for (std::size_t index = 0; index < 10; ++index) {
    builder = std::move(builder).item(
        "item-" + std::to_string(index),
        cgpui::div()
            .size(50.0F, 10.0F)
            .background(cgpui::rgb(10, 20, 30)));
  }
  cgpui::AnyElement element = std::move(builder).build();
  auto* list = dynamic_cast<cgpui::ScrollableListElement*>(element.get());
  if (list == nullptr) {
    return 608;
  }

  (void)list->layout(cgpui::LayoutInput{});
  state.set_offset(cgpui::Point{.x = 0.0F, .y = 35.0F});
  (void)list->layout(cgpui::LayoutInput{});

  const cgpui::UniformListLayoutSnapshot& snapshot =
      list->layout_snapshot();
  if (snapshot.visible_range.start_index != 3 ||
      snapshot.visible_range.end_index != 6 ||
      snapshot.recycling_window.retained_range.start_index != 2 ||
      snapshot.recycling_window.retained_range.end_index != 7 ||
      snapshot.recycling_window.retained_count != 5 ||
      snapshot.recycling_window.recycled_before_count != 2 ||
      snapshot.recycling_window.recycled_after_count != 3) {
    return 609;
  }
  if (!snapshot.items[0].recycled || !snapshot.items[1].recycled ||
      snapshot.items[2].recycled || !snapshot.items[3].visible ||
      !snapshot.items[5].visible || snapshot.items[6].visible ||
      snapshot.items[6].recycled || !snapshot.items[7].recycled) {
    return 610;
  }

  cgpui::PaintList paint_list;
  list->paint(paint_list);
  const std::span<const cgpui::PaintCommand> commands = paint_list.commands();
  if (commands.size() != 5) {
    return 611;
  }
  if (commands.front().solid_rect.rect.origin.y != -15.0F ||
      commands.back().solid_rect.rect.origin.y != 25.0F) {
    return 612;
  }
  return list->measurement_cache().entry_count() == 10 ? 0 : 613;
}

int test_scrollable_list_updates_selection_from_pointer_and_keyboard() {
  cgpui::ScrollState state;
  cgpui::AnyElement element =
      cgpui::scrollable_list(state)
          .size(50.0F, 35.0F)
          .item("alpha", cgpui::div().size(50.0F, 10.0F))
          .item("beta", cgpui::div().size(50.0F, 10.0F))
          .item("gamma", cgpui::div().size(50.0F, 10.0F))
          .build();

  auto* list = dynamic_cast<cgpui::ScrollableListElement*>(element.get());
  if (list == nullptr) {
    return 614;
  }

  (void)list->layout(cgpui::LayoutInput{});
  if (!list->selection().empty()) {
    return 615;
  }

  const cgpui::EventResult pointer_result = list->handle_event(
      cgpui::PointerButton{
          .button = cgpui::MouseButton::left,
          .pressed = true,
          .position = {.x = 5.0F, .y = 16.0F}},
      cgpui::ElementEventContext{.target_element_id = list->id()});
  if (!pointer_result.consumed || list->selection().selected_index() != 1 ||
      list->selection().selected_key()->value != "beta") {
    return 616;
  }

  const cgpui::UniformListLayoutSnapshot& pointer_snapshot =
      list->layout_snapshot();
  if (pointer_snapshot.items.size() != 3 ||
      pointer_snapshot.items[0].selected || !pointer_snapshot.items[1].selected ||
      pointer_snapshot.items[2].selected) {
    return 617;
  }

  const cgpui::EventResult keyboard_result = list->handle_event(
      cgpui::KeyboardKey{.key_code = 40, .action = cgpui::KeyAction::pressed},
      cgpui::ElementEventContext{.target_element_id = list->id()});
  if (!keyboard_result.consumed || list->selection().selected_index() != 2 ||
      list->selection().selected_key()->value != "gamma") {
    return 618;
  }

  const cgpui::EventResult release_result = list->handle_event(
      cgpui::KeyboardKey{.key_code = 40, .action = cgpui::KeyAction::released},
      cgpui::ElementEventContext{.target_element_id = list->id()});
  return !release_result.consumed && list->selection().selected_index() == 2
             ? 0
             : 619;
}

int test_hidden_overflow_intersects_nested_scrollable_list_clip() {
  cgpui::ScrollState state;
  cgpui::AnyElement list =
      cgpui::scrollable_list(state)
          .size(80.0F, 30.0F)
          .item("alpha",
                cgpui::div()
                    .size(80.0F, 20.0F)
                    .background(cgpui::rgb(10, 20, 30)))
          .item("beta",
                cgpui::div()
                    .size(80.0F, 20.0F)
                    .background(cgpui::rgb(40, 50, 60)))
          .build();

  const cgpui::Rect outer_clip{
      .origin = {.x = 0.0F, .y = 0.0F},
      .size = {.width = 60.0F, .height = 20.0F},
  };
  cgpui::AnyElement element =
      cgpui::into_element(cgpui::div()
                              .style(cgpui::Style{}
                                         .with_overflow(cgpui::Overflow::hidden)
                                         .with_clip_rect(outer_clip))
                              .child(std::move(list)));
  auto* styled = dynamic_cast<cgpui::StyledElement*>(element.get());
  if (styled == nullptr) {
    return 604;
  }

  (void)styled->layout(cgpui::LayoutInput{});
  cgpui::PaintList paint_list;
  styled->paint(paint_list);
  const std::span<const cgpui::PaintCommand> commands = paint_list.commands();
  if (commands.size() != 2) {
    return 605;
  }
  for (const cgpui::PaintCommand& command : commands) {
    if (!command.clip_rect.has_value() ||
        !same_rect(*command.clip_rect, outer_clip) ||
        command.clip_stack.full_depth != 2 ||
        command.clip_stack.clips.size() != 2 ||
        !same_rect(command.clip_stack.clips.back(), outer_clip)) {
      return 606;
    }
  }

  return commands[0].solid_rect.rect.size.width == 80.0F &&
                 commands[1].solid_rect.rect.origin.y == 20.0F
             ? 0
             : 607;
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
  if (const int result = test_any_element_into_element_preserves_owned_element();
      result != 0) {
    return result;
  }
  if (const int result = test_element_builder_into_element_builds_owned_element();
      result != 0) {
    return result;
  }
  if (const int result = test_free_authoring_factories_create_builders();
      result != 0) {
    return result;
  }
  if (const int result =
          test_free_authoring_factories_compose_with_existing_builder_methods();
      result != 0) {
    return result;
  }
  if (const int result = test_element_builder_child_accepts_builder();
      result != 0) {
    return result;
  }
  if (const int result = test_element_builder_child_accepts_any_element();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_builder_child_accepts_typed_element_ownership();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_builder_fluent_style_shortcuts_mutate_box_style();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_builder_fluent_size_shortcut_accepts_dimensions();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_builder_fluent_gap_shortcut_applies_to_stack_and_flex();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_builder_flex_vocabulary_helpers_map_to_existing_style();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_builder_sizing_color_border_vocabulary_helpers_map_to_existing_style();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_builder_layout_constraint_vocabulary_helpers_map_to_existing_style();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_builder_percentage_sizing_helpers_map_to_existing_style();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_builder_overflow_opacity_position_vocabulary_helpers_map_to_existing_style();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_builder_text_vocabulary_helpers_map_to_existing_style();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_builder_shadow_vocabulary_maps_to_style_storage_and_paint();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_builder_style_state_overlays_are_stored_on_styled_box();
      result != 0) {
    return result;
  }
  if (const int result = test_element_builder_key_applies_to_built_element();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_builder_stores_style_classes_and_inline_style();
      result != 0) {
    return result;
  }
  if (const int result = test_styled_element_resolves_theme_token_fallbacks();
      result != 0) {
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
  if (const int result =
          test_flex_row_justify_content_positions_children_on_main_axis();
      result != 0) {
    return result;
  }
  if (const int result =
          test_flex_row_align_items_positions_children_on_cross_axis();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_builder_flex_grow_and_shrink_shortcuts_apply_to_children();
      result != 0) {
    return result;
  }
  if (const int result = test_flex_row_grow_expands_children_on_main_axis();
      result != 0) {
    return result;
  }
  if (const int result = test_flex_column_shrink_reduces_children_on_main_axis();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_builder_absolute_and_inset_shortcuts_apply_to_child();
      result != 0) {
    return result;
  }
  if (const int result =
          test_flex_row_absolute_children_do_not_affect_main_layout();
      result != 0) {
    return result;
  }
  if (const int result = test_element_builder_fixed_shortcut_maps_to_position();
      result != 0) {
    return result;
  }
  if (const int result =
          test_styled_element_positioned_children_do_not_affect_vertical_flow();
      result != 0) {
    return result;
  }
  if (const int result = test_element_builder_layer_shortcut_applies_to_child();
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
  if (const int result =
          test_flex_column_justify_content_and_align_items_position_children();
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
  if (const int result =
          test_keyed_children_preserve_ids_across_reorder_insert_and_removal();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_lifecycle_root_mount_and_update_callbacks();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_lifecycle_keyed_children_mount_update_and_unmount();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_lifecycle_set_root_unmounts_previous_tree();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_tree_state_survives_keyed_reconcile_and_prunes_removed_nodes();
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
  if (const int result =
          test_div_builder_preserves_multiple_children_in_author_order();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_builder_children_accepts_owned_collection();
      result != 0) {
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
  if (const int result =
          test_styled_element_paints_rounded_background_rect_metadata();
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
  if (const int result = test_paint_list_intersects_nested_clips();
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
          test_styled_element_paint_metadata_composes_opacity_transform_in_order();
      result != 0) {
    return result;
  }
  if (const int result =
          test_styled_element_hidden_overflow_clips_hit_testing_to_bounds();
      result != 0) {
    return result;
  }
  if (const int result =
          test_styled_element_visible_overflow_preserves_child_hit_testing();
      result != 0) {
    return result;
  }
  if (const int result =
          test_styled_element_hidden_overflow_clips_hit_testing_to_explicit_clip_rect();
      result != 0) {
    return result;
  }
  if (const int result =
          test_text_element_binds_text_model_and_lays_out_skeleton();
      result != 0) {
    return result;
  }
  if (const int result =
          test_text_element_uses_font_size_for_deterministic_metrics();
      result != 0) {
    return result;
  }
  if (const int result =
          test_text_element_uses_shaping_run_for_utf8_layout_metrics();
      result != 0) {
    return result;
  }
  if (const int result =
          test_label_widget_paints_text_without_editing_metadata();
      result != 0) {
    return result;
  }
  if (const int result =
          test_text_input_widget_is_focusable_editable_text_element();
      result != 0) {
    return result;
  }
  if (const int result =
          test_styled_element_inherits_text_style_into_label_descendant();
      result != 0) {
    return result;
  }
  if (const int result =
          test_child_label_text_style_overrides_inherited_parent_style();
      result != 0) {
    return result;
  }
  if (const int result =
          test_inherited_text_style_flows_through_flex_container();
      result != 0) {
    return result;
  }
  if (const int result =
          test_child_view_element_references_view_and_lays_out_placeholder();
      result != 0) {
    return result;
  }
  if (const int result =
          test_text_element_paints_text_command_from_layout_bounds();
      result != 0) {
    return result;
  }
  if (const int result =
          test_text_element_text_paint_includes_glyph_metadata_for_cache();
      result != 0) {
    return result;
  }
  if (const int result =
          test_text_element_paints_caret_and_selection_metadata();
      result != 0) {
    return result;
  }
  if (const int result =
          test_text_element_paints_multiline_selection_and_caret_geometry();
      result != 0) {
    return result;
  }
  if (const int result = test_text_element_scrolls_caret_into_view();
      result != 0) {
    return result;
  }
  if (const int result =
          test_empty_text_element_still_paints_caret_metadata();
      result != 0) {
    return result;
  }
  if (const int result = test_element_builder_builds_text_leaf();
      result != 0) {
    return result;
  }
  if (const int result = test_element_builder_builds_child_view_placeholder();
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
          test_styled_element_layout_combines_author_and_external_constraints();
      result != 0) {
    return result;
  }
  if (const int result =
          test_styled_element_percentage_size_falls_back_without_finite_parent();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_builder_margin_padding_gap_shorthands_map_to_existing_layout();
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
  if (const int result =
          test_element_tree_paints_layers_with_explicit_z_index_precedence();
      result != 0) {
    return result;
  }
  if (const int result =
          test_styled_element_orders_direct_overlay_children_by_z_order();
      result != 0) {
    return result;
  }
  if (const int result =
          test_styled_element_dispatches_direct_overlay_events_by_z_order();
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
  if (const int result =
          test_element_tree_reports_preorder_ids_in_structure_order();
      result != 0) {
    return result;
  }
  if (const int result = test_element_tree_preorder_ids_handles_empty_tree();
      result != 0) {
    return result;
  }
  if (const int result = test_element_tree_reports_enabled_preorder_ids();
      result != 0) {
    return result;
  }
  if (const int result =
          test_accessibility_tree_reports_roles_names_and_focus_state();
      result != 0) {
    return result;
  }
  if (const int result = test_accessibility_tree_reports_focus_metadata();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_tree_enabled_preorder_ids_handles_empty_tree();
      result != 0) {
    return result;
  }
  if (const int result = test_element_tree_finds_element_by_id_and_type();
      result != 0) {
    return result;
  }
  if (const int result = test_base_element_event_handler_defaults_to_unhandled();
      result != 0) {
    return result;
  }
  if (const int result = test_base_element_is_not_focusable_by_default();
      result != 0) {
    return result;
  }
  if (const int result =
          test_focusable_element_activation_hook_observes_element_id();
      result != 0) {
    return result;
  }
  if (const int result =
          test_base_element_enabled_state_defaults_to_enabled_and_can_toggle();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_builder_applies_enabled_state_to_built_element();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_builder_disabled_helper_disables_built_element();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_builder_disabled_helper_composes_with_wrappers();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_builder_focusable_helper_marks_built_element_focusable();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_builder_focusable_helper_preserves_disabled_state();
      result != 0) {
    return result;
  }
  if (const int result = test_element_builder_records_focus_metadata();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_builder_focusable_helper_composes_with_click_handler();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_builder_key_handler_runs_on_keyboard_event();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_builder_key_handler_respects_disabled_state();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_builder_click_handler_runs_on_synthesized_click();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_builder_click_handler_respects_disabled_state();
      result != 0) {
    return result;
  }
  if (const int result =
          test_button_widget_composes_click_focus_disabled_and_style_state();
      result != 0) {
    return result;
  }
  if (const int result = test_button_widget_paints_style_box_before_child();
      result != 0) {
    return result;
  }
  if (const int result = test_widget_paint_command_stream_matches_snapshot();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_builder_pointer_handlers_route_concrete_events();
      result != 0) {
    return result;
  }
  if (const int result =
          test_element_builder_pointer_handlers_respect_disabled_state();
      result != 0) {
    return result;
  }
  if (const int result = test_styled_element_forwards_events_to_child();
      result != 0) {
    return result;
  }
  if (const int result =
          test_styled_element_skips_disabled_child_event_handling();
      result != 0) {
    return result;
  }
  if (const int result =
          test_scroll_element_binds_state_and_preserves_child_layout();
      result != 0) {
    return result;
  }
  if (const int result =
          test_scrollable_list_container_keys_clip_and_scroll_offset();
      result != 0) {
    return result;
  }
  if (const int result =
          test_scrollable_list_records_stable_identity_and_visible_range();
      result != 0) {
    return result;
  }
  if (const int result =
          test_scrollable_list_anchors_keyed_item_after_preceding_size_change();
      result != 0) {
    return result;
  }
  if (const int result =
          test_scrollable_list_records_item_measurement_cache_hits();
      result != 0) {
    return result;
  }
  if (const int result =
          test_scrollable_list_recycles_large_list_paint_to_retained_window();
      result != 0) {
    return result;
  }
  if (const int result =
          test_scrollable_list_updates_selection_from_pointer_and_keyboard();
      result != 0) {
    return result;
  }
  if (const int result =
          test_hidden_overflow_intersects_nested_scrollable_list_clip();
      result != 0) {
    return result;
  }
  return 0;
}
