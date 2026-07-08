#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::refresh_route_ancestry(EventRoute& route) const {
  route.element_ancestry.clear();
  if (route.target_element_id.has_value()) {
    route.element_ancestry =
        element_ancestry_for(*route.target_element_id);
    if (const std::optional<ViewId> child_view_id =
            child_view_target_for(*route.target_element_id);
        child_view_id.has_value()) {
      route.target_view_id = *child_view_id;
    }
  }
  route.view_ancestry = view_ancestry_for(route.target_view_id);
}

std::vector<ElementId> WindowRuntime::element_ancestry_for(
    ElementId element_id) const {
  std::vector<ElementId> ancestry;
  if (element_id.value == 0) {
    return ancestry;
  }

  if (static_element_tree_installed()) {
    ElementId current = element_id;
    while (current.value != 0 && static_element_node(current) != nullptr) {
      ancestry.push_back(current);
      const std::optional<ElementId> parent = element_parent(current);
      if (!parent.has_value()) {
        break;
      }
      current = *parent;
    }
    return ancestry;
  }

  if (owned_element_tree_ != nullptr) {
    ElementId current = element_id;
    while (current.value != 0 && owned_element_tree_->get(current) != nullptr) {
      ancestry.push_back(current);
      const std::optional<ElementId> parent =
          owned_element_tree_->parent(current);
      if (!parent.has_value()) {
        break;
      }
      current = *parent;
    }
    return ancestry;
  }

  if (element_root_ != nullptr && element_root_->id() == element_id) {
    ancestry.push_back(element_id);
  }
  return ancestry;
}

std::vector<ViewId> WindowRuntime::view_ancestry_for(ViewId view_id) const {
  std::vector<ViewId> ancestry;
  if (view_id.value != 0) {
    ancestry.push_back(view_id);
  }
  if (view_id != root_view_id_ && root_view_id_.value != 0) {
    ancestry.push_back(root_view_id_);
  }
  return ancestry;
}

std::optional<ViewId> WindowRuntime::child_view_target_for(
    ElementId element_id) const {
  const auto* child_view =
      dynamic_cast<const ChildViewElement*>(routed_element(element_id));
  if (child_view == nullptr || child_view->view_id().value == 0 ||
      find_view(child_view->view_id()) == nullptr) {
    return {};
  }
  return child_view->view_id();
}

Element* WindowRuntime::routed_element(ElementId element_id) {
  if (element_id.value == 0) {
    return nullptr;
  }
  if (owned_element_tree_ != nullptr) {
    return owned_element_tree_->get(element_id);
  }
  if (element_root_ != nullptr && element_root_->id() == element_id) {
    return const_cast<Element*>(element_root_);
  }
  return nullptr;
}

const Element* WindowRuntime::routed_element(ElementId element_id) const {
  if (element_id.value == 0) {
    return nullptr;
  }
  if (owned_element_tree_ != nullptr) {
    return owned_element_tree_->get(element_id);
  }
  if (element_root_ != nullptr && element_root_->id() == element_id) {
    return element_root_;
  }
  return nullptr;
}

} // namespace cgpui
