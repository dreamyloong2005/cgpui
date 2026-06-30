#pragma once

#include "cgpui/core/events.hpp"
#include "cgpui/ui/layout.hpp"
#include "cgpui/ui/style.hpp"
#include "cgpui/ui/text.hpp"

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string_view>
#include <variant>
#include <vector>

namespace cgpui {

class PaintList;
class Element;
class ElementBuilder;

struct ElementId {
  std::uint64_t value = 0;

  friend bool operator==(ElementId, ElementId) = default;
};

struct ViewId {
  std::uint64_t value = 0;

  friend bool operator==(ViewId, ViewId) = default;
};

struct EventResult {
  bool consumed = false;
  bool cancelled = false;

  [[nodiscard]] static constexpr EventResult unhandled() {
    return {};
  }

  [[nodiscard]] static constexpr EventResult consumed_event() {
    return {.consumed = true};
  }

  [[nodiscard]] static constexpr EventResult cancelled_event() {
    return {.consumed = true, .cancelled = true};
  }
};

struct ElementEventContext {
  ElementId target_element_id;
};

struct ElementFocusContext {
  ElementId element_id;
};

using ClickHandler = std::function<EventResult(const ElementEventContext&)>;
using PointerButtonHandler =
    std::function<EventResult(const PointerButton&, const ElementEventContext&)>;
using PointerMoveHandler =
    std::function<EventResult(const PointerMoved&, const ElementEventContext&)>;
using KeyHandler =
    std::function<EventResult(const KeyboardKey&, const ElementEventContext&)>;

using AnyElement = std::unique_ptr<Element>;

class Element {
 public:
  virtual ~Element() = default;

  [[nodiscard]] ElementId id() const {
    return id_;
  }

  [[nodiscard]] virtual LayoutOutput layout(LayoutInput input) const {
    const LayoutOutput output{
        .size = constrain_size({}, input.constraints),
    };
    set_layout_bounds(Rect{
        .origin = output.origin,
        .size = output.size,
    });
    return output;
  }

  [[nodiscard]] std::optional<Rect> layout_bounds() const {
    return layout_bounds_;
  }

  [[nodiscard]] virtual ElementId hit_test(Point point) const {
    return layout_bounds_.has_value() && contains(*layout_bounds_, point) ? id_
                                                                         : ElementId{};
  }

  virtual void paint(PaintList& paint_list) const {
    (void)paint_list;
  }

  [[nodiscard]] bool enabled() const {
    return enabled_;
  }

  void set_enabled(bool enabled) {
    enabled_ = enabled;
  }

  [[nodiscard]] virtual bool focusable() const {
    return false;
  }

  virtual void focus(const ElementFocusContext& context) {
    (void)context;
  }

  [[nodiscard]] virtual EventResult handle_event(
      const PlatformEvent& event,
      const ElementEventContext& context) {
    (void)event;
    (void)context;
    return EventResult::unhandled();
  }

  [[nodiscard]] virtual int z_index() const {
    return 0;
  }

  void set_layout_bounds(Rect bounds) const {
    layout_bounds_ = bounds;
  }

  void assign_id(ElementId id) {
    id_ = id;
  }

 private:
  ElementId id_;
  bool enabled_ = true;
  mutable std::optional<Rect> layout_bounds_;
};

class FixedSizeElement : public Element {
 public:
  explicit FixedSizeElement(Size preferred_size)
      : preferred_size_(preferred_size) {}

  [[nodiscard]] Size preferred_size() const {
    return preferred_size_;
  }

  [[nodiscard]] LayoutOutput layout(LayoutInput input) const override {
    const LayoutOutput output{
        .size = constrain_size(preferred_size_, input.constraints),
    };
    set_layout_bounds(Rect{
        .origin = output.origin,
        .size = output.size,
    });
    return output;
  }

 private:
  Size preferred_size_;
};

class VerticalStackElement : public Element {
 public:
  [[nodiscard]] float gap() const {
    return gap_;
  }

  void set_gap(float gap) {
    gap_ = gap;
  }

  void append_child(std::unique_ptr<Element> child) {
    if (child) {
      children_.push_back(std::move(child));
    }
  }

  [[nodiscard]] std::span<const std::unique_ptr<Element>> children() const {
    return children_;
  }

