#include "cgpui/ui/static_element_tree.hpp"

#include <limits>

namespace cgpui {
namespace {

[[nodiscard]] bool valid_dense_id(
    std::span<const StaticElementNode> nodes,
    ElementId id) {
  if (id.value == 0 || id.value > nodes.size()) {
    return false;
  }

  const std::size_t index = static_cast<std::size_t>(id.value - 1);
  return nodes[index].id == id;
}

[[nodiscard]] ElementId hit_test_from(
    const StaticElementTreeView& tree,
    ElementId id,
    Point point) {
  const StaticElementNode* node = tree.get(id);
  if (node == nullptr || !node->enabled || !contains(node->bounds, point)) {
    return {};
  }

  ElementId best_child;
  int best_z_order = std::numeric_limits<int>::min();
  for (const ElementId child_id : tree.children(id)) {
    const ElementId candidate_id = hit_test_from(tree, child_id, point);
    const StaticElementNode* candidate = tree.get(candidate_id);
    if (candidate == nullptr) {
      continue;
    }
    if (best_child.value == 0 || candidate->z_order >= best_z_order) {
      best_child = candidate_id;
      best_z_order = candidate->z_order;
    }
  }

  return best_child.value == 0 ? node->id : best_child;
}

} // namespace

const StaticElementNode* StaticElementTreeView::get(ElementId id) const {
  if (!valid_dense_id(nodes_, id)) {
    return nullptr;
  }

  return &nodes_[static_cast<std::size_t>(id.value - 1)];
}

std::span<const ElementId> StaticElementTreeView::children(ElementId id) const {
  const StaticElementNode* node = get(id);
  if (node == nullptr || node->children.first > child_ids_.size() ||
      node->children.count > child_ids_.size() - node->children.first) {
    return {};
  }

  return child_ids_.subspan(node->children.first, node->children.count);
}

bool StaticElementTreeView::valid() const {
  if (nodes_.empty()) {
    return root_id_.value == 0 && child_ids_.empty();
  }
  if (!valid_dense_id(nodes_, root_id_)) {
    return false;
  }

  for (std::size_t index = 0; index < nodes_.size(); ++index) {
    const StaticElementNode& node = nodes_[index];
    if (node.id.value != index + 1 ||
        node.children.first > child_ids_.size() ||
        node.children.count > child_ids_.size() - node.children.first) {
      return false;
    }

    for (const ElementId child_id : children(node.id)) {
      const StaticElementNode* child = get(child_id);
      if (child == nullptr || child->parent_id != node.id) {
        return false;
      }
    }
  }

  return true;
}

ElementId StaticElementTreeView::hit_test(Point point) const {
  return hit_test_from(*this, root_id_, point);
}

StaticElementTreeView make_static_element_tree(
    std::span<const StaticElementNode> nodes,
    std::span<const ElementId> child_ids,
    ElementId root_id) {
  return StaticElementTreeView{nodes, child_ids, root_id};
}

} // namespace cgpui
