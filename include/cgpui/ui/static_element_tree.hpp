#pragma once

#include "cgpui/core/geometry.hpp"
#include "cgpui/ui/element_ids.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace cgpui {

enum class StaticElementKind {
  container,
  row,
  column,
  text,
  image,
  custom,
};

struct StaticElementChildRange {
  std::uint32_t first = 0;
  std::uint32_t count = 0;

  [[nodiscard]] constexpr std::uint32_t end() const {
    return first + count;
  }

  [[nodiscard]] constexpr bool empty() const {
    return count == 0;
  }
};

struct StaticElementNode {
  ElementId id;
  ElementId parent_id;
  StaticElementChildRange children;
  StaticElementKind kind = StaticElementKind::container;
  Rect bounds;
  Size intrinsic_size;
  std::string_view text;
  bool enabled = true;
  bool focusable = false;
  int z_order = 0;
};

class StaticElementTreeView {
 public:
  constexpr StaticElementTreeView() = default;

  constexpr StaticElementTreeView(
      std::span<const StaticElementNode> nodes,
      std::span<const ElementId> child_ids,
      ElementId root_id)
      : nodes_(nodes),
        child_ids_(child_ids),
        root_id_(root_id) {}

  [[nodiscard]] constexpr std::span<const StaticElementNode> nodes() const {
    return nodes_;
  }

  [[nodiscard]] constexpr std::span<const ElementId> child_ids() const {
    return child_ids_;
  }

  [[nodiscard]] constexpr ElementId root_id() const {
    return root_id_;
  }

  [[nodiscard]] constexpr bool empty() const {
    return nodes_.empty();
  }

  [[nodiscard]] constexpr std::size_t size() const {
    return nodes_.size();
  }

  [[nodiscard]] const StaticElementNode* get(ElementId id) const;
  [[nodiscard]] std::span<const ElementId> children(ElementId id) const;
  [[nodiscard]] bool valid() const;
  [[nodiscard]] ElementId hit_test(Point point) const;

  template <typename Visitor>
  void for_each_preorder(Visitor&& visitor) const {
    visit_preorder(root_id_, visitor);
  }

 private:
  template <typename Visitor>
  void visit_preorder(ElementId id, Visitor& visitor) const {
    const StaticElementNode* node = get(id);
    if (node == nullptr) {
      return;
    }

    visitor(*node);
    for (const ElementId child_id : children(id)) {
      visit_preorder(child_id, visitor);
    }
  }

  std::span<const StaticElementNode> nodes_;
  std::span<const ElementId> child_ids_;
  ElementId root_id_;
};

[[nodiscard]] StaticElementTreeView make_static_element_tree(
    std::span<const StaticElementNode> nodes,
    std::span<const ElementId> child_ids,
    ElementId root_id);

} // namespace cgpui
