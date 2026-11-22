#include "ui_internal.hpp"

namespace cgpui {

// Hover element and cursor of an additional window, from its hit element.
void WindowRuntime::update_record_hover_cursor(
    WindowRuntimeRecord& record,
    const PlatformEvent& event,
    std::optional<ElementId> hit) {
  const bool exited = std::holds_alternative<PointerExited>(event);
  if (!exited && !std::holds_alternative<PointerMoved>(event)) {
    return;
  }
  CursorShape cursor = CursorShape::default_arrow;
  record.input.hovered_element_id = exited ? std::nullopt : hit;
  const ElementTree* tree = element_tree_for_record(record);
  if (!exited && hit.has_value() && tree != nullptr) {
    const Element* element = tree->get(*hit);
    const WindowElementHost& host = state_->elements.window_element_hosts_[record.runtime_id.value];
    if (const auto found = host.cursors.find(hit->value);
        found != host.cursors.end() && (element == nullptr || element->enabled())) {
      cursor = found->second;
    }
  }
  if (record.input.cursor_shape != cursor && record.window != nullptr) {
    record.window->set_cursor(cursor);
  }
  record.input.cursor_shape = cursor;
}

} // namespace cgpui