  [[nodiscard]] LayoutOutput layout(LayoutInput input) const override {
    Size content_size;
    std::size_t child_index = 0;
    for (const auto& child : children_) {
      if (child_index > 0) {
        content_size.height += gap_;
      }
      const LayoutOutput child_output = child->layout(LayoutInput{});
      child->set_layout_bounds(Rect{
          .origin = {.x = 0.0F, .y = content_size.height},
          .size = child_output.size,
      });
      content_size.width = std::max(content_size.width, child_output.size.width);
      content_size.height += child_output.size.height;
      child_index += 1;
    }

    const LayoutOutput output{
        .size = constrain_size(content_size, input.constraints),
    };
    set_layout_bounds(Rect{
        .origin = output.origin,
        .size = output.size,
    });
    return output;
  }

  [[nodiscard]] ElementId hit_test(Point point) const override {
    const std::optional<Rect> bounds = layout_bounds();
    if (!bounds.has_value() || !contains(*bounds, point)) {
      return {};
    }

    for (auto iterator = children_.rbegin(); iterator != children_.rend();
         ++iterator) {
      const ElementId hit = (*iterator)->hit_test(point);
      if (hit.value != 0) {
        return hit;
      }
    }
    return id();
  }

 private:
  float gap_ = 0.0F;
  std::vector<std::unique_ptr<Element>> children_;
};

enum class FlexDirection {
  row,
  column,
};

class FlexElement : public Element {
 public:
  explicit FlexElement(FlexDirection direction) : direction_(direction) {}

  [[nodiscard]] FlexDirection direction() const {
    return direction_;
  }

  [[nodiscard]] float gap() const {
    return gap_;
  }

  void set_gap(float gap) {
    gap_ = gap;
  }

  void append_child(std::unique_ptr<Element> child) {
    if (child) {
      children_.push_back(std::move(child));
    }
  }

  [[nodiscard]] std::span<const std::unique_ptr<Element>> children() const {
    return children_;
  }

  [[nodiscard]] LayoutOutput layout(LayoutInput input) const override {
    Size content_size;
    Point child_origin;
    std::size_t child_index = 0;
    for (const auto& child : children_) {
      if (child_index > 0) {
        if (direction_ == FlexDirection::row) {
          child_origin.x += gap_;
          content_size.width += gap_;
        } else {
          child_origin.y += gap_;
          content_size.height += gap_;
        }
      }
      const LayoutOutput child_output = child->layout(LayoutInput{});
      child->set_layout_bounds(Rect{
          .origin = child_origin,
          .size = child_output.size,
      });

      if (direction_ == FlexDirection::row) {
        child_origin.x += child_output.size.width;
        content_size.width += child_output.size.width;
        content_size.height =
            std::max(content_size.height, child_output.size.height);
      } else {
        child_origin.y += child_output.size.height;
        content_size.width =
            std::max(content_size.width, child_output.size.width);
        content_size.height += child_output.size.height;
      }
      child_index += 1;
    }

    const LayoutOutput output{
        .size = constrain_size(content_size, input.constraints),
    };
    set_layout_bounds(Rect{
        .origin = output.origin,
        .size = output.size,
    });
    return output;
  }

  [[nodiscard]] ElementId hit_test(Point point) const override {
    const std::optional<Rect> bounds = layout_bounds();
    if (!bounds.has_value() || !contains(*bounds, point)) {
      return {};
    }

    for (auto iterator = children_.rbegin(); iterator != children_.rend();
         ++iterator) {
      const ElementId hit = (*iterator)->hit_test(point);
      if (hit.value != 0) {
        return hit;
      }
    }
    return id();
  }

 private:
  FlexDirection direction_;
  float gap_ = 0.0F;
  std::vector<std::unique_ptr<Element>> children_;
};

class StyledElement : public Element {
 public:
  explicit StyledElement(Style style, std::unique_ptr<Element> child = {})
      : style_state_(StyleState{.base = style}),
        child_(std::move(child)) {}

  explicit StyledElement(
      StyleState style_state,
      std::unique_ptr<Element> child = {})
      : style_state_(style_state),
        child_(std::move(child)) {}

  [[nodiscard]] const Style& style() const {
    return style_state_.base;
  }

  [[nodiscard]] const StyleState& style_state() const {
    return style_state_;
  }

  [[nodiscard]] Element* child() {
    return child_.get();
  }

  [[nodiscard]] const Element* child() const {
    return child_.get();
  }

