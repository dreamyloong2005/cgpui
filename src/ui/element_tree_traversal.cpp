#include "cgpui/ui/element_tree.hpp"

#include <limits>

namespace cgpui {

ElementId ElementTree::root_id() const {
  return root_id_;
}

std::optional<ElementId> ElementTree::parent(ElementId id) const {
  const Node* node = find_node(id);
  return node == nullptr ? std::optional<ElementId>{} : node->parent;
}

std::span<const ElementId> ElementTree::children(ElementId id) const {
  const Node* node = find_node(id);
  if (node == nullptr) {
    return {};
  }
  return node->children;
}

LayoutOutput ElementTree::layout_root(LayoutInput input) const {
  const Element* root = get(root_id_);
  if (root == nullptr) {
    return LayoutOutput{
        .size = constrain_size({}, input.constraints),
    };
  }
  const LayoutOutput output = root->layout(input);
  root->place_layout(Rect{.origin = output.origin, .size = output.size});
  layout_children(root_id_, input);
  return output;
}

// Tree children are laid out after their parent and placed at the parent's
// window origin plus their own layout origin, so bounds stay window
// coordinates at every depth.
void ElementTree::layout_children(ElementId id, LayoutInput input) const {
  const Node* node = find_node(id);
  if (node == nullptr) {
    return;
  }
  const std::optional<Rect> parent_bounds = node->element->layout_bounds();
  const Point origin = parent_bounds.has_value() ? parent_bounds->origin : Point{};
  LayoutInput child_input = input;
  child_input.constraints = {};
  for (const ElementId child_id : node->children) {
    const Node* child = find_node(child_id);
    if (child == nullptr) {
      continue;
    }
    const LayoutOutput output = child->element->layout(child_input);
    child->element->place_layout(Rect{
        .origin = {.x = origin.x + output.origin.x,
                   .y = origin.y + output.origin.y},
        .size = output.size,
    });
    layout_children(child_id, input);
  }
}

ElementId ElementTree::hit_test_root(Point point) const {
  return hit_test_subtree(root_id_, point);
}

// Topmost first: tree children before their parent, higher z order first,
// later siblings first within one z order. Walks distinct z levels instead
// of sorting so hit tests do not allocate.
ElementId ElementTree::hit_test_subtree(ElementId id, Point point) const {
  const Node* node = find_node(id);
  if (node == nullptr) {
    return {};
  }
  const std::span<const ElementId> children = node->children;
  long long upper = std::numeric_limits<long long>::max();
  while (!children.empty()) {
    long long level = std::numeric_limits<long long>::min();
    for (const ElementId child : children) {
      const Element* element = get(child);
      const long long z = element == nullptr ? level : element->z_order();
      if (z < upper && z > level) level = z;
    }
    if (level == std::numeric_limits<long long>::min()) break;
    for (std::size_t index = children.size(); index-- > 0;) {
      const Element* element = get(children[index]);
      if (element == nullptr || element->z_order() != level) continue;
      const ElementId hit = hit_test_subtree(children[index], point);
      if (hit.value != 0) return hit;
    }
    upper = level;
  }
  return node->element->hit_test(point);
}

std::vector<ElementId> ElementTree::preorder_ids() const {
  std::vector<ElementId> ids;
  append_preorder_ids(root_id_, ids);
  return ids;
}

std::vector<ElementId> ElementTree::enabled_preorder_ids() const {
  std::vector<ElementId> ids;
  append_enabled_preorder_ids(root_id_, ids);
  return ids;
}

void ElementTree::append_preorder_ids(
    ElementId id,
    std::vector<ElementId>& ids) const {
  const Node* node = find_node(id);
  if (node == nullptr) {
    return;
  }

  ids.push_back(id);
  for (ElementId child_id : node->children) {
    append_preorder_ids(child_id, ids);
  }
}

void ElementTree::append_enabled_preorder_ids(
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

} // namespace cgpui
