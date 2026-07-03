#pragma once

#include "cgpui/ui/runtime.hpp"

#include <algorithm>
#include <expected>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace cgpui {
namespace {

inline EventKind event_kind_for(const PlatformEvent& event) {
  if (std::holds_alternative<WindowActivated>(event)) {
    return EventKind::window_activated;
  }
  if (std::holds_alternative<WindowFocused>(event)) {
    return EventKind::window_focused;
  }
  if (std::holds_alternative<WindowMinimized>(event)) {
    return EventKind::window_minimized;
  }
  if (std::holds_alternative<WindowRestored>(event)) {
    return EventKind::window_restored;
  }
  if (std::holds_alternative<WindowCloseRequested>(event)) {
    return EventKind::window_close_requested;
  }
  if (std::holds_alternative<PointerMoved>(event)) {
    return EventKind::pointer_moved;
  }
  if (std::holds_alternative<PointerButton>(event)) {
    return EventKind::pointer_button;
  }
  if (std::holds_alternative<PointerScrolled>(event)) {
    return EventKind::pointer_scrolled;
  }
  if (std::holds_alternative<DragEntered>(event)) {
    return EventKind::drag_entered;
  }
  if (std::holds_alternative<DragUpdated>(event)) {
    return EventKind::drag_updated;
  }
  if (std::holds_alternative<DragDropped>(event)) {
    return EventKind::drag_dropped;
  }
  if (std::holds_alternative<DragExited>(event)) {
    return EventKind::drag_exited;
  }
  if (std::holds_alternative<KeyboardKey>(event)) {
    return EventKind::keyboard_key;
  }
  if (std::holds_alternative<TextInput>(event)) {
    return EventKind::text_input;
  }
  if (std::holds_alternative<ImeComposition>(event)) {
    return EventKind::ime_composition;
  }
  if (std::holds_alternative<ImeDeleteSurroundingText>(event)) {
    return EventKind::ime_delete_surrounding_text;
  }
  return EventKind::unknown;
}

inline std::size_t drag_drop_payload_value_count(const DragDropPayload& payload) {
  switch (payload.kind) {
    case DragDropPayloadKind::text:
      return payload.text.empty() ? 0 : 1;
    case DragDropPayloadKind::files:
      return payload.files.size();
    case DragDropPayloadKind::none:
      return 0;
  }
  return 0;
}

inline std::size_t drag_drop_payload_value_count(const PlatformEvent& event) {
  if (const auto* drag_entered = std::get_if<DragEntered>(&event);
      drag_entered != nullptr) {
    return drag_drop_payload_value_count(drag_entered->payload);
  }
  if (const auto* drag_updated = std::get_if<DragUpdated>(&event);
      drag_updated != nullptr) {
    return drag_drop_payload_value_count(drag_updated->payload);
  }
  if (const auto* drag_dropped = std::get_if<DragDropped>(&event);
      drag_dropped != nullptr) {
    return drag_drop_payload_value_count(drag_dropped->payload);
  }
  if (const auto* drag_exited = std::get_if<DragExited>(&event);
      drag_exited != nullptr) {
    return drag_drop_payload_value_count(drag_exited->payload);
  }
  return 0;
}

inline std::string drag_drop_operation_for(const PlatformEvent& event) {
  if (std::holds_alternative<DragEntered>(event)) {
    return "drag-entered";
  }
  if (std::holds_alternative<DragUpdated>(event)) {
    return "drag-updated";
  }
  if (std::holds_alternative<DragDropped>(event)) {
    return "drag-dropped";
  }
  if (std::holds_alternative<DragExited>(event)) {
    return "drag-exited";
  }
  return "drag-drop";
}

inline PlatformAccessibilityRole platform_accessibility_role(
    AccessibilityRole role) {
  switch (role) {
    case AccessibilityRole::label:
      return PlatformAccessibilityRole::label;
    case AccessibilityRole::button:
      return PlatformAccessibilityRole::button;
    case AccessibilityRole::text:
      return PlatformAccessibilityRole::text;
    case AccessibilityRole::text_input:
      return PlatformAccessibilityRole::text_input;
    case AccessibilityRole::generic:
      return PlatformAccessibilityRole::generic;
  }
  return PlatformAccessibilityRole::generic;
}

inline PlatformAccessibilityTreeUpdate platform_accessibility_update_from(
    const AccessibilityTreeSnapshot& snapshot) {
  PlatformAccessibilityTreeUpdate update{
      .root_element_id = snapshot.root_element_id.value,
      .node_count = snapshot.nodes.size(),
  };
  update.nodes.reserve(snapshot.nodes.size());
  for (const AccessibilityNode& node : snapshot.nodes) {
    if (node.focused) {
      update.focused_node_count += 1;
    }
    std::optional<std::uint64_t> parent_id;
    if (node.parent_element_id.has_value()) {
      parent_id = node.parent_element_id->value;
    }
    update.nodes.push_back(PlatformAccessibilityNodeUpdate{
        .element_id = node.element_id.value,
        .parent_element_id = parent_id,
        .role = platform_accessibility_role(node.role),
        .name = node.name,
        .text = node.text,
        .value = node.role == AccessibilityRole::text_input ? node.text
                                                            : std::string{},
        .enabled = node.enabled,
        .focusable = node.focusable,
        .focused = node.focused,
        .bounds = node.bounds,
        .child_count = node.children.size(),
    });
  }
  return update;
}

inline void append_platform_accessibility_live_updates(
    PlatformAccessibilityTreeUpdate& update,
    const PlatformAccessibilityTreeUpdate& previous) {
  std::unordered_map<std::uint64_t, const PlatformAccessibilityNodeUpdate*>
      previous_nodes;
  previous_nodes.reserve(previous.nodes.size());
  for (const PlatformAccessibilityNodeUpdate& node : previous.nodes) {
    previous_nodes.emplace(node.element_id, &node);
  }

  for (const PlatformAccessibilityNodeUpdate& node : update.nodes) {
    const auto previous_node = previous_nodes.find(node.element_id);
    if (previous_node == previous_nodes.end()) {
      continue;
    }
    const PlatformAccessibilityNodeUpdate& before = *previous_node->second;
    if (before.value != node.value) {
      update.live_updates.push_back(PlatformAccessibilityLiveUpdate{
          .kind = PlatformAccessibilityLiveUpdateKind::value_changed,
          .element_id = node.element_id,
          .value = node.value,
          .text = node.text,
          .focused = node.focused,
      });
    }
    if (before.text != node.text) {
      update.live_updates.push_back(PlatformAccessibilityLiveUpdate{
          .kind = PlatformAccessibilityLiveUpdateKind::text_changed,
          .element_id = node.element_id,
          .value = node.value,
          .text = node.text,
          .focused = node.focused,
      });
    }
    if (before.focused != node.focused) {
      update.live_updates.push_back(PlatformAccessibilityLiveUpdate{
          .kind = PlatformAccessibilityLiveUpdateKind::focus_changed,
          .element_id = node.element_id,
          .value = node.value,
          .text = node.text,
          .focused = node.focused,
      });
    }
  }
}

inline std::optional<Point> pointer_position_for(const PlatformEvent& event) {
  if (const auto* moved = std::get_if<PointerMoved>(&event);
      moved != nullptr) {
    return moved->position;
  }
  if (const auto* button = std::get_if<PointerButton>(&event);
      button != nullptr) {
    return button->position;
  }
  if (const auto* scrolled = std::get_if<PointerScrolled>(&event);
      scrolled != nullptr) {
    return scrolled->position;
  }
  if (const auto* drag_entered = std::get_if<DragEntered>(&event);
      drag_entered != nullptr) {
    return drag_entered->position;
  }
  if (const auto* drag_updated = std::get_if<DragUpdated>(&event);
      drag_updated != nullptr) {
    return drag_updated->position;
  }
  if (const auto* drag_dropped = std::get_if<DragDropped>(&event);
      drag_dropped != nullptr) {
    return drag_dropped->position;
  }
  if (const auto* drag_exited = std::get_if<DragExited>(&event);
      drag_exited != nullptr) {
    return drag_exited->position;
  }
  return {};
}

inline bool is_keyboard_routed_event(const PlatformEvent& event) {
  return std::holds_alternative<KeyboardKey>(event) ||
         std::holds_alternative<TextInput>(event) ||
         std::holds_alternative<ImeComposition>(event) ||
         std::holds_alternative<ImeDeleteSurroundingText>(event);
}

inline bool is_focus_activation_event(const PlatformEvent& event) {
  const auto* button = std::get_if<PointerButton>(&event);
  return button != nullptr && button->button == MouseButton::left &&
         button->pressed;
}

inline bool is_focus_traversal_key(const KeyboardKey& key) {
  constexpr std::uint32_t tab_key_code = 9;
  return key.key_code == tab_key_code && key.action == KeyAction::pressed &&
         !key.modifiers.control && !key.modifiers.alt && !key.modifiers.super;
}

inline bool modifiers_equal(KeyboardModifiers lhs, KeyboardModifiers rhs) {
  return lhs.shift == rhs.shift && lhs.control == rhs.control &&
         lhs.alt == rhs.alt && lhs.super == rhs.super;
}

inline bool rect_equal(Rect lhs, Rect rhs) {
  return lhs.origin.x == rhs.origin.x && lhs.origin.y == rhs.origin.y &&
         lhs.size.width == rhs.size.width && lhs.size.height == rhs.size.height;
}

inline bool ime_text_input_placement_equal(
    const std::optional<ImeTextInputPlacement>& lhs,
    const std::optional<ImeTextInputPlacement>& rhs) {
  if (lhs.has_value() != rhs.has_value()) {
    return false;
  }
  if (!lhs.has_value()) {
    return true;
  }
  return lhs->byte_offset == rhs->byte_offset &&
         rect_equal(lhs->rect, rhs->rect);
}

inline bool is_valid_pointer_capture_owner(const PointerCaptureOwner& owner) {
  if (const ViewId* view_id = owner.view_id(); view_id != nullptr) {
    return view_id->value != 0;
  }
  if (const ElementId* element_id = owner.element_id();
      element_id != nullptr) {
    return element_id->value != 0;
  }
  return false;
}

inline void apply_pointer_capture_owner_to_route(
    const PointerCaptureOwner& owner,
    EventRoute& route) {
  if (const ViewId* view_id = owner.view_id(); view_id != nullptr) {
    route.target_view_id = *view_id;
    route.target_element_id.reset();
    return;
  }
  if (const ElementId* element_id = owner.element_id();
      element_id != nullptr) {
    route.target_element_id = *element_id;
  }
}

inline PaintMetadata compose_paint_metadata(
    PaintMetadata parent,
    PaintMetadata child) {
  return PaintMetadata{
      .opacity = parent.opacity * child.opacity,
      .transform = compose(parent.transform, child.transform),
  };
}

inline PaintMetadata paint_metadata_for_style(const Style& style) {
  return PaintMetadata{
      .opacity = style.opacity,
      .transform = style.transform,
  };
}

inline std::optional<Rect> current_clip_rect_for(const std::vector<Rect>& clips) {
  return clips.empty() ? std::optional<Rect>{}
                       : std::optional<Rect>{clips.back()};
}

inline RendererClipStackRecord clip_stack_record_for(const std::vector<Rect>& clips) {
  return renderer_clip_stack_record(
      std::span<const Rect>{clips.data(), clips.size()});
}

inline RendererCompositionStackRecord composition_stack_record_for(
    const std::vector<PaintMetadata>& entries) {
  return renderer_composition_stack_record(
      std::span<const PaintMetadata>{entries.data(), entries.size()});
}

inline void record_clip_stack_statistics(
    FrameStatistics& statistics,
    const PaintCommand& command) {
  if (command.clip_stack.empty()) {
    return;
  }
  statistics.clip_stack_command_count += 1;
  statistics.max_clip_stack_depth = std::max(
      statistics.max_clip_stack_depth,
      command.clip_stack.full_depth);
}

inline void record_composition_stack_statistics(
    FrameStatistics& statistics,
    const PaintCommand& command) {
  if (command.composition_stack.empty()) {
    return;
  }
  statistics.composition_stack_command_count += 1;
  statistics.max_composition_stack_depth = std::max(
      statistics.max_composition_stack_depth,
      command.composition_stack.full_depth);
}

inline ElementId hit_test_runtime_element_root(
    const ElementTree* tree,
    const Element* root,
    Point point) {
  if (tree != nullptr) {
    return tree->hit_test_root(point);
  }
  return root == nullptr ? ElementId{} : root->hit_test(point);
}

inline void paint_styled_box_base(
    PaintList& paint_list,
    const std::optional<Rect>& bounds,
    const Style& style) {
  if (bounds.has_value() && style.background_color.has_value()) {
    const BorderRadii radius = style.border_radius;
    if (radius.top_left > 0.0F || radius.top_right > 0.0F ||
        radius.bottom_right > 0.0F || radius.bottom_left > 0.0F) {
      paint_list.fill_rounded_rect(*bounds, *style.background_color, radius);
    } else {
      paint_list.fill_rect(*bounds, *style.background_color);
    }
  }
  if (bounds.has_value() && style.border_color.has_value()) {
    const Rect rect = *bounds;
    const Color color = *style.border_color;
    const float top = style.border_width.top;
    const float right = style.border_width.right;
    const float bottom = style.border_width.bottom;
    const float left = style.border_width.left;
    const float vertical_side_height =
        std::max(0.0F, rect.size.height - top - bottom);

    if (top > 0.0F) {
      paint_list.fill_rect(
          Rect{
              .origin = rect.origin,
              .size = {.width = rect.size.width, .height = top},
          },
          color);
    }
    if (right > 0.0F && vertical_side_height > 0.0F) {
      paint_list.fill_rect(
          Rect{
              .origin =
                  {
                      .x = rect.origin.x + rect.size.width - right,
                      .y = rect.origin.y + top,
                  },
              .size = {.width = right, .height = vertical_side_height},
          },
          color);
    }
    if (bottom > 0.0F) {
      paint_list.fill_rect(
          Rect{
              .origin =
                  {
                      .x = rect.origin.x,
                      .y = rect.origin.y + rect.size.height - bottom,
                  },
              .size = {.width = rect.size.width, .height = bottom},
          },
          color);
    }
    if (left > 0.0F && vertical_side_height > 0.0F) {
      paint_list.fill_rect(
          Rect{
              .origin = {.x = rect.origin.x, .y = rect.origin.y + top},
              .size = {.width = left, .height = vertical_side_height},
          },
          color);
    }
  }
}


} // namespace

} // namespace cgpui
