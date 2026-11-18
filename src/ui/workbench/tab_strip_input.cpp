#include "cgpui/ui/tab_strip.hpp"

#include "cgpui/ui/runtime_context.hpp"
#include "workbench_internal.hpp"

#include <cmath>
#include <variant>

namespace cgpui {

void TabStrip::emit(TabStripEventKind kind, std::size_t index, std::size_t to) {
  if (!handler_ || index >= tabs_.size()) return;
  handler_(TabStripEvent{
      .kind = kind,
      .tab = tabs_[index].id,
      .from_index = index,
      .to_index = kind == TabStripEventKind::reorder ? to : index,
  });
}

EventResult TabStrip::handle_event(
    const PlatformEvent& event,
    const WindowRuntimeContext& context) {
  const std::optional<ElementId> owner =
      context.input_state().keyboard_focus_element_owner;
  focused_ = owner.has_value() && strip_id_.value != 0 && *owner == strip_id_;
  if (const auto* key = std::get_if<KeyboardKey>(&event)) {
    return handle_key(*key, context);
  }
  return handle_pointer(event, context);
}

EventResult TabStrip::handle_key(
    const KeyboardKey& key,
    const WindowRuntimeContext& context) {
  using namespace workbench_detail;
  if (!focused_ || key.action != KeyAction::pressed || tabs_.empty() ||
      key.modifiers.control || key.modifiers.alt) {
    return EventResult::unhandled();
  }
  const std::size_t last = tabs_.size() - 1;
  const std::size_t current =
      selected_.has_value() ? index_of(*selected_).value_or(0) : 0;
  std::size_t next = current;
  switch (key.key_code) {
    case key_left: next = current == 0 ? 0 : current - 1; break;
    case key_right: next = current >= last ? last : current + 1; break;
    case key_home: next = 0; break;
    case key_end: next = last; break;
    default: return EventResult::unhandled();
  }
  if (next != current || !selected_.has_value()) {
    emit(TabStripEventKind::select, next);
    scroll_into_view(next);
    context.request_render();
  }
  return EventResult::consumed_event();
}

EventResult TabStrip::handle_pointer(
    const PlatformEvent& event,
    const WindowRuntimeContext& context) {
  if (const auto* moved = std::get_if<PointerMoved>(&event)) {
    if (pressed_.has_value() && !pressed_close_) {
      if (!dragging_ &&
          std::fabs(moved->position.x - press_x_) > options_.drag_threshold) {
        dragging_ = true;
      }
      if (dragging_) {
        drag_x_ = moved->position.x;
        context.request_paint();
        return EventResult::consumed_event();
      }
    }
    const std::optional<std::size_t> hovered = tab_at(moved->position);
    const bool close = hovered.has_value() && tabs_[*hovered].closable &&
                       contains(close_rect(*hovered), moved->position);
    if (hovered != hovered_ || close != hover_close_) {
      hovered_ = hovered;
      hover_close_ = close;
      context.request_paint();
    }
    return EventResult::unhandled();
  }
  if (std::holds_alternative<PointerExited>(event)) {
    if (hovered_.has_value()) context.request_paint();
    hovered_.reset();
    hover_close_ = false;
    return EventResult::unhandled();
  }
  if (const auto* scrolled = std::get_if<PointerScrolled>(&event)) {
    if (!contains(bounds_, scrolled->position)) return EventResult::unhandled();
    // Wheel notches scroll 40 px; precise deltas are already pixels.
    const float delta = scrolled->delta.x + scrolled->delta.y;
    scroll_by(scrolled->precise ? -delta : -delta * 40.0F);
    context.request_paint();
    return EventResult::consumed_event();
  }
  const auto* button = std::get_if<PointerButton>(&event);
  if (button == nullptr) return EventResult::unhandled();
  if (button->button == MouseButton::middle) {
    const std::optional<std::size_t> tab = tab_at(button->position);
    if (button->pressed) {
      middle_pressed_ = tab;
      return tab.has_value() ? EventResult::consumed_event()
                             : EventResult::unhandled();
    }
    const bool same = tab.has_value() && tab == middle_pressed_;
    middle_pressed_.reset();
    if (!same) return EventResult::unhandled();
    if (tabs_[*tab].closable) emit(TabStripEventKind::close, *tab);
    context.request_render();
    return EventResult::consumed_event();
  }
  if (button->button != MouseButton::left) return EventResult::unhandled();
  if (!button->pressed) {
    if (!pressed_.has_value()) return EventResult::unhandled();
    const std::size_t index = *pressed_;
    if (pressed_close_ && contains(close_rect(index), button->position)) {
      emit(TabStripEventKind::close, index);
    } else if (dragging_) {
      const std::size_t to = drop_index_for(button->position.x);
      if (to != index) emit(TabStripEventKind::reorder, index, to);
    }
    pressed_.reset();
    pressed_close_ = false;
    dragging_ = false;
    if (strip_id_.value != 0) context.release_pointer(strip_id_);
    context.request_render();
    return EventResult::consumed_event();
  }
  const std::optional<std::size_t> tab = tab_at(button->position);
  if (!tab.has_value()) return EventResult::unhandled();
  pressed_ = tab;
  press_x_ = button->position.x;
  drag_x_ = press_x_;
  pressed_close_ =
      tabs_[*tab].closable && contains(close_rect(*tab), button->position);
  if (!pressed_close_ && selected_ != tabs_[*tab].id) {
    emit(TabStripEventKind::select, *tab);
  }
  if (strip_id_.value != 0) {
    context.capture_pointer(strip_id_);
    context.request_keyboard_focus(strip_id_);
  }
  context.request_render();
  return EventResult::consumed_event();
}

} // namespace cgpui
