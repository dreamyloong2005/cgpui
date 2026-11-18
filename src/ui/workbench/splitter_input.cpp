#include "cgpui/ui/splitter.hpp"

#include "cgpui/ui/runtime_context.hpp"
#include "workbench_internal.hpp"

#include <algorithm>
#include <variant>

namespace cgpui {

void Splitter::store_fractions_from_sizes() {
  float collapsed_share = 0.0F;
  float expanded_size = 0.0F;
  for (std::size_t index = 0; index < panes_.size(); ++index) {
    if (collapsed_[index]) {
      collapsed_share += fractions_[index];
    } else {
      expanded_size += sizes_[index];
    }
  }
  if (expanded_size <= 0.0F) return;
  const float expanded_share = std::max(0.0F, 1.0F - collapsed_share);
  for (std::size_t index = 0; index < panes_.size(); ++index) {
    if (!collapsed_[index]) {
      fractions_[index] = sizes_[index] / expanded_size * expanded_share;
    }
  }
}

void Splitter::notify(
    std::size_t divider,
    SplitterChangeSource source,
    bool finished) {
  if (!change_handler_) return;
  change_handler_(SplitterChange{
      .fractions = fractions_,
      .divider = divider,
      .source = source,
      .finished = finished,
  });
}

bool Splitter::move_divider(
    std::size_t divider,
    float delta,
    SplitterChangeSource source,
    bool finished) {
  if (divider + 1 >= panes_.size()) return false;
  const std::size_t a = divider;
  const std::size_t b = divider + 1;
  const SplitterPaneOptions& pa = panes_[a];
  const SplitterPaneOptions& pb = panes_[b];
  const float combined = sizes_[a] + sizes_[b];
  const float desired_a = sizes_[a] + delta;
  const float desired_b = combined - desired_a;
  float next_a = sizes_[a];
  bool collapse_a = false;
  bool collapse_b = false;
  // A collapsible pane dragged below half its minimum collapses; dragging
  // it back past that point expands it again.
  if (pa.collapsible && desired_a < pa.min_size * 0.5F && pb.max_size >= combined) {
    collapse_a = true;
    next_a = 0.0F;
  } else if (pb.collapsible && desired_b < pb.min_size * 0.5F &&
             pa.max_size >= combined) {
    collapse_b = true;
    next_a = combined;
  } else {
    const float low = std::max(pa.min_size, combined - pb.max_size);
    const float high = std::min(pa.max_size, combined - pb.min_size);
    if (low > high) return false;
    next_a = std::clamp(desired_a, low, high);
  }
  if (next_a == sizes_[a] && collapse_a == collapsed_[a] &&
      collapse_b == collapsed_[b]) {
    return false;
  }
  collapsed_[a] = collapse_a;
  collapsed_[b] = collapse_b;
  sizes_[a] = next_a;
  sizes_[b] = combined - next_a;
  store_fractions_from_sizes();
  layout(bounds_, scale_);
  notify(divider, source, finished);
  return true;
}

EventResult Splitter::handle_event(
    const PlatformEvent& event,
    const WindowRuntimeContext& context) {
  const std::optional<ElementId> owner =
      context.input_state().keyboard_focus_element_owner;
  focused_.reset();
  for (std::size_t index = 0; index < divider_ids_.size(); ++index) {
    if (owner.has_value() && divider_ids_[index].value != 0 &&
        *owner == divider_ids_[index]) {
      focused_ = index;
    }
  }
  if (const auto* key = std::get_if<KeyboardKey>(&event)) {
    return handle_key(*key, context);
  }
  return handle_pointer(event, context);
}

EventResult Splitter::handle_pointer(
    const PlatformEvent& event,
    const WindowRuntimeContext& context) {
  if (const auto* moved = std::get_if<PointerMoved>(&event)) {
    if (dragging_.has_value()) {
      const float line = main_of(divider_rect(*dragging_).origin);
      const float target = main_of(moved->position) - drag_grab_offset_;
      (void)move_divider(*dragging_, target - line,
                         SplitterChangeSource::pointer, false);
      context.request_render();
      return EventResult::consumed_event();
    }
    const std::optional<std::size_t> hovered = divider_at(moved->position);
    if (hovered != hovered_) {
      hovered_ = hovered;
      context.request_paint();
    }
    return EventResult::unhandled();
  }
  if (std::holds_alternative<PointerExited>(event) && hovered_.has_value() &&
      !dragging_.has_value()) {
    hovered_.reset();
    context.request_paint();
    return EventResult::unhandled();
  }
  const bool capture_lost =
      std::holds_alternative<PointerCaptureChanged>(event) &&
      !std::get<PointerCaptureChanged>(event).captured;
  const auto* button = std::get_if<PointerButton>(&event);
  if (dragging_.has_value() &&
      (capture_lost || (button != nullptr && button->button == MouseButton::left &&
                        !button->pressed))) {
    const std::size_t divider = *dragging_;
    dragging_.reset();
    context.release_pointer(divider_ids_[divider]);
    notify(divider, SplitterChangeSource::pointer, true);
    context.request_render();
    return EventResult::consumed_event();
  }
  if (button == nullptr || button->button != MouseButton::left ||
      !button->pressed) {
    return EventResult::unhandled();
  }
  const std::optional<std::size_t> divider = divider_at(button->position);
  if (!divider.has_value()) return EventResult::unhandled();
  if (button->click_count >= 2) {
    restore_default_fractions();
    notify(*divider, SplitterChangeSource::restore, true);
    context.request_render();
    return EventResult::consumed_event();
  }
  dragging_ = divider;
  drag_grab_offset_ =
      main_of(button->position) - main_of(divider_rect(*divider).origin);
  if (divider_ids_[*divider].value != 0) {
    context.capture_pointer(divider_ids_[*divider]);
    context.request_keyboard_focus(divider_ids_[*divider]);
  }
  context.request_render();
  return EventResult::consumed_event();
}

EventResult Splitter::handle_key(
    const KeyboardKey& key,
    const WindowRuntimeContext& context) {
  using namespace workbench_detail;
  if (!focused_.has_value() || key.action != KeyAction::pressed) {
    return EventResult::unhandled();
  }
  const std::size_t divider = *focused_;
  const float step = key.modifiers.shift ? options_.keyboard_large_step
                                         : options_.keyboard_step;
  constexpr float far = 1.0e6F;
  bool changed = false;
  switch (key.key_code) {
    case key_left:
    case key_up:
      changed = move_divider(divider, -step, SplitterChangeSource::keyboard);
      break;
    case key_right:
    case key_down:
      changed = move_divider(divider, step, SplitterChangeSource::keyboard);
      break;
    case key_home:
      changed = move_divider(divider, -far, SplitterChangeSource::keyboard);
      break;
    case key_end:
      changed = move_divider(divider, far, SplitterChangeSource::keyboard);
      break;
    case key_enter: {
      // Toggles the adjacent collapsible pane (the leading one first).
      const std::size_t pane =
          panes_[divider].collapsible ? divider : divider + 1;
      if (!panes_[pane].collapsible) return EventResult::unhandled();
      set_collapsed(pane, !collapsed_[pane]);
      notify(divider, SplitterChangeSource::collapse, true);
      changed = true;
      break;
    }
    default:
      return EventResult::unhandled();
  }
  if (changed) context.request_render();
  return EventResult::consumed_event();
}

} // namespace cgpui
