#include "cgpui/ui/tree_view.hpp"

#include "cgpui/ui/runtime_context.hpp"
#include "workbench_internal.hpp"

#include <algorithm>
#include <cctype>
#include <variant>

namespace cgpui {

EventResult TreeView::handle_event(
    const PlatformEvent& event,
    const WindowRuntimeContext& context) {
  const std::optional<ElementId> owner =
      context.input_state().keyboard_focus_element_owner;
  focused_ = owner.has_value() && tree_id_.value != 0 && *owner == tree_id_;
  if (const auto* key = std::get_if<KeyboardKey>(&event)) {
    constexpr std::uint32_t shift_key = 16;
    constexpr std::uint32_t control_key = 17;
    held_ = key->modifiers;
    const bool down = key->action == KeyAction::pressed;
    if (key->key_code == shift_key) held_.shift = down;
    if (key->key_code == control_key) held_.control = down;
    return handle_key(*key, context);
  }
  if (const auto* text = std::get_if<TextInput>(&event)) {
    if (!focused_ || text->composed || text->modifiers.control ||
        text->modifiers.alt) {
      return EventResult::unhandled();
    }
    return handle_type_ahead(text->text, context);
  }
  return handle_pointer(event, context);
}

EventResult TreeView::handle_pointer(
    const PlatformEvent& event,
    const WindowRuntimeContext& context) {
  if (const auto* moved = std::get_if<PointerMoved>(&event)) {
    const std::optional<std::size_t> row = row_at(moved->position);
    if (row != hovered_row_) {
      hovered_row_ = row;
      context.request_paint();
    }
    return EventResult::unhandled();
  }
  if (std::holds_alternative<PointerExited>(event)) {
    if (hovered_row_.has_value()) context.request_paint();
    hovered_row_.reset();
    return EventResult::unhandled();
  }
  if (const auto* scrolled = std::get_if<PointerScrolled>(&event)) {
    if (!cgpui::contains(bounds_, scrolled->position)) return EventResult::unhandled();
    // A wheel notch scrolls three rows; precise deltas are pixels.
    const float delta = scrolled->precise ? scrolled->delta.y
                                          : scrolled->delta.y * 3.0F * options_.row_height;
    scroll_to(scroll_ - delta);
    hovered_row_ = row_at(scrolled->position);
    context.request_render();
    return EventResult::consumed_event();
  }
  const auto* button = std::get_if<PointerButton>(&event);
  if (button == nullptr || !button->pressed) return EventResult::unhandled();
  const std::optional<std::size_t> row = row_at(button->position);
  if (!row.has_value()) return EventResult::unhandled();
  const TreeItemId item = rows_[*row];
  const Node& node = nodes_.at(item);
  if (tree_id_.value != 0) context.request_keyboard_focus(tree_id_);
  if (button->button == MouseButton::right) {
    if (!selected(item)) select(item);
    emit(TreeViewEventKind::context_menu_requested, item, button->position);
    context.request_render();
    return EventResult::consumed_event();
  }
  if (button->button != MouseButton::left) return EventResult::unhandled();
  const float twisty_x =
      bounds_.origin.x + 4.0F + static_cast<float>(node.depth) * options_.indent;
  const bool on_twisty = node.has_children && button->position.x >= twisty_x &&
                         button->position.x < twisty_x + options_.indent;
  if (on_twisty || (button->click_count >= 2 && node.has_children)) {
    if (node.expanded) {
      collapse(item);
    } else {
      expand(item);
    }
    focused_item_ = item;
  } else if (button->click_count >= 2) {
    emit(TreeViewEventKind::activated, item);
  } else {
    // Pointer events carry no modifiers; the tree tracks Shift / Ctrl from
    // the keyboard events it sees.
    const TreeSelectMode mode = held_.control ? TreeSelectMode::toggle
                                : held_.shift ? TreeSelectMode::extend
                                              : TreeSelectMode::replace;
    select(item, mode);
  }
  context.request_render();
  return EventResult::consumed_event();
}

EventResult TreeView::handle_key(
    const KeyboardKey& key,
    const WindowRuntimeContext& context) {
  using namespace workbench_detail;
  if (!focused_ || key.action != KeyAction::pressed) return EventResult::unhandled();
  const std::size_t count = row_count();
  if (count == 0) return EventResult::unhandled();
  const std::optional<std::size_t> current =
      focused_item_.has_value() ? row_of(*focused_item_) : std::nullopt;
  const std::size_t at = current.value_or(0);
  const TreeSelectMode move_mode =
      key.modifiers.shift ? TreeSelectMode::extend : TreeSelectMode::replace;
  const std::size_t page = page_rows();
  switch (key.key_code) {
    case key_up:
      focus_row(current.has_value() && at > 0 ? at - 1 : 0, move_mode);
      break;
    case key_down:
      focus_row(current.has_value() ? std::min(count - 1, at + 1) : 0, move_mode);
      break;
    case key_page_up: focus_row(at > page ? at - page : 0, move_mode); break;
    case key_page_down: focus_row(std::min(count - 1, at + page), move_mode); break;
    case key_home: focus_row(0, move_mode); break;
    case key_end: focus_row(count - 1, move_mode); break;
    case key_left: {
      if (!current.has_value()) return EventResult::unhandled();
      const TreeItemId item = rows_[at];
      if (expanded(item)) {
        collapse(item);
      } else if (const auto up = parent(item)) {
        select(*up);
        scroll_into_view(*up);
      }
      break;
    }
    case key_right: {
      if (!current.has_value()) return EventResult::unhandled();
      const TreeItemId item = rows_[at];
      const Node& node = nodes_.at(item);
      if (node.has_children && !node.expanded) {
        expand(item);
      } else if (node.expanded && !node.children.empty()) {
        select(node.children.front());
        scroll_into_view(node.children.front());
      }
      break;
    }
    case key_enter:
      if (!current.has_value()) return EventResult::unhandled();
      emit(TreeViewEventKind::activated, rows_[at]);
      break;
    case key_space:
      if (!key.modifiers.control || !current.has_value()) {
        return EventResult::unhandled();
      }
      select(rows_[at], TreeSelectMode::toggle);
      break;
    case key_apps:
    case key_f10: {
      if (key.key_code == key_f10 && !key.modifiers.shift) {
        return EventResult::unhandled();
      }
      if (!current.has_value()) return EventResult::unhandled();
      const Rect rect = row_rect(at);
      emit(TreeViewEventKind::context_menu_requested, rows_[at],
           Point{rect.origin.x + 24.0F, rect.origin.y + rect.size.height});
      break;
    }
    default:
      return EventResult::unhandled();
  }
  context.request_render();
  return EventResult::consumed_event();
}

// Type-to-select: typed characters build a prefix (reset after
// type_ahead_timeout_ms without typing) and focus the next row whose label
// starts with it, ASCII case-insensitively, wrapping at the end.
EventResult TreeView::handle_type_ahead(
    std::string_view text,
    const WindowRuntimeContext& context) {
  if (text.empty() || text == " ") return EventResult::unhandled();
  std::string& prefix = type_ahead_->text;
  for (const char character : text) {
    prefix.push_back(static_cast<char>(
        std::tolower(static_cast<unsigned char>(character))));
  }
  const std::uint64_t generation = ++type_ahead_->generation;
  (void)context.schedule_timer(
      options_.type_ahead_timeout_ms,
      [weak = std::weak_ptr<TypeAhead>(type_ahead_),
       generation](const WindowRuntimeContext&) {
        const std::shared_ptr<TypeAhead> state = weak.lock();
        if (state != nullptr && state->generation == generation) {
          state->text.clear();
        }
      });
  const std::size_t count = row_count();
  const std::size_t at = focused_item_.has_value()
      ? row_of(*focused_item_).value_or(0) : 0;
  const std::size_t start = prefix.size() == 1 ? at + 1 : at;
  for (std::size_t step = 0; step < count; ++step) {
    const std::size_t row = (start + step) % count;
    const std::string& label = nodes_.at(rows_[row]).label;
    if (label.size() < prefix.size()) continue;
    bool match = true;
    for (std::size_t index = 0; match && index < prefix.size(); ++index) {
      match = std::tolower(static_cast<unsigned char>(label[index])) ==
              static_cast<unsigned char>(prefix[index]);
    }
    if (match) {
      focus_row(row, TreeSelectMode::replace);
      context.request_render();
      break;
    }
  }
  return EventResult::consumed_event();
}

} // namespace cgpui