  [[nodiscard]] LayoutOutput layout(LayoutInput input) const override {
    const Style& base_style = style();
    Size content_size = base_style.preferred_size;
    if (child_) {
      const LayoutOutput child_output = child_->layout(input);
      content_size = child_output.size;
      child_->set_layout_bounds(Rect{
          .origin =
              {
                  .x = base_style.margin.left + base_style.padding.left,
                  .y = base_style.margin.top + base_style.padding.top,
              },
          .size = child_output.size,
      });
    }
    const Size preferred{
        .width = content_size.width + base_style.padding.left +
                 base_style.padding.right + base_style.margin.left +
                 base_style.margin.right,
        .height = content_size.height + base_style.padding.top +
                  base_style.padding.bottom + base_style.margin.top +
                  base_style.margin.bottom,
    };
    const LayoutOutput output{
        .size = constrain_size(preferred, input.constraints),
    };
    set_layout_bounds(Rect{
        .origin = output.origin,
        .size = output.size,
    });
    return output;
  }

  void paint(PaintList& paint_list) const override;

  [[nodiscard]] EventResult handle_event(
      const PlatformEvent& event,
      const ElementEventContext& context) override {
    return child_ == nullptr || !child_->enabled()
               ? EventResult::unhandled()
               : child_->handle_event(event, context);
  }

  [[nodiscard]] int z_index() const override {
    return style().z_index;
  }

 private:
  StyleState style_state_;
  std::unique_ptr<Element> child_;
};

class TextElement : public Element {
 public:
  explicit TextElement(TextModel* model) : model_(model) {}

  [[nodiscard]] TextModel* model() const {
    return model_;
  }

  [[nodiscard]] std::string_view text() const {
    return model_ == nullptr ? std::string_view{} : model_->text();
  }

  [[nodiscard]] LayoutOutput layout(LayoutInput input) const override {
    const Size preferred{
        .width = static_cast<float>(text().size()) * glyph_width,
        .height = glyph_height,
    };
    const LayoutOutput output{
        .size = constrain_size(preferred, input.constraints),
    };
    set_layout_bounds(Rect{
        .origin = output.origin,
        .size = output.size,
    });
    return output;
  }

  void paint(PaintList& paint_list) const override;

 private:
  static constexpr float glyph_width = 8.0F;
  static constexpr float glyph_height = 16.0F;

  TextModel* model_ = nullptr;
};

class ChildViewElement : public Element {
 public:
  ChildViewElement(ViewId view_id, Size placeholder_size)
      : view_id_(view_id),
        placeholder_size_(placeholder_size) {}

  [[nodiscard]] ViewId view_id() const {
    return view_id_;
  }

  [[nodiscard]] Size placeholder_size() const {
    return placeholder_size_;
  }

  [[nodiscard]] LayoutOutput layout(LayoutInput input) const override {
    const LayoutOutput output{
        .size = constrain_size(placeholder_size_, input.constraints),
    };
    set_layout_bounds(Rect{
        .origin = output.origin,
        .size = output.size,
    });
    return output;
  }

 private:
  ViewId view_id_;
  Size placeholder_size_;
};

class ClickElement : public Element {
 public:
  ClickElement(std::unique_ptr<Element> child, ClickHandler handler)
      : child_(std::move(child)),
        handler_(std::move(handler)) {}

  [[nodiscard]] Element* child() {
    return child_.get();
  }

  [[nodiscard]] const Element* child() const {
    return child_.get();
  }

  [[nodiscard]] LayoutOutput layout(LayoutInput input) const override {
    if (child_) {
      const LayoutOutput output = child_->layout(input);
      child_->set_layout_bounds(Rect{
          .origin = output.origin,
          .size = output.size,
      });
      set_layout_bounds(Rect{
          .origin = output.origin,
          .size = output.size,
      });
      return output;
    }
    return Element::layout(input);
  }

  [[nodiscard]] ElementId hit_test(Point point) const override {
    const ElementId child_hit = child_ ? child_->hit_test(point) : ElementId{};
    return child_hit.value != 0 ? child_hit : Element::hit_test(point);
  }

  void paint(PaintList& paint_list) const override {
    if (child_) {
      child_->paint(paint_list);
    }
  }

