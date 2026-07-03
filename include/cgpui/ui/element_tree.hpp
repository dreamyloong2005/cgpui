#pragma once

#include "cgpui/ui/element_core.hpp"

#include <any>
#include <cstddef>
#include <memory>
#include <optional>
#include <span>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace cgpui {

class ElementTree {
 public:
  [[nodiscard]] ElementId set_root(std::unique_ptr<Element> element);
  [[nodiscard]] ElementId append_child(
      ElementId parent,
      std::unique_ptr<Element> element);
  [[nodiscard]] ElementId reconcile_root(std::unique_ptr<Element> element);
  [[nodiscard]] ElementId reconcile_child(
      ElementId parent,
      std::size_t index,
      std::unique_ptr<Element> element);
  [[nodiscard]] std::vector<ElementId> reconcile_children(
      ElementId parent,
      std::vector<AnyElement> elements);

  [[nodiscard]] ElementId root_id() const;
  [[nodiscard]] Element* get(ElementId id);
  [[nodiscard]] const Element* get(ElementId id) const;

  template <typename T>
  [[nodiscard]] T* state(ElementId id);

  template <typename T>
  [[nodiscard]] const T* state(ElementId id) const;

  template <typename T, typename... Args>
  T* emplace_state(ElementId id, Args&&... args);

  template <typename T, typename... Args>
  T* state_or_init(ElementId id, Args&&... args);

  template <typename T>
  [[nodiscard]] T* find_as(ElementId id);

  template <typename T>
  [[nodiscard]] const T* find_as(ElementId id) const;

  [[nodiscard]] std::optional<ElementId> parent(ElementId id) const;
  [[nodiscard]] std::span<const ElementId> children(ElementId id) const;
  [[nodiscard]] LayoutOutput layout_root(LayoutInput input) const;
  [[nodiscard]] ElementId hit_test_root(Point point) const;
  [[nodiscard]] std::vector<ElementId> preorder_ids() const;
  [[nodiscard]] std::vector<ElementId> enabled_preorder_ids() const;
  [[nodiscard]] AccessibilityTreeSnapshot accessibility_snapshot(
      AccessibilitySnapshotOptions options = {}) const;
  void paint(PaintList& paint_list) const;

 private:
  struct Node {
    std::unique_ptr<Element> element;
    ElementId id;
    std::optional<ElementId> parent;
    std::vector<ElementId> children;
    std::unordered_map<std::type_index, std::any> states;
  };

  [[nodiscard]] ElementId allocate_id();
  [[nodiscard]] static bool contains_id(
      const std::vector<ElementId>& ids,
      ElementId id);
  [[nodiscard]] ElementId find_keyed_child(
      const std::vector<ElementId>& old_children,
      const ElementKey& key,
      const std::vector<ElementId>& used_children) const;
  [[nodiscard]] ElementId find_unkeyed_child_at_index(
      const std::vector<ElementId>& old_children,
      std::size_t index,
      const std::vector<ElementId>& used_children) const;
  [[nodiscard]] Node* find_node(ElementId id);
  [[nodiscard]] const Node* find_node(ElementId id) const;
  void remove_subtree(ElementId id);
  void append_preorder_ids(ElementId id, std::vector<ElementId>& ids) const;
  void append_enabled_preorder_ids(
      ElementId id,
      std::vector<ElementId>& ids) const;
  void append_accessibility_nodes(
      ElementId id,
      const AccessibilitySnapshotOptions& options,
      std::vector<AccessibilityNode>& nodes) const;
  void paint_subtree(ElementId id, PaintList& paint_list) const;

  std::vector<Node> nodes_;
  ElementId root_id_;
  std::uint64_t next_id_ = 1;
};

} // namespace cgpui

#include "cgpui/ui/element_tree_templates.hpp"
