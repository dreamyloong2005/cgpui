#include "cgpui/platform/platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdint>
#include <variant>
#include <vector>

namespace {

LPARAM key_lparam(
    std::uint16_t repeat_count,
    std::uint8_t scan_code,
    bool extended = false,
    bool context = false,
    bool previous = false,
    bool transition = false) {
  std::uintptr_t value = repeat_count;
  value |= static_cast<std::uintptr_t>(scan_code) << 16U;
  value |= static_cast<std::uintptr_t>(extended) << 24U;
  value |= static_cast<std::uintptr_t>(context) << 29U;
  value |= static_cast<std::uintptr_t>(previous) << 30U;
  value |= static_cast<std::uintptr_t>(transition) << 31U;
  return static_cast<LPARAM>(value);
}

} // namespace

int main() {
  auto app = cgpui::create_platform_application();
  if (!app) {
    return 1;
  }
  std::vector<cgpui::KeyboardKey> events;
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{.title = "CGPUI Win32 Keyboard Key Test"},
      [&](const cgpui::PlatformEvent& event) {
        if (const auto* key = std::get_if<cgpui::KeyboardKey>(&event)) {
          events.push_back(*key);
        }
      });
  if (!window) {
    return 2;
  }
  const auto surface =
      std::get<cgpui::Win32SurfaceHandle>((*window)->native_surface());
  const auto hwnd = static_cast<HWND>(surface.hwnd);
  if (hwnd == nullptr) {
    return 3;
  }
  SendMessageW(hwnd, WM_KEYDOWN, 'A', key_lparam(1, 0x1E));
  SendMessageW(hwnd, WM_KEYDOWN, 'A', key_lparam(3, 0x1E, false, false, true));
  SendMessageW(hwnd, WM_KEYUP, 'A', key_lparam(1, 0x1E, false, false, true, true));
  SendMessageW(
      hwnd,
      WM_SYSKEYDOWN,
      VK_INSERT,
      key_lparam(1, 0x52, true));
  SendMessageW(
      hwnd,
      WM_SYSKEYUP,
      VK_INSERT,
      key_lparam(1, 0x52, true, false, true, true));
  if (events.size() != 5) {
    return 4;
  }
  if (events[0].key_code != 'A' ||
      events[0].action != cgpui::KeyAction::pressed ||
      events[0].scan_code != 0x1E || events[0].repeat_count != 1 ||
      events[0].repeated || events[0].extended || events[0].system) {
    return 5;
  }
  if (events[1].repeat_count != 3 || !events[1].repeated) {
    return 6;
  }
  if (events[2].action != cgpui::KeyAction::released || events[2].repeated) {
    return 7;
  }
  if (events[3].action != cgpui::KeyAction::pressed ||
      events[3].scan_code != 0x52 || !events[3].extended ||
      !events[3].system || events[3].repeated) {
    return 8;
  }
  if (events[4].action != cgpui::KeyAction::released ||
      !events[4].extended || !events[4].system || events[4].repeated) {
    return 9;
  }
  return 0;
}