  [[nodiscard]] EventResult handle_event(
      const PlatformEvent& event,
      const ElementEventContext& context) override {
    if (!enabled()) {
      return EventResult::unhandled();
    }
    if (const auto* pointer_button = std::get_if<PointerButton>(&event);
        pointer_button != nullptr && pointer_button->pressed && handler_) {
      return handler_(context);
    }
    return child_ == nullptr || !child_->enabled()
               ? EventResult::unhandled()
               : child_->handle_event(event, context);
  }

 private:
  std::unique_ptr<Element> child_;
  ClickHandler handler_;
};

class PointerElement : public Element {
 public:
  PointerElement(
      std::unique_ptr<Element> child,
      PointerButtonHandler down_handler,
      PointerButtonHandler up_handler,
      PointerMoveHandler move_handler)
      : child_(std::move(child)),
        down_handler_(std::move(down_handler)),
        up_handler_(std::move(up_handler)),
        move_handler_(std::move(move_handler)) {}

  [[nodiscard]] Element* child() {
    return child_.get();
  }

  [[nodiscard]] const Element* child() const {
    return child_.get();
  }

  [[nodiscard]] LayoutOutput layout(LayoutInput input) const override {
    if (child_) {
      const LayoutOutput output = child_->layout(input);
      child_->set_layout_bounds(Rect{
          .origin = output.origin,
          .size = output.size,
      });
      set_layout_bounds(Rect{
          .origin = output.origin,
          .size = output.size,
      });
      return output;
    }
    return Element::layout(input);
  }

  [[nodiscard]] ElementId hit_test(Point point) const override {
    const ElementId child_hit = child_ ? child_->hit_test(point) : ElementId{};
    return child_hit.value != 0 ? child_hit : Element::hit_test(point);
  }

  void paint(PaintList& paint_list) const override {
    if (child_) {
      child_->paint(paint_list);
    }
  }

  [[nodiscard]] EventResult handle_event(
      const PlatformEvent& event,
      const ElementEventContext& context) override {
    if (!enabled()) {
      return EventResult::unhandled();
    }

    if (const auto* pointer_button = std::get_if<PointerButton>(&event);
        pointer_button != nullptr) {
      PointerButtonHandler& handler =
          pointer_button->pressed ? down_handler_ : up_handler_;
      if (handler) {
        const EventResult result = handler(*pointer_button, context);
        if (result.consumed || result.cancelled) {
          return result;
        }
      }
    } else if (const auto* pointer_move = std::get_if<PointerMoved>(&event);
               pointer_move != nullptr && move_handler_) {
      const EventResult result = move_handler_(*pointer_move, context);
      if (result.consumed || result.cancelled) {
        return result;
      }
    }

    return child_ == nullptr || !child_->enabled()
               ? EventResult::unhandled()
               : child_->handle_event(event, context);
  }

 private:
  std::unique_ptr<Element> child_;
  PointerButtonHandler down_handler_;
  PointerButtonHandler up_handler_;
  PointerMoveHandler move_handler_;
};

class FocusableElement : public Element {
 public:
  explicit FocusableElement(std::unique_ptr<Element> child)
      : child_(std::move(child)) {}

  [[nodiscard]] Element* child() {
    return child_.get();
  }

  [[nodiscard]] const Element* child() const {
    return child_.get();
  }

  [[nodiscard]] bool focusable() const override {
    return true;
  }

  [[nodiscard]] LayoutOutput layout(LayoutInput input) const override {
    if (child_) {
      const LayoutOutput output = child_->layout(input);
      child_->set_layout_bounds(Rect{
          .origin = output.origin,
          .size = output.size,
      });
      set_layout_bounds(Rect{
          .origin = output.origin,
          .size = output.size,
      });
      return output;
    }
    return Element::layout(input);
  }

  [[nodiscard]] ElementId hit_test(Point point) const override {
    const ElementId child_hit = child_ ? child_->hit_test(point) : ElementId{};
    return child_hit.value != 0 ? child_hit : Element::hit_test(point);
  }

  void paint(PaintList& paint_list) const override {
    if (child_) {
      child_->paint(paint_list);
    }
  }

  [[nodiscard]] EventResult handle_event(
      const PlatformEvent& event,
      const ElementEventContext& context) override {
    return !enabled() || child_ == nullptr || !child_->enabled()
               ? EventResult::unhandled()
               : child_->handle_event(event, context);
  }

