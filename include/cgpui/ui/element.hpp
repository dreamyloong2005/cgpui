#pragma once

#include "cgpui/core/events.hpp"
#include "cgpui/ui/layout.hpp"
#include "cgpui/ui/style.hpp"
#include "cgpui/ui/text.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace cgpui {

class PaintList;

struct ElementId {
  std::uint64_t value = 0;

  friend bool operator==(ElementId, ElementId) = default;
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
      : style_(style),
        child_(std::move(child)) {}

  [[nodiscard]] const Style& style() const {
    return style_;
  }

  [[nodiscard]] Element* child() {
    return child_.get();
  }

  [[nodiscard]] const Element* child() const {
    return child_.get();
  }

  [[nodiscard]] LayoutOutput layout(LayoutInput input) const override {
    Size content_size = style_.preferred_size;
    if (child_) {
      const LayoutOutput child_output = child_->layout(input);
      content_size = child_output.size;
      child_->set_layout_bounds(Rect{
          .origin =
              {
                  .x = style_.margin.left + style_.padding.left,
                  .y = style_.margin.top + style_.padding.top,
              },
          .size = child_output.size,
      });
    }
    const Size preferred{
        .width = content_size.width + style_.padding.left +
                 style_.padding.right + style_.margin.left +
                 style_.margin.right,
        .height = content_size.height + style_.padding.top +
                  style_.padding.bottom + style_.margin.top +
                  style_.margin.bottom,
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
    return style_.z_index;
  }

 private:
  Style style_;
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

  [[nodiscard]] ElementBuilder style(Style style) && {
    style_ = style;
    return std::move(*this);
  }

  [[nodiscard]] ElementBuilder enabled(bool value) && {
    enabled_ = value;
    return std::move(*this);
  }

  [[nodiscard]] ElementBuilder child(std::unique_ptr<Element> child) && {
    if (child) {
      children_.push_back(std::move(child));
    }
    return std::move(*this);
  }

  [[nodiscard]] std::unique_ptr<Element> build() && {
    if (kind_ == Kind::fixed_size) {
      return finish(std::make_unique<FixedSizeElement>(size_));
    }
    if (kind_ == Kind::text) {
      return finish(std::make_unique<TextElement>(text_model_));
    }
    if (kind_ == Kind::v_stack) {
      auto element = std::make_unique<VerticalStackElement>();
      element->set_gap(style_.gap);
      for (auto& child : children_) {
        element->append_child(std::move(child));
      }
      return finish(std::move(element));
    }
    if (kind_ == Kind::row || kind_ == Kind::column) {
      auto element = std::make_unique<FlexElement>(
          kind_ == Kind::row ? FlexDirection::row : FlexDirection::column);
      element->set_gap(style_.gap);
      for (auto& child : children_) {
        element->append_child(std::move(child));
      }
      return finish(std::move(element));
    }
    std::unique_ptr<Element> child;
    if (!children_.empty()) {
      child = std::move(children_.front());
    }
    return finish(std::make_unique<StyledElement>(style_, std::move(child)));
  }

 private:
  enum class Kind {
    box,
    row,
    column,
    v_stack,
    fixed_size,
    text,
  };

  explicit ElementBuilder(Kind kind) : kind_(kind) {}

  [[nodiscard]] std::unique_ptr<Element> finish(
      std::unique_ptr<Element> element) const {
    element->set_enabled(enabled_);
    return element;
  }

  Kind kind_ = Kind::box;
  Style style_;
  Size size_;
  TextModel* text_model_ = nullptr;
  bool enabled_ = true;
  std::vector<std::unique_ptr<Element>> children_;
};

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
