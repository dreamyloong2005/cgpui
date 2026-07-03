#pragma once

#include "cgpui/platform/platform.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace cgpui {

struct Win32UiaProviderNode {
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

class Win32UiaAccessibilityAdapter {
 public:
  void update(PlatformAccessibilityTreeUpdate update);

  [[nodiscard]] std::uint64_t root_element_id() const;
  [[nodiscard]] std::size_t node_count() const;
  [[nodiscard]] std::size_t focused_node_count() const;
  [[nodiscard]] std::size_t text_input_node_count() const;
  [[nodiscard]] const std::vector<Win32UiaProviderNode>& uia_provider_nodes()
      const;
  [[nodiscard]] const std::vector<PlatformAccessibilityLiveUpdate>&
  last_live_updates() const;

 private:
  PlatformAccessibilityTreeUpdate last_update_;
  std::vector<Win32UiaProviderNode> provider_nodes_;
  std::vector<PlatformAccessibilityLiveUpdate> live_updates_;
  std::uint64_t root_element_id_ = 0;
  std::size_t node_count_ = 0;
  std::size_t focused_node_count_ = 0;
  std::size_t text_input_node_count_ = 0;
};

} // namespace cgpui