 private:
  std::unique_ptr<Element> child_;
};

class KeyElement : public Element {
 public:
  KeyElement(std::unique_ptr<Element> child, KeyHandler handler)
      : child_(std::move(child)),
        handler_(std::move(handler)) {}

  [[nodiscard]] Element* child() {
    return child_.get();
  }

  [[nodiscard]] const Element* child() const {
    return child_.get();
  }

  [[nodiscard]] LayoutOutput layout(LayoutInput input) const override {
    if (child_) {
      const LayoutOutput output = child_->layout(input);
      child_->set_layout_bounds(Rect{
          .origin = output.origin,
          .size = output.size,
      });
      set_layout_bounds(Rect{
          .origin = output.origin,
          .size = output.size,
      });
      return output;
    }
    return Element::layout(input);
  }

  [[nodiscard]] ElementId hit_test(Point point) const override {
    const ElementId child_hit = child_ ? child_->hit_test(point) : ElementId{};
    return child_hit.value != 0 ? child_hit : Element::hit_test(point);
  }

  void paint(PaintList& paint_list) const override {
    if (child_) {
      child_->paint(paint_list);
    }
  }

  [[nodiscard]] EventResult handle_event(
      const PlatformEvent& event,
      const ElementEventContext& context) override {
    if (!enabled()) {
      return EventResult::unhandled();
    }
    if (const auto* key = std::get_if<KeyboardKey>(&event);
        key != nullptr && handler_) {
      return handler_(*key, context);
    }
    return child_ == nullptr || !child_->enabled()
               ? EventResult::unhandled()
               : child_->handle_event(event, context);
  }

 private:
  std::unique_ptr<Element> child_;
  KeyHandler handler_;
};

class ElementBuilder {
 public:
  [[nodiscard]] static ElementBuilder box() {
    return ElementBuilder(Kind::box);
  }

  [[nodiscard]] static ElementBuilder row() {
    return ElementBuilder(Kind::row);
  }

  [[nodiscard]] static ElementBuilder column() {
    return ElementBuilder(Kind::column);
  }

  [[nodiscard]] static ElementBuilder v_stack() {
    return ElementBuilder(Kind::v_stack);
  }

  [[nodiscard]] static ElementBuilder fixed_size(Size size) {
    ElementBuilder builder(Kind::fixed_size);
    builder.size_ = size;
    return builder;
  }

  [[nodiscard]] static ElementBuilder text(TextModel& model) {
    ElementBuilder builder(Kind::text);
    builder.text_model_ = &model;
    return builder;
  }

  [[nodiscard]] static ElementBuilder child_view(ViewId view_id) {
    ElementBuilder builder(Kind::child_view);
    builder.child_view_id_ = view_id;
    return builder;
  }

  [[nodiscard]] ElementBuilder style(Style style) && {
    style_state_.base = style;
    return std::move(*this);
  }

  [[nodiscard]] ElementBuilder hover_style(StyleOverlay overlay) && {
    style_state_.hover = overlay;
    return std::move(*this);
  }

  [[nodiscard]] ElementBuilder focus_style(StyleOverlay overlay) && {
    style_state_.focus = overlay;
    return std::move(*this);
  }

  [[nodiscard]] ElementBuilder disabled_style(StyleOverlay overlay) && {
    style_state_.disabled = overlay;
    return std::move(*this);
  }

  [[nodiscard]] ElementBuilder size(Size size) && {
    style_state_.base = style_state_.base.with_preferred_size(size);
    if (kind_ == Kind::fixed_size || kind_ == Kind::child_view) {
      size_ = size;
    }
    return std::move(*this);
  }

  [[nodiscard]] ElementBuilder size(float width, float height) && {
    return std::move(*this).size(Size{.width = width, .height = height});
  }

  [[nodiscard]] ElementBuilder padding(EdgeSizes edges) && {
    style_state_.base = style_state_.base.with_padding(edges);
    return std::move(*this);
  }

  [[nodiscard]] ElementBuilder margin(EdgeSizes edges) && {
    style_state_.base = style_state_.base.with_margin(edges);
    return std::move(*this);
  }

  [[nodiscard]] ElementBuilder background(Color color) && {
    style_state_.base = style_state_.base.with_background_color(color);
    return std::move(*this);
  }

  [[nodiscard]] ElementBuilder foreground(Color color) && {
    style_state_.base = style_state_.base.with_foreground_color(color);
    return std::move(*this);
  }

