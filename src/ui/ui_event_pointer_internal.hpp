#pragma once

#include "cgpui/ui/runtime.hpp"

#include <optional>
#include <variant>

namespace cgpui {
namespace {

inline std::optional<Point> pointer_position_for(const PlatformEvent& event) {
  if (const auto* moved = std::get_if<PointerMoved>(&event);
      moved != nullptr) {
    return moved->position;
  }
  if (const auto* button = std::get_if<PointerButton>(&event);
      button != nullptr) {
    return button->position;
  }
  if (const auto* scrolled = std::get_if<PointerScrolled>(&event);
      scrolled != nullptr) {
    return scrolled->position;
  }
  if (const auto* drag_entered = std::get_if<DragEntered>(&event);
      drag_entered != nullptr) {
    return drag_entered->position;
  }
  if (const auto* drag_updated = std::get_if<DragUpdated>(&event);
      drag_updated != nullptr) {
    return drag_updated->position;
  }
  if (const auto* drag_dropped = std::get_if<DragDropped>(&event);
      drag_dropped != nullptr) {
    return drag_dropped->position;
  }
  if (const auto* drag_exited = std::get_if<DragExited>(&event);
      drag_exited != nullptr) {
    return drag_exited->position;
  }
  return {};
}

inline bool is_valid_pointer_capture_owner(const PointerCaptureOwner& owner) {
  if (const ViewId* view_id = owner.view_id(); view_id != nullptr) {
    return view_id->value != 0;
  }
  if (const ElementId* element_id = owner.element_id();
      element_id != nullptr) {
    return element_id->value != 0;
  }
  return false;
}

inline void apply_pointer_capture_owner_to_route(
    const PointerCaptureOwner& owner,
    EventRoute& route) {
  if (const ViewId* view_id = owner.view_id(); view_id != nullptr) {
    route.target_view_id = *view_id;
    route.target_element_id.reset();
    return;
  }
  if (const ElementId* element_id = owner.element_id();
      element_id != nullptr) {
    route.target_element_id = *element_id;
  }
}

inline ElementId hit_test_runtime_element_root(
    const ElementTree* tree,
    const Element* root,
    Point point) {
  if (tree != nullptr) {
    return tree->hit_test_root(point);
  }
  return root == nullptr ? ElementId{} : root->hit_test(point);
}

} // namespace

} // namespace cgpui
