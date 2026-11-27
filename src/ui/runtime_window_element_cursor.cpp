#include "ui_internal.hpp"

namespace cgpui {
namespace {
struct CursorAttempt {
  CursorShape shape;
  const CursorAttempt* previous;
  bool contains(CursorShape cursor) const {
    for (auto* attempt = this; attempt != nullptr; attempt = attempt->previous)
      if (attempt->shape == cursor) return true;
    return false;
  }
};
} // namespace

// Hover element and cursor of an additional window, from its hit element.
std::optional<ElementId> WindowRuntime::update_record_hover_cursor(
    WindowRuntimeRecord& record,
    const PlatformEvent& event) {
  const bool exited = std::holds_alternative<PointerExited>(event);
  const auto position = pointer_position_for(event);
  const auto hit_test = [&]() -> std::optional<ElementId> {
    if (!position) return {};
    ElementId hit;
    if (const auto* tree = static_tree_for_record(record)) hit = tree->hit_test(*position);
    else if (const auto* tree = element_tree_for_record(record)) hit = tree->hit_test_root(*position);
    return hit.value == 0 ? std::optional<ElementId>{} : hit;
  };
  const auto cursor_for = [&](std::optional<ElementId> hit) {
    if (exited || !hit) return CursorShape::default_arrow;
    const auto* tree = element_tree_for_record(record);
    const auto* static_tree = static_tree_for_record(record);
    const auto* element = tree == nullptr ? nullptr : tree->get(*hit);
    const auto* node = static_tree == nullptr ? nullptr : static_tree->get(*hit);
    if (node != nullptr ? !node->enabled : element != nullptr && !element->enabled())
      return CursorShape::default_arrow;
    const auto host = state_->elements.window_element_hosts_.find(record.runtime_id.value);
    if (host != state_->elements.window_element_hosts_.end()) {
      if (const auto found = host->second.cursors.find(hit->value); found != host->second.cursors.end())
        return found->second;
    }
    return CursorShape::default_arrow;
  };
  const auto synchronize = [&](auto&& self, const CursorAttempt* attempted) -> std::optional<ElementId> {
    const auto hit = hit_test();
    if (!exited && !std::holds_alternative<PointerMoved>(event)) return hit;
    record.input.hovered_element_id = exited ? std::nullopt : hit;
    const auto cursor = cursor_for(hit);
    if (record.input.cursor_shape == cursor || record.window == nullptr) return hit;
    // A callback can redraw forever with no fixed point. Submit each shape at
    // most once per input; keep the actual last submission and newest hit on a
    // cycle. The next input may converge. Stack depth is bounded by distinct
    // cursor values, without heap storage or assumptions about enum ordinals.
    if (attempted != nullptr && attempted->contains(cursor)) return hit;
    const CursorAttempt attempt{cursor, attempted};
    record.input.cursor_shape = cursor;
    record.window->set_cursor(cursor);
    if (!record.active) return {};
    // Never retain Element pointers or View-borrowed spans across set_cursor.
    return self(self, &attempt);
  };
  return synchronize(synchronize, nullptr);
}

} // namespace cgpui