  [[nodiscard]] ElementBuilder border_width(EdgeSizes edges) && {
    style_state_.base = style_state_.base.with_border_width(edges);
    return std::move(*this);
  }

  [[nodiscard]] ElementBuilder border_color(Color color) && {
    style_state_.base = style_state_.base.with_border_color(color);
    return std::move(*this);
  }

  [[nodiscard]] ElementBuilder border_radius(BorderRadii radius) && {
    style_state_.base = style_state_.base.with_border_radius(radius);
    return std::move(*this);
  }

  [[nodiscard]] ElementBuilder gap(float value) && {
    style_state_.base = style_state_.base.with_gap(value);
    return std::move(*this);
  }

  [[nodiscard]] ElementBuilder enabled(bool value) && {
    enabled_ = value;
    return std::move(*this);
  }

  [[nodiscard]] ElementBuilder disabled() && {
    enabled_ = false;
    return std::move(*this);
  }

  [[nodiscard]] ElementBuilder focusable() && {
    focusable_ = true;
    return std::move(*this);
  }

  [[nodiscard]] ElementBuilder on_click(ClickHandler handler) && {
    click_handler_ = std::move(handler);
    return std::move(*this);
  }

  [[nodiscard]] ElementBuilder on_pointer_down(
      PointerButtonHandler handler) && {
    pointer_down_handler_ = std::move(handler);
    return std::move(*this);
  }

  [[nodiscard]] ElementBuilder on_pointer_up(PointerButtonHandler handler) && {
    pointer_up_handler_ = std::move(handler);
    return std::move(*this);
  }

  [[nodiscard]] ElementBuilder on_pointer_move(PointerMoveHandler handler) && {
    pointer_move_handler_ = std::move(handler);
    return std::move(*this);
  }

  [[nodiscard]] ElementBuilder on_key(KeyHandler handler) && {
    key_handler_ = std::move(handler);
    return std::move(*this);
  }

  [[nodiscard]] ElementBuilder child(std::unique_ptr<Element> child) && {
    if (child) {
      children_.push_back(std::move(child));
    }
    return std::move(*this);
  }

  [[nodiscard]] ElementBuilder child(ElementBuilder child) &&;

  template <typename T>
    requires std::derived_from<T, Element> && (!std::same_as<T, Element>)
  [[nodiscard]] ElementBuilder child(std::unique_ptr<T> child) && {
    return std::move(*this).child(std::unique_ptr<Element>(std::move(child)));
  }

  [[nodiscard]] std::unique_ptr<Element> build() && {
    if (kind_ == Kind::fixed_size) {
      return finish(std::make_unique<FixedSizeElement>(size_));
    }
    if (kind_ == Kind::text) {
      return finish(std::make_unique<TextElement>(text_model_));
    }
    if (kind_ == Kind::child_view) {
      return finish(std::make_unique<ChildViewElement>(child_view_id_, size_));
    }
    if (kind_ == Kind::v_stack) {
      auto element = std::make_unique<VerticalStackElement>();
      element->set_gap(style_state_.base.gap);
      for (auto& child : children_) {
        element->append_child(std::move(child));
      }
      return finish(std::move(element));
    }
    if (kind_ == Kind::row || kind_ == Kind::column) {
      auto element = std::make_unique<FlexElement>(
          kind_ == Kind::row ? FlexDirection::row : FlexDirection::column);
      element->set_gap(style_state_.base.gap);
      for (auto& child : children_) {
        element->append_child(std::move(child));
      }
      return finish(std::move(element));
    }
    std::unique_ptr<Element> child;
    if (!children_.empty()) {
      child = std::move(children_.front());
    }
    return finish(
        std::make_unique<StyledElement>(style_state_, std::move(child)));
  }

 private:
  enum class Kind {
    box,
    row,
    column,
    v_stack,
    fixed_size,
    text,
    child_view,
  };

  explicit ElementBuilder(Kind kind) : kind_(kind) {}

