#pragma once

#include "cgpui/core/events.hpp"
#include "cgpui/ui/layout.hpp"
#include "cgpui/ui/scroll.hpp"
#include "cgpui/ui/style.hpp"
#include "cgpui/ui/text.hpp"

#include <any>
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>
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

struct ElementKey {
  std::string value;

  friend bool operator==(const ElementKey&, const ElementKey&) = default;
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
  std::function<EventResult(std::string_view)> dispatch_action;
};

struct ElementFocusContext {
  ElementId element_id;
};

struct ElementLifecycleContext {
  ElementId element_id;
  std::optional<ElementId> parent_element_id;
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

  [[nodiscard]] const std::optional<ElementKey>& key() const {
    return key_;
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

  [[nodiscard]] float flex_grow() const {
    return flex_grow_;
  }

  void set_flex_grow(float value) {
    flex_grow_ = value;
  }

  [[nodiscard]] float flex_shrink() const {
    return flex_shrink_;
  }

  void set_flex_shrink(float value) {
    flex_shrink_ = value;
  }

  [[nodiscard]] Position position() const {
    return position_;
  }

  void set_position(Position value) {
    position_ = value;
  }

  [[nodiscard]] EdgeSizes inset() const {
    return inset_;
  }

  void set_inset(EdgeSizes value) {
    inset_ = value;
  }

  [[nodiscard]] virtual int z_index() const {
    return z_index_;
  }

  void set_z_index(int value) {
    z_index_ = value;
  }

  [[nodiscard]] virtual int layer() const {
    return layer_;
  }

  void set_layer(int value) {
    layer_ = value;
  }

  [[nodiscard]] int z_order() const {
    const int explicit_z_index = z_index();
    return explicit_z_index != 0 ? explicit_z_index : layer();
  }

  [[nodiscard]] virtual bool focusable() const {
    return false;
  }

  virtual void focus(const ElementFocusContext& context) {
    (void)context;
  }

  virtual void on_mount(const ElementLifecycleContext& context) {
    (void)context;
  }

  virtual void on_update(const ElementLifecycleContext& context) {
    (void)context;
  }

  virtual void on_unmount(const ElementLifecycleContext& context) {
    (void)context;
  }

  [[nodiscard]] virtual EventResult handle_event(
      const PlatformEvent& event,
      const ElementEventContext& context) {
    (void)event;
    (void)context;
    return EventResult::unhandled();
  }

  void set_layout_bounds(Rect bounds) const {
    layout_bounds_ = bounds;
  }

  void assign_id(ElementId id) {
    id_ = id;
  }

  void set_key(std::optional<ElementKey> key) {
    key_ = std::move(key);
  }

 private:
  ElementId id_;
  std::optional<ElementKey> key_;
  bool enabled_ = true;
  float flex_grow_ = 0.0F;
  float flex_shrink_ = 0.0F;
  Position position_ = Position::relative;
  EdgeSizes inset_;
  int z_index_ = 0;
  int layer_ = 0;
  mutable std::optional<Rect> layout_bounds_;
};

[[nodiscard]] inline Point absolute_origin(Size parent_size, EdgeSizes inset) {
  (void)parent_size;
  return Point{.x = inset.left, .y = inset.top};
}

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
    std::size_t relative_child_index = 0;
    for (const auto& child : children_) {
      const LayoutOutput child_output = child->layout(LayoutInput{});
      if (child->position() == Position::absolute) {
        continue;
      }
      if (relative_child_index > 0) {
        content_size.height += gap_;
      }
      child->set_layout_bounds(Rect{
          .origin = {.x = 0.0F, .y = content_size.height},
          .size = child_output.size,
      });
      content_size.width = std::max(content_size.width, child_output.size.width);
      content_size.height += child_output.size.height;
      relative_child_index += 1;
    }

