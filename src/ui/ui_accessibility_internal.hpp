#pragma once

#include "cgpui/ui/runtime.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>

namespace cgpui {
namespace {

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
    case AccessibilityRole::image:
      return PlatformAccessibilityRole::image;
    case AccessibilityRole::checkbox:
      return PlatformAccessibilityRole::checkbox;
    case AccessibilityRole::radio:
      return PlatformAccessibilityRole::radio;
    case AccessibilityRole::switch_control:
      return PlatformAccessibilityRole::switch_control;
    case AccessibilityRole::slider:
      return PlatformAccessibilityRole::slider;
    case AccessibilityRole::list_item:
      return PlatformAccessibilityRole::list_item;
    case AccessibilityRole::menu_item:
      return PlatformAccessibilityRole::menu_item;
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
        .value = node.value,
        .patterns = PlatformAccessibilityPatternState{
            .invokable = node.patterns.invokable,
            .value_settable = node.patterns.value_settable,
            .toggled = node.patterns.toggled,
            .range = node.patterns.range.has_value()
                ? std::optional<PlatformAccessibilityRangeValue>{
                      PlatformAccessibilityRangeValue{
                          .value = node.patterns.range->value,
                          .minimum = node.patterns.range->minimum,
                          .maximum = node.patterns.range->maximum,
                          .small_change = node.patterns.range->small_change,
                          .large_change = node.patterns.range->large_change,
                      }}
                : std::nullopt,
        },
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

} // namespace

} // namespace cgpui