  [[nodiscard]] std::unique_ptr<Element> finish(
      std::unique_ptr<Element> element) const {
    element->set_enabled(enabled_);
    if (click_handler_) {
      auto click_element =
          std::make_unique<ClickElement>(std::move(element), click_handler_);
      click_element->set_enabled(enabled_);
      element = std::move(click_element);
    }
    if (pointer_down_handler_ || pointer_up_handler_ || pointer_move_handler_) {
      auto pointer_element = std::make_unique<PointerElement>(
          std::move(element),
          pointer_down_handler_,
          pointer_up_handler_,
          pointer_move_handler_);
      pointer_element->set_enabled(enabled_);
      element = std::move(pointer_element);
    }
    if (key_handler_) {
      auto key_element =
          std::make_unique<KeyElement>(std::move(element), key_handler_);
      key_element->set_enabled(enabled_);
      element = std::move(key_element);
    }
    if (focusable_) {
      auto focusable_element =
          std::make_unique<FocusableElement>(std::move(element));
      focusable_element->set_enabled(enabled_);
      element = std::move(focusable_element);
    }
    return element;
  }

  Kind kind_ = Kind::box;
  StyleState style_state_;
  Size size_;
  TextModel* text_model_ = nullptr;
  ViewId child_view_id_;
  bool enabled_ = true;
  bool focusable_ = false;
  ClickHandler click_handler_;
  PointerButtonHandler pointer_down_handler_;
  PointerButtonHandler pointer_up_handler_;
  PointerMoveHandler pointer_move_handler_;
  KeyHandler key_handler_;
  std::vector<std::unique_ptr<Element>> children_;
};

[[nodiscard]] inline AnyElement into_element(AnyElement element) {
  return element;
}

[[nodiscard]] inline AnyElement into_element(ElementBuilder builder) {
  return std::move(builder).build();
}

[[nodiscard]] inline ElementBuilder ElementBuilder::child(
    ElementBuilder child) && {
  return std::move(*this).child(into_element(std::move(child)));
}

[[nodiscard]] inline ElementBuilder div() {
  return ElementBuilder::box();
}

[[nodiscard]] inline ElementBuilder h_flex() {
  return ElementBuilder::row();
}

[[nodiscard]] inline ElementBuilder v_flex() {
  return ElementBuilder::column();
}

[[nodiscard]] inline ElementBuilder v_stack() {
  return ElementBuilder::v_stack();
}

[[nodiscard]] inline ElementBuilder text(TextModel& model) {
  return ElementBuilder::text(model);
}

[[nodiscard]] inline ElementBuilder child_view(ViewId view_id) {
  return ElementBuilder::child_view(view_id);
}

class ElementTree {
 public:
  [[nodiscard]] ElementId set_root(std::unique_ptr<Element> element) {
    nodes_.clear();
    root_id_ = {};
    if (!element) {
      return {};
    }

    const ElementId id = allocate_id();
    element->assign_id(id);
    nodes_.push_back(Node{
        .element = std::move(element),
        .id = id,
    });
    root_id_ = id;
    return id;
  }

  [[nodiscard]] ElementId append_child(
      ElementId parent,
      std::unique_ptr<Element> element) {
    Node* parent_node = find_node(parent);
    if (parent_node == nullptr || !element) {
      return {};
    }

    const ElementId id = allocate_id();
    element->assign_id(id);
    nodes_.push_back(Node{
        .element = std::move(element),
        .id = id,
        .parent = parent,
    });
    parent_node = find_node(parent);
    parent_node->children.push_back(id);
    return id;
  }

  [[nodiscard]] ElementId reconcile_root(std::unique_ptr<Element> element) {
    if (!element) {
      return {};
    }
    if (root_id_.value == 0) {
      return set_root(std::move(element));
    }

    Node* root_node = find_node(root_id_);
    if (root_node == nullptr) {
      return set_root(std::move(element));
    }

    element->assign_id(root_id_);
    root_node->element = std::move(element);
    return root_id_;
  }

  [[nodiscard]] ElementId reconcile_child(
      ElementId parent,
      std::size_t index,
      std::unique_ptr<Element> element) {
    Node* parent_node = find_node(parent);
    if (parent_node == nullptr || !element) {
      return {};
    }

    if (index == parent_node->children.size()) {
      return append_child(parent, std::move(element));
    }
    if (index > parent_node->children.size()) {
      return {};
    }

    const ElementId child_id = parent_node->children[index];
    Node* child_node = find_node(child_id);
    if (child_node == nullptr) {
      return {};
    }

    element->assign_id(child_id);
    child_node->element = std::move(element);
    child_node->parent = parent;
    return child_id;
  }

