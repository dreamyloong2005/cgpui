#pragma once

#include "cgpui/ui/layout.hpp"
#include "cgpui/ui/style.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <vector>

namespace cgpui {

class PaintList;

struct ElementId {
  std::uint64_t value = 0;

  friend bool operator==(ElementId, ElementId) = default;
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

  void set_layout_bounds(Rect bounds) const {
    layout_bounds_ = bounds;
  }

  void assign_id(ElementId id) {
    id_ = id;
  }

 private:
  ElementId id_;
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
    for (const auto& child : children_) {
      const LayoutOutput child_output = child->layout(LayoutInput{});
      child->set_layout_bounds(Rect{
          .origin = {.x = 0.0F, .y = content_size.height},
          .size = child_output.size,
      });
      content_size.width = std::max(content_size.width, child_output.size.width);
      content_size.height += child_output.size.height;
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
    const Size preferred =
        child_ ? child_->layout(input).size : style_.preferred_size;
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
  Style style_;
  std::unique_ptr<Element> child_;
};

class ElementBuilder {
 public:
  [[nodiscard]] static ElementBuilder box() {
    return {};
  }

  [[nodiscard]] ElementBuilder style(Style style) && {
    style_ = style;
    return std::move(*this);
  }

  [[nodiscard]] ElementBuilder child(std::unique_ptr<Element> child) && {
    child_ = std::move(child);
    return std::move(*this);
  }

  [[nodiscard]] std::unique_ptr<Element> build() && {
    return std::make_unique<StyledElement>(style_, std::move(child_));
  }

 private:
  Style style_;
  std::unique_ptr<Element> child_;
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

  std::vector<Node> nodes_;
  ElementId root_id_;
  std::uint64_t next_id_ = 1;
};

} // namespace cgpui
