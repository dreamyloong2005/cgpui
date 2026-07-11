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
  std::vector<cgpui::TextInput> text_events;
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{.title = "CGPUI Win32 Dead Key Test"},
      [&](const cgpui::PlatformEvent& event) {
        if (const auto* text = std::get_if<cgpui::TextInput>(&event)) {
          text_events.push_back(*text);
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
  SendMessageW(hwnd, WM_DEADCHAR, 0x00B4, 0);
  if (!text_events.empty()) {
    return 4;
  }
  SendMessageW(hwnd, WM_CHAR, 0x00E9, 0);
  SendMessageW(hwnd, WM_CHAR, L'x', 0);
  SendMessageW(hwnd, WM_SYSDEADCHAR, L'`', 0);
  SendMessageW(hwnd, WM_CHAR, 0x00E0, 0);
  SendMessageW(hwnd, WM_DEADCHAR, L'^', 0);
  SendMessageW(hwnd, WM_KILLFOCUS, 0, 0);
  SendMessageW(hwnd, WM_CHAR, L'z', 0);
  if (text_events.size() != 4) {
    return 5;
  }
  if (text_events[0].text != std::string{"\xC3\xA9"} ||
      !text_events[0].composed) {
    return 6;
  }
  if (text_events[1].text != "x" || text_events[1].composed) {
    return 7;
  }
  if (text_events[2].text != std::string{"\xC3\xA0"} ||
      !text_events[2].composed) {
    return 8;
  }
  if (text_events[3].text != "z" || text_events[3].composed) {
    return 9;
  }
  return 0;
}