  [[nodiscard]] ElementId root_id() const {
    return root_id_;
  }

  [[nodiscard]] Element* get(ElementId id) {
    Node* node = find_node(id);
    return node == nullptr ? nullptr : node->element.get();
  }

  [[nodiscard]] const Element* get(ElementId id) const {
    const Node* node = find_node(id);
    return node == nullptr ? nullptr : node->element.get();
  }

  template <typename T>
  [[nodiscard]] T* find_as(ElementId id) {
    return dynamic_cast<T*>(get(id));
  }

  template <typename T>
  [[nodiscard]] const T* find_as(ElementId id) const {
    return dynamic_cast<const T*>(get(id));
  }

  [[nodiscard]] std::optional<ElementId> parent(ElementId id) const {
    const Node* node = find_node(id);
    return node == nullptr ? std::optional<ElementId>{} : node->parent;
  }

  [[nodiscard]] std::span<const ElementId> children(ElementId id) const {
    const Node* node = find_node(id);
    if (node == nullptr) {
      return {};
    }
    return node->children;
  }

  [[nodiscard]] LayoutOutput layout_root(LayoutInput input) const {
    const Element* root = get(root_id_);
    if (root == nullptr) {
      return LayoutOutput{
          .size = constrain_size({}, input.constraints),
      };
    }
    return root->layout(input);
  }

  [[nodiscard]] ElementId hit_test_root(Point point) const {
    const Element* root = get(root_id_);
    return root == nullptr ? ElementId{} : root->hit_test(point);
  }

  [[nodiscard]] std::vector<ElementId> preorder_ids() const {
    std::vector<ElementId> ids;
    append_preorder_ids(root_id_, ids);
    return ids;
  }

  [[nodiscard]] std::vector<ElementId> enabled_preorder_ids() const {
    std::vector<ElementId> ids;
    append_enabled_preorder_ids(root_id_, ids);
    return ids;
  }

  void paint(PaintList& paint_list) const {
    paint_subtree(root_id_, paint_list);
  }

 private:
  struct Node {
    std::unique_ptr<Element> element;
    ElementId id;
    std::optional<ElementId> parent;
    std::vector<ElementId> children;
  };

  [[nodiscard]] ElementId allocate_id() {
    const ElementId id{next_id_};
    next_id_ += 1;
    return id;
  }

  [[nodiscard]] Node* find_node(ElementId id) {
    for (Node& node : nodes_) {
      if (node.id == id) {
        return &node;
      }
    }
    return nullptr;
  }

  [[nodiscard]] const Node* find_node(ElementId id) const {
    for (const Node& node : nodes_) {
      if (node.id == id) {
        return &node;
      }
    }
    return nullptr;
  }

  void append_preorder_ids(ElementId id, std::vector<ElementId>& ids) const {
    const Node* node = find_node(id);
    if (node == nullptr) {
      return;
    }

    ids.push_back(id);
    for (ElementId child_id : node->children) {
      append_preorder_ids(child_id, ids);
    }
  }

  void append_enabled_preorder_ids(
      ElementId id,
      std::vector<ElementId>& ids) const {
    const Node* node = find_node(id);
    if (node == nullptr) {
      return;
    }

    if (node->element->enabled()) {
      ids.push_back(id);
    }
    for (ElementId child_id : node->children) {
      append_enabled_preorder_ids(child_id, ids);
    }
  }

  void paint_subtree(ElementId id, PaintList& paint_list) const {
    const Node* node = find_node(id);
    if (node == nullptr) {
      return;
    }

    node->element->paint(paint_list);
    std::vector<ElementId> ordered_children(node->children.begin(),
                                            node->children.end());
    std::stable_sort(
        ordered_children.begin(),
        ordered_children.end(),
        [this](ElementId lhs, ElementId rhs) {
          const Node* lhs_node = find_node(lhs);
          const Node* rhs_node = find_node(rhs);
          const int lhs_z =
              lhs_node == nullptr ? 0 : lhs_node->element->z_index();
          const int rhs_z =
              rhs_node == nullptr ? 0 : rhs_node->element->z_index();
          return lhs_z < rhs_z;
        });
    for (ElementId child_id : ordered_children) {
      paint_subtree(child_id, paint_list);
    }
  }

  std::vector<Node> nodes_;
  ElementId root_id_;
  std::uint64_t next_id_ = 1;
};

} // namespace cgpui
