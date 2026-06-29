#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <vector>

namespace cgpui {

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

  void assign_id(ElementId id) {
    id_ = id;
  }

 private:
  ElementId id_;
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
