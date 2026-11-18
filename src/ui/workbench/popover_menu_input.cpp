#include "cgpui/ui/popover_menu.hpp"

#include "cgpui/ui/runtime_context.hpp"
#include "workbench_internal.hpp"

#include <string>
#include <variant>

namespace cgpui {

bool PopoverMenu::selectable(const MenuItem& item) const {
  return item.kind != MenuItemKind::separator && item.enabled;
}

// Deepest level first: submenus overlap their parents.
std::optional<std::pair<std::size_t, std::size_t>> PopoverMenu::item_at(
    Point point) const {
  for (std::size_t level = levels_.size(); level-- > 0;) {
    if (!contains(levels_[level].bounds, point)) continue;
    for (std::size_t index = 0; index < levels_[level].items->size(); ++index) {
      if (contains(item_rect(level, index), point)) {
        return std::pair{level, index};
      }
    }
    return std::pair{level, levels_[level].items->size()};
  }
  return std::nullopt;
}

// Moves the highlight in the deepest level, skipping separators and
// disabled items and wrapping at the ends.
void PopoverMenu::move_highlight(int direction, bool to_edge) {
  if (levels_.empty()) return;
  Level& level = levels_.back();
  const std::vector<MenuItem>& items = *level.items;
  const auto count = static_cast<int>(items.size());
  if (count == 0) return;
  int at = to_edge ? (direction > 0 ? -1 : count)
                   : (level.highlighted.has_value()
                          ? static_cast<int>(*level.highlighted)
                          : (direction > 0 ? -1 : count));
  for (int step = 0; step < count; ++step) {
    at += direction;
    if (at < 0) at = count - 1;
    if (at >= count) at = 0;
    if (selectable(items[static_cast<std::size_t>(at)])) {
      level.highlighted = static_cast<std::size_t>(at);
      return;
    }
  }
}

void PopoverMenu::activate(
    std::size_t level,
    std::size_t index,
    const WindowRuntimeContext& context) {
  const MenuItem& item = (*levels_[level].items)[index];
  if (!selectable(item)) return;
  if (item.kind == MenuItemKind::submenu) {
    open_submenu(level, index);
    move_highlight(1, true);
    context.request_render();
    return;
  }
  // Close first (restoring focus), then report: the handler may reopen.
  const std::string id = item.id;
  close(PopoverCloseReason::activated, context);
  if (activate_handler_) activate_handler_(id);
}

EventResult PopoverMenu::handle_event(
    const PlatformEvent& event,
    const WindowRuntimeContext& context) {
  if (!open_) return EventResult::unhandled();
  if (const auto* focused = std::get_if<WindowFocused>(&event)) {
    if (!focused->focused) close(PopoverCloseReason::focus_lost, context);
    return EventResult::unhandled();
  }
  if (const auto* key = std::get_if<KeyboardKey>(&event)) {
    return handle_key(*key, context);
  }
  if (std::holds_alternative<TextInput>(event)) {
    return EventResult::consumed_event();
  }
  return handle_pointer(event, context);
}

// The menu is modal while open: every key is consumed.
EventResult PopoverMenu::handle_key(
    const KeyboardKey& key,
    const WindowRuntimeContext& context) {
  using namespace workbench_detail;
  if (key.action != KeyAction::pressed) return EventResult::consumed_event();
  Level& level = levels_.back();
  switch (key.key_code) {
    case key_escape:
    case key_left:
      if (levels_.size() > 1) {
        truncate(levels_.size() - 1);
      } else if (key.key_code == key_escape) {
        close(PopoverCloseReason::escape, context);
        return EventResult::consumed_event();
      }
      break;
    case key_tab:
      close(PopoverCloseReason::focus_lost, context);
      return EventResult::consumed_event();
    case key_up: move_highlight(-1, false); break;
    case key_down: move_highlight(1, false); break;
    case key_home: move_highlight(1, true); break;
    case key_end: move_highlight(-1, true); break;
    case key_right:
      if (level.highlighted.has_value() &&
          (*level.items)[*level.highlighted].kind == MenuItemKind::submenu) {
        activate(levels_.size() - 1, *level.highlighted, context);
      }
      break;
    case key_enter:
    case key_space:
      if (level.highlighted.has_value()) {
        activate(levels_.size() - 1, *level.highlighted, context);
        return EventResult::consumed_event();
      }
      break;
    default:
      return EventResult::consumed_event();
  }
  context.request_render();
  return EventResult::consumed_event();
}

EventResult PopoverMenu::handle_pointer(
    const PlatformEvent& event,
    const WindowRuntimeContext& context) {
  if (const auto* moved = std::get_if<PointerMoved>(&event)) {
    const auto hit = item_at(moved->position);
    if (hit.has_value() && hit->second < levels_[hit->first].items->size()) {
      const auto [level, index] = *hit;
      const MenuItem& item = (*levels_[level].items)[index];
      const std::optional<std::size_t> before = levels_[level].highlighted;
      const std::size_t depth = levels_.size();
      levels_[level].highlighted =
          selectable(item) ? std::optional<std::size_t>{index} : std::nullopt;
      // Hovering opens a submenu at once and closes deeper unrelated ones.
      const bool opens_child = depth > level + 1 &&
                               levels_[level + 1].parent_index == index;
      if (!opens_child) {
        truncate(level + 1);
        if (item.kind == MenuItemKind::submenu && item.enabled) {
          open_submenu(level, index);
        }
      }
      if (before != levels_[level].highlighted || depth != levels_.size()) {
        context.request_render();
      }
    }
    return EventResult::consumed_event();
  }
  if (const auto* button = std::get_if<PointerButton>(&event)) {
    const auto hit = item_at(button->position);
    if (!hit.has_value()) {
      if (button->pressed) close(PopoverCloseReason::outside_click, context);
      return EventResult::consumed_event();
    }
    // A release activates only after a press inside the menu, so the
    // release of the click that opened a popover does not pick an item.
    if (button->pressed) {
      pressed_inside_ = true;
      return EventResult::consumed_event();
    }
    if (pressed_inside_ && button->button == MouseButton::left &&
        hit->second < levels_[hit->first].items->size()) {
      activate(hit->first, hit->second, context);
    }
    return EventResult::consumed_event();
  }
  // Scrolls and other pointer input stay inside the modal menu.
  if (std::holds_alternative<PointerScrolled>(event) ||
      std::holds_alternative<PointerExited>(event)) {
    return EventResult::consumed_event();
  }
  return EventResult::unhandled();
}

} // namespace cgpui
