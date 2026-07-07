#pragma once

#include "cgpui/core/geometry.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace cgpui {

enum class PlatformAccessibilityRole {
  generic,
  label,
  button,
  text,
  text_input,
  checkbox,
  radio,
  switch_control,
};

struct PlatformAccessibilityNodeUpdate {
  std::uint64_t element_id = 0;
  std::optional<std::uint64_t> parent_element_id;
  PlatformAccessibilityRole role = PlatformAccessibilityRole::generic;
  std::string name;
  std::string text;
  std::string value;
  bool enabled = true;
  bool focusable = false;
  bool focused = false;
  std::optional<Rect> bounds;
  std::size_t child_count = 0;
};

enum class PlatformAccessibilityLiveUpdateKind {
  value_changed,
  text_changed,
  focus_changed,
};

struct PlatformAccessibilityLiveUpdate {
  PlatformAccessibilityLiveUpdateKind kind =
      PlatformAccessibilityLiveUpdateKind::value_changed;
  std::uint64_t element_id = 0;
  std::string value;
  std::string text;
  bool focused = false;
};

struct PlatformAccessibilityTreeUpdate {
  std::uint64_t root_element_id = 0;
  std::size_t node_count = 0;
  std::size_t focused_node_count = 0;
  std::vector<PlatformAccessibilityNodeUpdate> nodes;
  std::vector<PlatformAccessibilityLiveUpdate> live_updates;
};

} // namespace cgpui
