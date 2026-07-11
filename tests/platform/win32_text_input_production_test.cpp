#include "cgpui/platform/platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string>
#include <variant>
#include <vector>

int main() {
  auto app = cgpui::create_platform_application();
  if (!app) {
    return 1;
  }
  std::vector<cgpui::TextInput> events;
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{.title = "CGPUI Win32 Text Input Production"},
      [&](const cgpui::PlatformEvent& event) {
        if (const auto* text = std::get_if<cgpui::TextInput>(&event)) {
          events.push_back(*text);
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

  if (SendMessageW(hwnd, WM_UNICHAR, UNICODE_NOCHAR, 0) != TRUE) {
    return 4;
  }
  SendMessageW(hwnd, WM_CHAR, 0xD83D, 0);
  if (!events.empty()) {
    return 5;
  }
  SendMessageW(hwnd, WM_CHAR, 0xDE00, 0);
  if (events.size() != 1 ||
      events[0].text != std::string{"\xF0\x9F\x98\x80"} ||
      events[0].composed) {
    return 6;
  }

  SendMessageW(hwnd, WM_UNICHAR, 0x1F642, 0);
  if (events.size() != 2 ||
      events[1].text != std::string{"\xF0\x9F\x99\x82"} ||
      events[1].composed) {
    return 7;
  }
  SendMessageW(hwnd, WM_SYSDEADCHAR, L'`', 0);
  SendMessageW(hwnd, WM_SYSCHAR, L'x', 0);
  if (events.size() != 2) {
    return 8;
  }
  SendMessageW(hwnd, WM_CHAR, L'z', 0);
  if (events.size() != 3 || events[2].text != "z" || events[2].composed) {
    return 9;
  }

  SendMessageW(hwnd, WM_CHAR, 0xD83D, 0);
  SendMessageW(hwnd, WM_KILLFOCUS, 0, 0);
  SendMessageW(hwnd, WM_CHAR, 0xDE00, 0);
  SendMessageW(hwnd, WM_CHAR, VK_BACK, 0);
  SendMessageW(hwnd, WM_CHAR, 0x01, 0);
  SendMessageW(hwnd, WM_CHAR, 0x110000, 0);
  SendMessageW(hwnd, WM_UNICHAR, 0x110000, 0);
  if (events.size() != 3) {
    return 10;
  }
  return 0;
}
