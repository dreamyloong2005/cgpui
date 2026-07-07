#include "ui_internal.hpp"

namespace cgpui {

bool WindowRuntime::focus_next_element(bool reverse) {
  if (owned_element_tree_ == nullptr) {
    return false;
  }

  const std::vector<ElementId> focusable_ids = ordered_focusable_element_ids();
  if (focusable_ids.empty()) {
    return false;
  }

  std::size_t next_index = reverse ? focusable_ids.size() - 1 : 0;
  if (keyboard_focus_element_owner_.has_value()) {
    const auto current = std::find(
        focusable_ids.begin(),
        focusable_ids.end(),
        *keyboard_focus_element_owner_);
    if (current != focusable_ids.end()) {
      const std::size_t current_index =
          static_cast<std::size_t>(current - focusable_ids.begin());
      if (reverse) {
        next_index = current_index == 0 ? focusable_ids.size() - 1
                                        : current_index - 1;
      } else {
        next_index = current_index + 1 == focusable_ids.size()
                         ? 0
                         : current_index + 1;
      }
    }
  }

  request_keyboard_focus(focusable_ids[next_index]);
  if (Element* element = routed_element(focusable_ids[next_index]);
      element != nullptr) {
    element->focus(ElementFocusContext{.element_id = focusable_ids[next_index]});
  }
  return true;
}

} // namespace cgpui