    const LayoutOutput output{
        .size = constrain_size(content_size, input.constraints),
    };
    set_layout_bounds(Rect{
        .origin = output.origin,
        .size = output.size,
    });
    for (const auto& child : children_) {
      if (child->position() != Position::absolute) {
        continue;
      }
      const LayoutOutput child_output = child->layout(LayoutInput{});
      child->set_layout_bounds(Rect{
          .origin = absolute_origin(output.size, child->inset()),
          .size = child_output.size,
      });
    }
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

  [[nodiscard]] AlignItems align_items() const {
    return align_items_;
  }

  void set_align_items(AlignItems align_items) {
    align_items_ = align_items;
  }

  [[nodiscard]] JustifyContent justify_content() const {
    return justify_content_;
  }

  void set_justify_content(JustifyContent justify_content) {
    justify_content_ = justify_content;
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
    std::vector<Size> child_sizes;
    child_sizes.reserve(children_.size());
    Size content_size;
    float total_flex_grow = 0.0F;
    float total_flex_shrink = 0.0F;
    std::vector<std::size_t> relative_indices;
    relative_indices.reserve(children_.size());
    std::size_t relative_child_index = 0;
    for (std::size_t child_index = 0; child_index < children_.size();
         ++child_index) {
      const auto& child = children_[child_index];
      const LayoutOutput child_output = child->layout(LayoutInput{});
      child_sizes.push_back(child_output.size);
      if (child->position() == Position::absolute) {
        continue;
      }
      relative_indices.push_back(child_index);
      if (relative_child_index > 0) {
        if (direction_ == FlexDirection::row) {
          content_size.width += gap_;
        } else {
          content_size.height += gap_;
        }
      }
      total_flex_grow += std::max(0.0F, child->flex_grow());
      total_flex_shrink += std::max(0.0F, child->flex_shrink());

      if (direction_ == FlexDirection::row) {
        content_size.width += child_output.size.width;
        content_size.height =
            std::max(content_size.height, child_output.size.height);
      } else {
        content_size.width =
            std::max(content_size.width, child_output.size.width);
        content_size.height += child_output.size.height;
      }
      relative_child_index += 1;
    }

    const LayoutOutput output{
        .size = constrain_size(content_size, input.constraints),
    };
    set_layout_bounds(Rect{
        .origin = output.origin,
        .size = output.size,
    });

    const bool row = direction_ == FlexDirection::row;
    const float output_main = row ? output.size.width : output.size.height;
    const float output_cross = row ? output.size.height : output.size.width;
    const float content_main = row ? content_size.width : content_size.height;
    const float positive_free_space = std::max(0.0F, output_main - content_main);
    const float overflow_space = std::max(0.0F, content_main - output_main);
    for (std::size_t index = 0; index < child_sizes.size(); ++index) {
      if (children_[index]->position() == Position::absolute) {
        continue;
      }
      Size& child_size = child_sizes[index];
      float child_main = row ? child_size.width : child_size.height;
      if (positive_free_space > 0.0F && total_flex_grow > 0.0F) {
        child_main +=
            positive_free_space *
            (std::max(0.0F, children_[index]->flex_grow()) / total_flex_grow);
      } else if (overflow_space > 0.0F && total_flex_shrink > 0.0F) {
        child_main -=
            overflow_space *
            (std::max(0.0F, children_[index]->flex_shrink()) /
             total_flex_shrink);
        child_main = std::max(0.0F, child_main);
      }
      if (row) {
        child_size.width = child_main;
      } else {
        child_size.height = child_main;
      }
    }

    float laid_out_main = 0.0F;
    for (std::size_t relative_index = 0; relative_index < relative_indices.size();
         ++relative_index) {
      const std::size_t index = relative_indices[relative_index];
      laid_out_main += row ? child_sizes[index].width : child_sizes[index].height;
      if (relative_index + 1 < relative_indices.size()) {
        laid_out_main += gap_;
      }
    }
    const float free_space = std::max(0.0F, output_main - laid_out_main);
    float main_offset = 0.0F;
    float extra_gap = 0.0F;
    if (justify_content_ == JustifyContent::center) {
      main_offset = free_space / 2.0F;
    } else if (justify_content_ == JustifyContent::end) {
      main_offset = free_space;
    } else if (justify_content_ == JustifyContent::space_between &&
               relative_indices.size() > 1) {
      extra_gap = free_space / static_cast<float>(relative_indices.size() - 1);
    }

    float cursor = main_offset;
    for (std::size_t relative_index = 0; relative_index < relative_indices.size();
         ++relative_index) {
      const std::size_t index = relative_indices[relative_index];
      const Size child_size = child_sizes[index];
      const float child_cross = row ? child_size.height : child_size.width;
      float cross_offset = 0.0F;
      if (align_items_ == AlignItems::center) {
        cross_offset = std::max(0.0F, output_cross - child_cross) / 2.0F;
      } else if (align_items_ == AlignItems::end) {
        cross_offset = std::max(0.0F, output_cross - child_cross);
      }

      children_[index]->set_layout_bounds(Rect{
          .origin =
              row ? Point{.x = cursor, .y = cross_offset}
                  : Point{.x = cross_offset, .y = cursor},
          .size = child_size,
      });

      cursor += row ? child_size.width : child_size.height;
      if (relative_index + 1 < relative_indices.size()) {
        cursor += gap_ + extra_gap;
      }
    }
    for (std::size_t index = 0; index < children_.size(); ++index) {
      if (children_[index]->position() != Position::absolute) {
        continue;
      }
      children_[index]->set_layout_bounds(Rect{
          .origin = absolute_origin(output.size, children_[index]->inset()),
          .size = child_sizes[index],
      });
    }
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
  AlignItems align_items_ = AlignItems::start;
  JustifyContent justify_content_ = JustifyContent::start;
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

  [[nodiscard]] const StyleClasses& style_classes() const {
    return style_classes_;
  }

  [[nodiscard]] const StyleOverlay& inline_style() const {
    return inline_style_;
  }

  void set_style_classes(StyleClasses classes) {
    style_classes_ = std::move(classes);
  }

  void set_inline_style(StyleOverlay style) {
    inline_style_ = std::move(style);
  }

  [[nodiscard]] Style resolved_style(
      const StyleCascade& cascade,
      StyleStateFlags flags) const {
    return cgpui::resolved_style(
        cascade,
        style_state_,
        style_classes_,
        inline_style_,
        flags);
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

  [[nodiscard]] ElementId hit_test(Point point) const override {
    if (style().overflow == Overflow::hidden) {
      const std::optional<Rect> bounds = layout_bounds();
      if (!bounds.has_value()) {
        return {};
      }
      const Rect clip_bounds =
          style().clip_rect.has_value() ? *style().clip_rect : *bounds;
      if (!contains(clip_bounds, point)) {
        return {};
      }
    }

    const ElementId child_hit = child_ ? child_->hit_test(point) : ElementId{};
    return child_hit.value != 0 ? child_hit : Element::hit_test(point);
  }

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

  [[nodiscard]] int layer() const override {
    return style().layer;
  }

 private:
  StyleState style_state_;
  StyleClasses style_classes_;
  StyleOverlay inline_style_;
  std::unique_ptr<Element> child_;
};

class TextElement : public Element {
 public:
  explicit TextElement(TextModel* model, Style style = {})
      : model_(model),
        style_(std::move(style)) {}

  [[nodiscard]] TextModel* model() const {
    return model_;
  }

  [[nodiscard]] std::string_view text() const {
    return model_ == nullptr ? std::string_view{} : model_->text();
  }

  [[nodiscard]] const Style& style() const {
    return style_;
  }

  [[nodiscard]] const FontDescriptor& font() const {
    return style_.font;
  }

  [[nodiscard]] float font_size() const {
    return style_.font_size;
  }

  [[nodiscard]] float glyph_width() const {
    return font_size() * 0.5F;
  }

  [[nodiscard]] LayoutOutput layout(LayoutInput input) const override {
    const Size preferred{
        .width = static_cast<float>(text().size()) * glyph_width(),
        .height = font_size(),
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
  TextModel* model_ = nullptr;
  Style style_;
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

class ButtonElement : public Element {
 public:
  ButtonElement(
      std::string action_name,
      StyleState style_state,
      ClickHandler click_handler = {},
      std::unique_ptr<Element> child = {})
      : action_name_(std::move(action_name)),
        style_state_(style_state),
        click_handler_(std::move(click_handler)),
        child_(std::move(child)) {}

  [[nodiscard]] std::string_view action_name() const {
    return action_name_;
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

  [[nodiscard]] bool focusable() const override {
    return true;
  }

  [[nodiscard]] LayoutOutput layout(LayoutInput input) const override {
    Size content_size = style_state_.base.preferred_size;
    if (child_) {
      const LayoutOutput child_output = child_->layout(input);
      content_size = child_output.size;
      child_->set_layout_bounds(Rect{
          .origin =
              {
                  .x = style_state_.base.margin.left +
                       style_state_.base.padding.left,
                  .y = style_state_.base.margin.top +
                       style_state_.base.padding.top,
              },
          .size = child_output.size,
      });
    }
    const Size preferred{
        .width = content_size.width + style_state_.base.padding.left +
                 style_state_.base.padding.right +
                 style_state_.base.margin.left +
                 style_state_.base.margin.right,
        .height = content_size.height + style_state_.base.padding.top +
                  style_state_.base.padding.bottom +
                  style_state_.base.margin.top +
                  style_state_.base.margin.bottom,
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

  [[nodiscard]] ElementId hit_test(Point point) const override {
    const ElementId child_hit = child_ ? child_->hit_test(point) : ElementId{};
    return child_hit.value != 0 ? child_hit : Element::hit_test(point);
  }

  void paint(PaintList& paint_list) const override;

  [[nodiscard]] EventResult handle_event(
      const PlatformEvent& event,
      const ElementEventContext& context) override {
    if (!enabled()) {
      return EventResult::unhandled();
    }
    const auto* pointer_button = std::get_if<PointerButton>(&event);
    if (pointer_button == nullptr || !pointer_button->pressed ||
        pointer_button->button != MouseButton::left) {
      return child_ == nullptr || !child_->enabled()
                 ? EventResult::unhandled()
                 : child_->handle_event(event, context);
    }

    EventResult result = EventResult::unhandled();
    if (click_handler_) {
      result = click_handler_(context);
      if (result.consumed || result.cancelled) {
        return result;
      }
    }
    if (!action_name_.empty() && context.dispatch_action) {
      result = context.dispatch_action(action_name_);
    }
    return result;
  }

  [[nodiscard]] int z_index() const override {
    return style_state_.base.z_index;
  }

  [[nodiscard]] int layer() const override {
    return style_state_.base.layer;
  }

 private:
  std::string action_name_;
  StyleState style_state_;
  ClickHandler click_handler_;
  std::unique_ptr<Element> child_;
};

class ScrollElement : public Element {
 public:
  ScrollElement(ScrollState& state, std::unique_ptr<Element> child)
      : state_(&state),
        child_(std::move(child)) {}

  [[nodiscard]] ScrollState* state() {
    return state_;
  }

  [[nodiscard]] const ScrollState* state() const {
    return state_;
  }

  [[nodiscard]] Element* child() {
    return child_.get();
  }

  [[nodiscard]] const Element* child() const {
    return child_.get();
  }

  [[nodiscard]] LayoutOutput layout(LayoutInput input) const override {
    if (child_ == nullptr) {
      const LayoutOutput output = Element::layout(input);
      if (state_ != nullptr) {
        state_->set_viewport_size(output.size);
        state_->set_content_size(output.size);
      }
      return output;
    }

    const LayoutOutput child_output = child_->layout(LayoutInput{});
    child_->set_layout_bounds(Rect{
        .origin = child_output.origin,
        .size = child_output.size,
    });
    const LayoutOutput output{
        .size = constrain_size(child_output.size, input.constraints),
    };
    set_layout_bounds(Rect{
        .origin = output.origin,
        .size = output.size,
    });
    if (state_ != nullptr) {
      state_->set_viewport_size(output.size);
      state_->set_content_size(child_output.size);
    }
    return output;
  }

  [[nodiscard]] ElementId hit_test(Point point) const override {
    return Element::hit_test(point);
  }

  void paint(PaintList& paint_list) const override {
    if (child_) {
      child_->paint(paint_list);
    }
  }

  [[nodiscard]] EventResult handle_event(
      const PlatformEvent& event,
      const ElementEventContext& context) override {
    return child_ == nullptr || !child_->enabled()
               ? EventResult::unhandled()
               : child_->handle_event(event, context);
  }

 private:
  ScrollState* state_ = nullptr;
  std::unique_ptr<Element> child_;
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

  [[nodiscard]] ElementBuilder class_name(StyleClassId id) && {
    style_classes_.add(std::move(id));
    return std::move(*this);
  }

  [[nodiscard]] ElementBuilder class_name(std::string_view value) && {
    return std::move(*this).class_name(style_class(value));
  }

  [[nodiscard]] ElementBuilder inline_style(StyleOverlay style) && {
    inline_style_ = std::move(style);
    return std::move(*this);
  }

  [[nodiscard]] ElementBuilder key(ElementKey key) && {
    key_ = std::move(key);
    return std::move(*this);
  }

  [[nodiscard]] ElementBuilder key(std::string_view value) && {
    key_ = ElementKey{.value = std::string(value)};
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

  [[nodiscard]] ElementBuilder font(FontDescriptor descriptor) && {
    style_state_.base = style_state_.base.with_font(std::move(descriptor));
    return std::move(*this);
  }

  [[nodiscard]] ElementBuilder font_size(float value) && {
    style_state_.base = style_state_.base.with_font_size(value);
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

  [[nodiscard]] ElementBuilder align_items(AlignItems value) && {
    style_state_.base = style_state_.base.with_align_items(value);
    return std::move(*this);
  }

  [[nodiscard]] ElementBuilder justify_content(JustifyContent value) && {
    style_state_.base = style_state_.base.with_justify_content(value);
    return std::move(*this);
  }

  [[nodiscard]] ElementBuilder flex_grow(float value) && {
    style_state_.base = style_state_.base.with_flex_grow(value);
    return std::move(*this);
  }

  [[nodiscard]] ElementBuilder flex_shrink(float value) && {
    style_state_.base = style_state_.base.with_flex_shrink(value);
    return std::move(*this);
  }

  [[nodiscard]] ElementBuilder layer(int value) && {
    style_state_.base = style_state_.base.with_layer(value);
    return std::move(*this);
  }

  [[nodiscard]] ElementBuilder position(Position value) && {
    style_state_.base = style_state_.base.with_position(value);
    return std::move(*this);
  }

  [[nodiscard]] ElementBuilder absolute() && {
    return std::move(*this).position(Position::absolute);
  }

  [[nodiscard]] ElementBuilder inset(EdgeSizes edges) && {
    style_state_.base = style_state_.base.with_inset(edges);
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
      return finish(
          std::make_unique<TextElement>(text_model_, style_state_.base));
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
      element->set_align_items(style_state_.base.align_items);
      element->set_justify_content(style_state_.base.justify_content);
      for (auto& child : children_) {
        element->append_child(std::move(child));
      }
      return finish(std::move(element));
    }
    std::unique_ptr<Element> child;
    if (!children_.empty()) {
      child = std::move(children_.front());
    }
    auto element = std::make_unique<StyledElement>(style_state_, std::move(child));
    element->set_style_classes(style_classes_);
    element->set_inline_style(inline_style_);
    return finish(std::move(element));
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

  void apply_element_attributes(Element& element) const {
    element.set_enabled(enabled_);
    element.set_key(key_);
    element.set_flex_grow(style_state_.base.flex_grow);
    element.set_flex_shrink(style_state_.base.flex_shrink);
    element.set_position(style_state_.base.position);
    element.set_inset(style_state_.base.inset);
    element.set_z_index(style_state_.base.z_index);
    element.set_layer(style_state_.base.layer);
  }

  [[nodiscard]] std::unique_ptr<Element> finish(
      std::unique_ptr<Element> element) const {
    apply_element_attributes(*element);
    if (click_handler_) {
      auto click_element =
          std::make_unique<ClickElement>(std::move(element), click_handler_);
      apply_element_attributes(*click_element);
      element = std::move(click_element);
    }
    if (pointer_down_handler_ || pointer_up_handler_ || pointer_move_handler_) {
      auto pointer_element = std::make_unique<PointerElement>(
          std::move(element),
          pointer_down_handler_,
          pointer_up_handler_,
          pointer_move_handler_);
      apply_element_attributes(*pointer_element);
      element = std::move(pointer_element);
    }
    if (key_handler_) {
      auto key_element =
          std::make_unique<KeyElement>(std::move(element), key_handler_);
      apply_element_attributes(*key_element);
      element = std::move(key_element);
    }
    if (focusable_) {
      auto focusable_element =
          std::make_unique<FocusableElement>(std::move(element));
      apply_element_attributes(*focusable_element);
      element = std::move(focusable_element);
    }
    return element;
  }

  Kind kind_ = Kind::box;
  StyleState style_state_;
  StyleClasses style_classes_;
  StyleOverlay inline_style_;
  std::optional<ElementKey> key_;
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

class ButtonBuilder {
 public:
  explicit ButtonBuilder(std::string action_name)
      : action_name_(std::move(action_name)) {}

  [[nodiscard]] ButtonBuilder style(Style style) && {
    style_state_.base = style;
    return std::move(*this);
  }

  [[nodiscard]] ButtonBuilder hover_style(StyleOverlay overlay) && {
    style_state_.hover = overlay;
    return std::move(*this);
  }

  [[nodiscard]] ButtonBuilder focus_style(StyleOverlay overlay) && {
    style_state_.focus = overlay;
    return std::move(*this);
  }

  [[nodiscard]] ButtonBuilder disabled_style(StyleOverlay overlay) && {
    style_state_.disabled = overlay;
    return std::move(*this);
  }

  [[nodiscard]] ButtonBuilder child(std::unique_ptr<Element> child) && {
    child_ = std::move(child);
    return std::move(*this);
  }

  [[nodiscard]] ButtonBuilder child(ElementBuilder child) && {
    return std::move(*this).child(into_element(std::move(child)));
  }

  template <typename T>
    requires std::derived_from<T, Element> && (!std::same_as<T, Element>)
  [[nodiscard]] ButtonBuilder child(std::unique_ptr<T> child) && {
    return std::move(*this).child(std::unique_ptr<Element>(std::move(child)));
  }

  [[nodiscard]] ButtonBuilder on_click(ClickHandler handler) && {
    click_handler_ = std::move(handler);
    return std::move(*this);
  }

  [[nodiscard]] ButtonBuilder enabled(bool value) && {
    enabled_ = value;
    return std::move(*this);
  }

  [[nodiscard]] ButtonBuilder disabled() && {
    enabled_ = false;
    return std::move(*this);
  }

  [[nodiscard]] ButtonBuilder key(ElementKey key) && {
    key_ = std::move(key);
    return std::move(*this);
  }

  [[nodiscard]] ButtonBuilder key(std::string_view value) && {
    key_ = ElementKey{.value = std::string(value)};
    return std::move(*this);
  }

  [[nodiscard]] AnyElement build() && {
    auto element = std::make_unique<ButtonElement>(
        std::move(action_name_),
        style_state_,
        std::move(click_handler_),
        std::move(child_));
    element->set_enabled(enabled_);
    element->set_key(key_);
    element->set_flex_grow(style_state_.base.flex_grow);
    element->set_flex_shrink(style_state_.base.flex_shrink);
    element->set_position(style_state_.base.position);
    element->set_inset(style_state_.base.inset);
    element->set_z_index(style_state_.base.z_index);
    element->set_layer(style_state_.base.layer);
    return element;
  }

 private:
  std::string action_name_;
  StyleState style_state_;
  ClickHandler click_handler_;
  std::unique_ptr<Element> child_;
  std::optional<ElementKey> key_;
  bool enabled_ = true;
};

[[nodiscard]] inline ButtonBuilder button(std::string_view action_name) {
  return ButtonBuilder(std::string(action_name));
}

[[nodiscard]] inline AnyElement scroll(ScrollState& state, AnyElement child) {
  return std::make_unique<ScrollElement>(state, std::move(child));
}

[[nodiscard]] inline AnyElement scroll(ScrollState& state, ElementBuilder child) {
  return scroll(state, into_element(std::move(child)));
}

class ElementTree {
 public:
  [[nodiscard]] ElementId set_root(std::unique_ptr<Element> element) {
    if (root_id_.value != 0) {
      remove_subtree(root_id_);
    }
    nodes_.clear();
    root_id_ = {};
    if (!element) {
      return {};
    }

    const ElementId id = allocate_id();
    element->assign_id(id);
    element->on_mount(ElementLifecycleContext{
        .element_id = id,
        .parent_element_id = std::nullopt,
    });
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
    element->on_mount(ElementLifecycleContext{
        .element_id = id,
        .parent_element_id = parent,
    });
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
    element->on_update(ElementLifecycleContext{
        .element_id = root_id_,
        .parent_element_id = std::nullopt,
    });
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
    element->on_update(ElementLifecycleContext{
        .element_id = child_id,
        .parent_element_id = parent,
    });
    child_node->element = std::move(element);
    child_node->parent = parent;
    return child_id;
  }

  [[nodiscard]] std::vector<ElementId> reconcile_children(
      ElementId parent,
      std::vector<AnyElement> elements) {
    Node* parent_node = find_node(parent);
    if (parent_node == nullptr) {
      return {};
    }

    const std::vector<ElementId> old_children = parent_node->children;
    std::vector<ElementId> new_children;
    std::vector<ElementId> used_children;
    new_children.reserve(elements.size());
    used_children.reserve(elements.size());

    for (std::size_t index = 0; index < elements.size(); ++index) {
      AnyElement& element = elements[index];
      if (!element) {
        continue;
      }

      ElementId child_id;
      if (element->key().has_value()) {
        child_id =
            find_keyed_child(old_children, *element->key(), used_children);
      } else {
        child_id = find_unkeyed_child_at_index(
            old_children,
            index,
            used_children);
      }

      if (child_id.value == 0) {
        child_id = allocate_id();
        element->assign_id(child_id);
        element->on_mount(ElementLifecycleContext{
            .element_id = child_id,
            .parent_element_id = parent,
        });
        nodes_.push_back(Node{
            .element = std::move(element),
            .id = child_id,
            .parent = parent,
        });
      } else {
        Node* child_node = find_node(child_id);
        if (child_node == nullptr) {
          child_id = allocate_id();
          element->assign_id(child_id);
          element->on_mount(ElementLifecycleContext{
              .element_id = child_id,
              .parent_element_id = parent,
          });
          nodes_.push_back(Node{
              .element = std::move(element),
              .id = child_id,
              .parent = parent,
          });
        } else {
          element->assign_id(child_id);
          element->on_update(ElementLifecycleContext{
              .element_id = child_id,
              .parent_element_id = parent,
          });
          child_node->element = std::move(element);
          child_node->parent = parent;
        }
      }

      new_children.push_back(child_id);
      used_children.push_back(child_id);
    }

    parent_node = find_node(parent);
    if (parent_node == nullptr) {
      return {};
    }
    parent_node->children = new_children;

    for (ElementId old_child : old_children) {
      if (!contains_id(new_children, old_child)) {
        remove_subtree(old_child);
      }
    }

    return new_children;
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
  [[nodiscard]] T* state(ElementId id) {
    Node* node = find_node(id);
    if (node == nullptr) {
      return nullptr;
    }
    const auto entry = node->states.find(std::type_index(typeid(T)));
    if (entry == node->states.end()) {
      return nullptr;
    }
    return std::any_cast<T>(&entry->second);
  }

  template <typename T>
  [[nodiscard]] const T* state(ElementId id) const {
    const Node* node = find_node(id);
    if (node == nullptr) {
      return nullptr;
    }
    const auto entry = node->states.find(std::type_index(typeid(T)));
    if (entry == node->states.end()) {
      return nullptr;
    }
    return std::any_cast<T>(&entry->second);
  }

  template <typename T, typename... Args>
  T* emplace_state(ElementId id, Args&&... args) {
    Node* node = find_node(id);
    if (node == nullptr) {
      return nullptr;
    }
    std::any& stored = node->states[std::type_index(typeid(T))];
    stored.emplace<T>(std::forward<Args>(args)...);
    return std::any_cast<T>(&stored);
  }

  template <typename T, typename... Args>
  T* state_or_init(ElementId id, Args&&... args) {
    if (T* existing = state<T>(id); existing != nullptr) {
      return existing;
    }
    return emplace_state<T>(id, std::forward<Args>(args)...);
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
    std::unordered_map<std::type_index, std::any> states;
  };

  [[nodiscard]] ElementId allocate_id() {
    const ElementId id{next_id_};
    next_id_ += 1;
    return id;
  }

  [[nodiscard]] static bool contains_id(
      const std::vector<ElementId>& ids,
      ElementId id) {
    return std::find(ids.begin(), ids.end(), id) != ids.end();
  }

  [[nodiscard]] ElementId find_keyed_child(
      const std::vector<ElementId>& old_children,
      const ElementKey& key,
      const std::vector<ElementId>& used_children) const {
    for (ElementId child_id : old_children) {
      if (contains_id(used_children, child_id)) {
        continue;
      }
      const Node* child_node = find_node(child_id);
      if (child_node == nullptr || !child_node->element->key().has_value()) {
        continue;
      }
      if (*child_node->element->key() == key) {
        return child_id;
      }
    }
    return {};
  }

  [[nodiscard]] ElementId find_unkeyed_child_at_index(
      const std::vector<ElementId>& old_children,
      std::size_t index,
      const std::vector<ElementId>& used_children) const {
    if (index >= old_children.size()) {
      return {};
    }
    const ElementId child_id = old_children[index];
    if (contains_id(used_children, child_id)) {
      return {};
    }
    const Node* child_node = find_node(child_id);
    if (child_node == nullptr || child_node->element->key().has_value()) {
      return {};
    }
    return child_id;
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

  void remove_subtree(ElementId id) {
    const Node* node = find_node(id);
    if (node == nullptr) {
      return;
    }

    const std::vector<ElementId> children = node->children;
    for (ElementId child_id : children) {
      remove_subtree(child_id);
    }
    node = find_node(id);
    if (node != nullptr) {
      node->element->on_unmount(ElementLifecycleContext{
          .element_id = id,
          .parent_element_id = node->parent,
      });
    }
    nodes_.erase(
        std::remove_if(
            nodes_.begin(),
            nodes_.end(),
            [id](const Node& candidate) {
              return candidate.id == id;
            }),
        nodes_.end());
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
              lhs_node == nullptr ? 0 : lhs_node->element->z_order();
          const int rhs_z =
              rhs_node == nullptr ? 0 : rhs_node->element->z_order();
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
