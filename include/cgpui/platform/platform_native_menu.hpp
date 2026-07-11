#pragma once

#include "cgpui/core/event_keyboard.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace cgpui {

enum class NativeMenuItemKind {
  command,
  separator,
  submenu,
};

struct NativeMenuAccelerator {
  std::uint32_t key_code = 0;
  KeyAction action = KeyAction::pressed;
  KeyboardModifiers modifiers;
};

struct NativeMenuItem {
  NativeMenuItemKind kind = NativeMenuItemKind::command;
  std::string title;
  std::string action_name;
  std::optional<NativeMenuAccelerator> accelerator;
  bool enabled = true;
  bool checked = false;
  bool radio = false;
  std::vector<NativeMenuItem> children;
};

struct NativeMenuModel {
  std::vector<NativeMenuItem> items;
};

struct PlatformMenuInstallationResult {
  bool supported = false;
  std::string backend;
  std::size_t menu_count = 0;
  std::size_t item_count = 0;
  std::size_t accelerator_count = 0;
};

struct NativeMenuInstallation {
  NativeMenuModel model;
  PlatformMenuInstallationResult platform;
};

[[nodiscard]] std::size_t native_menu_item_count(
    const NativeMenuModel& model);
[[nodiscard]] std::size_t native_menu_accelerator_count(
    const NativeMenuModel& model);

} // namespace cgpui
