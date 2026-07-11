#pragma once

#include "cgpui/platform/platform_native_menu.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <optional>

namespace cgpui {

class Win32NativeMenuTree {
 public:
  Win32NativeMenuTree() = default;
  ~Win32NativeMenuTree();

  Win32NativeMenuTree(const Win32NativeMenuTree&) = delete;
  Win32NativeMenuTree& operator=(const Win32NativeMenuTree&) = delete;
  Win32NativeMenuTree(Win32NativeMenuTree&& other) noexcept;
  Win32NativeMenuTree& operator=(Win32NativeMenuTree&& other) noexcept;

  [[nodiscard]] static std::optional<Win32NativeMenuTree> build(
      const NativeMenuModel& model);
  [[nodiscard]] HMENU root() const;

 private:
  explicit Win32NativeMenuTree(HMENU root);
  HMENU root_ = nullptr;
};

[[nodiscard]] bool win32_apply_native_menu(HWND hwnd, HMENU menu);

} // namespace cgpui
