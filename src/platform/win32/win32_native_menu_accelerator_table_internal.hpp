#pragma once

#include "cgpui/platform/platform_native_menu.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstddef>
#include <optional>

namespace cgpui {

inline constexpr WORD win32_native_menu_first_command_id = 0x1000U;

class Win32NativeMenuAcceleratorTable {
 public:
  Win32NativeMenuAcceleratorTable() = default;
  ~Win32NativeMenuAcceleratorTable();

  Win32NativeMenuAcceleratorTable(
      const Win32NativeMenuAcceleratorTable&) = delete;
  Win32NativeMenuAcceleratorTable& operator=(
      const Win32NativeMenuAcceleratorTable&) = delete;
  Win32NativeMenuAcceleratorTable(
      Win32NativeMenuAcceleratorTable&& other) noexcept;
  Win32NativeMenuAcceleratorTable& operator=(
      Win32NativeMenuAcceleratorTable&& other) noexcept;

  [[nodiscard]] static std::optional<Win32NativeMenuAcceleratorTable> build(
      const NativeMenuModel& model);
  [[nodiscard]] bool translate(MSG& message) const;
  [[nodiscard]] std::size_t registered_count() const;

 private:
  Win32NativeMenuAcceleratorTable(HACCEL table, std::size_t registered_count);
  HACCEL table_ = nullptr;
  std::size_t registered_count_ = 0;
};

} // namespace cgpui
