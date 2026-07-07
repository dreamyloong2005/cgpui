#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::request_style_state_invalidation(
    std::optional<ElementId> previous_element_id,
    std::optional<ElementId> next_element_id) {
  if (previous_element_id == next_element_id) {
    return;
  }
  request_render();
}

} // namespace cgpui
