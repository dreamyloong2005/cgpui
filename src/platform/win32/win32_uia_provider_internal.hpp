#pragma once

#include "cgpui/platform/platform_accessibility.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ole2.h>
#include <UIAutomationCore.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>

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

class Win32UiaProviderTree;
using Win32UiaProviderTreeHandle = std::shared_ptr<Win32UiaProviderTree>;

IRawElementProviderSimple* create_win32_uia_provider(
    Win32UiaProviderTreeHandle tree,
    Win32UiaProviderNode node,
    bool is_root);

} // namespace cgpui
