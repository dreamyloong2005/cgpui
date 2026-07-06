#include "element_layer_ordering.hpp"

#include <algorithm>
#include <cstddef>

namespace cgpui {

namespace {

struct ConstOrderedChild {
  std::size_t index = 0;
  const Element* element = nullptr;
};

struct OrderedChild {
  std::size_t index = 0;
  Element* element = nullptr;
};

[[nodiscard]] std::vector<ConstOrderedChild> collect_const_children(
    const std::vector<std::unique_ptr<Element>>& children) {
  std::vector<ConstOrderedChild> ordered;
  ordered.reserve(children.size());
  for (std::size_t index = 0; index < children.size(); ++index) {
    if (children[index] != nullptr) {
      ordered.push_back(ConstOrderedChild{
          .index = index,
          .element = children[index].get(),
      });
    }
  }
  return ordered;
}

[[nodiscard]] std::vector<OrderedChild> collect_children(
    std::vector<std::unique_ptr<Element>>& children) {
  std::vector<OrderedChild> ordered;
  ordered.reserve(children.size());
  for (std::size_t index = 0; index < children.size(); ++index) {
    if (children[index] != nullptr) {
      ordered.push_back(OrderedChild{
          .index = index,
          .element = children[index].get(),
      });
    }
  }
  return ordered;
}

[[nodiscard]] bool paints_before(ConstOrderedChild lhs, ConstOrderedChild rhs) {
  const int lhs_order = lhs.element->z_order();
  const int rhs_order = rhs.element->z_order();
  if (lhs_order != rhs_order) {
    return lhs_order < rhs_order;
  }
  return lhs.index < rhs.index;
}

[[nodiscard]] bool hits_before(ConstOrderedChild lhs, ConstOrderedChild rhs) {
  const int lhs_order = lhs.element->z_order();
  const int rhs_order = rhs.element->z_order();
  if (lhs_order != rhs_order) {
    return lhs_order > rhs_order;
  }
  return lhs.index > rhs.index;
}

[[nodiscard]] bool events_before(OrderedChild lhs, OrderedChild rhs) {
  const int lhs_order = lhs.element->z_order();
  const int rhs_order = rhs.element->z_order();
  if (lhs_order != rhs_order) {
    return lhs_order > rhs_order;
  }
  return lhs.index > rhs.index;
}

} // namespace

std::vector<const Element*> paint_ordered_children(
    const std::vector<std::unique_ptr<Element>>& children) {
  std::vector<ConstOrderedChild> ordered = collect_const_children(children);
  std::sort(ordered.begin(), ordered.end(), paints_before);
  std::vector<const Element*> elements;
  elements.reserve(ordered.size());
  for (const ConstOrderedChild child : ordered) {
    elements.push_back(child.element);
  }
  return elements;
}

std::vector<const Element*> hit_test_ordered_children(
    const std::vector<std::unique_ptr<Element>>& children) {
  std::vector<ConstOrderedChild> ordered = collect_const_children(children);
  std::sort(ordered.begin(), ordered.end(), hits_before);
  std::vector<const Element*> elements;
  elements.reserve(ordered.size());
  for (const ConstOrderedChild child : ordered) {
    elements.push_back(child.element);
  }
  return elements;
}

std::vector<Element*> event_ordered_children(
    std::vector<std::unique_ptr<Element>>& children) {
  std::vector<OrderedChild> ordered = collect_children(children);
  std::sort(ordered.begin(), ordered.end(), events_before);
  std::vector<Element*> elements;
  elements.reserve(ordered.size());
  for (const OrderedChild child : ordered) {
    elements.push_back(child.element);
  }
  return elements;
}

} // namespace cgpui
