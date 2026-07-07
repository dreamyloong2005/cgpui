#include "cgpui/ui/element_scroll_nodes.hpp"

#include <optional>
#include <variant>

namespace cgpui {
namespace {

constexpr std::uint32_t kKeyHome = 36;
constexpr std::uint32_t kKeyEnd = 35;
constexpr std::uint32_t kKeyArrowUp = 38;
constexpr std::uint32_t kKeyArrowDown = 40;

void refresh_selection_flags(
    UniformListLayoutSnapshot& snapshot,
    const UniformListSelectionState& selection) {
  for (UniformListItemIdentity& item : snapshot.items) {
    item.selected = selection.selected(item.key);
  }
}

[[nodiscard]] bool changed_selection(
    const UniformListSelectionState& state,
    const UniformListSelection& selection) {
  return state.empty() || state.selected_index() != selection.index ||
         state.selected_key() != selection.key;
}

[[nodiscard]] std::optional<UniformListSelectionDirection> direction_for(
    const KeyboardKey& key) {
  if (key.action != KeyAction::pressed) {
    return std::nullopt;
  }
  if (key.key_code == kKeyArrowUp) {
    return UniformListSelectionDirection::previous;
  }
  if (key.key_code == kKeyArrowDown) {
    return UniformListSelectionDirection::next;
  }
  if (key.key_code == kKeyHome) {
    return UniformListSelectionDirection::first;
  }
  if (key.key_code == kKeyEnd) {
    return UniformListSelectionDirection::last;
  }
  return std::nullopt;
}

[[nodiscard]] Point content_point(Point point, const ScrollState* state) {
  if (state == nullptr) {
    return point;
  }
  const Point offset = state->offset();
  return Point{.x = point.x + offset.x, .y = point.y + offset.y};
}

} // namespace

EventResult ScrollableListElement::handle_event(
    const PlatformEvent& event,
    const ElementEventContext& context) {
  if (!enabled()) {
    return EventResult::unhandled();
  }

  if (const auto* pointer = std::get_if<PointerButton>(&event);
      pointer != nullptr && pointer->pressed &&
      pointer->button == MouseButton::left) {
    const std::optional<UniformListSelection> selection =
        select_uniform_list_item_at_point(
            layout_snapshot_.items,
            content_point(pointer->position, state_),
            UniformListSelectionSource::pointer);
    if (selection.has_value()) {
      selection_.set(*selection);
      refresh_selection_flags(layout_snapshot_, selection_);
      return EventResult::consumed_event();
    }
  }

  if (const auto* key = std::get_if<KeyboardKey>(&event); key != nullptr) {
    const std::optional<UniformListSelectionDirection> direction =
        direction_for(*key);
    if (direction.has_value()) {
      const std::optional<UniformListSelection> selection =
          move_uniform_list_selection(
              layout_snapshot_.items,
              selection_,
              *direction);
      if (selection.has_value() && changed_selection(selection_, *selection)) {
        selection_.set(*selection);
        refresh_selection_flags(layout_snapshot_, selection_);
        return EventResult::consumed_event();
      }
    }
  }

  return content_.handle_event(event, context);
}

} // namespace cgpui
