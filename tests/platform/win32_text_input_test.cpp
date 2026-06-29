#include "cgpui/platform/platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdint>
#include <string>
#include <variant>

namespace {

constexpr std::uint32_t expected_key = 'A';

void set_virtual_key_down(int virtual_key, bool down) {
  BYTE keyboard_state[256]{};
  GetKeyboardState(keyboard_state);
  keyboard_state[virtual_key] = down ? 0x80 : 0x00;
  SetKeyboardState(keyboard_state);
}

} // namespace

int main() {
  auto app = cgpui::create_platform_application();
  if (!app) {
    return 2;
  }

  bool key_pressed_with_shift = false;
  bool text_received = false;
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{
          .title = "CGPUI Win32 Text Input Test",
          .size = cgpui::Size{320.0F, 240.0F}},
      [&](const cgpui::PlatformEvent& event) {
        if (const auto* key = std::get_if<cgpui::KeyboardKey>(&event);
            key != nullptr && key->key_code == expected_key &&
            key->action == cgpui::KeyAction::pressed) {
          key_pressed_with_shift = key->modifiers.shift &&
              !key->modifiers.control && !key->modifiers.alt &&
              !key->modifiers.super;
        }
        if (const auto* text = std::get_if<cgpui::TextInput>(&event);
            text != nullptr && text->text == "A") {
          text_received = text->modifiers.shift &&
              !text->modifiers.control && !text->modifiers.alt &&
              !text->modifiers.super;
        }
      });
  if (!window) {
    return 3;
  }

  const auto surface =
      std::get<cgpui::Win32SurfaceHandle>((*window)->native_surface());
  auto* hwnd = static_cast<HWND>(surface.hwnd);
  if (hwnd == nullptr) {
    return 4;
  }

  set_virtual_key_down(VK_SHIFT, true);
  SendMessageW(hwnd, WM_KEYDOWN, expected_key, 0);
  SendMessageW(hwnd, WM_CHAR, L'A', 0);
  set_virtual_key_down(VK_SHIFT, false);

  if (!key_pressed_with_shift) {
    return 5;
  }
  if (!text_received) {
    return 6;
  }

  return 0;
}
