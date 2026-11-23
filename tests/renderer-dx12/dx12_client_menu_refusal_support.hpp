#pragma once

#include "dx12_ordinary_pixels_support.hpp"

namespace cgpui_test {
inline cgpui::NativeMenuModel refusal_native_menu() {
  return {.items = {{.kind = cgpui::NativeMenuItemKind::submenu, .title = "&File",
      .children = {{.title = "&Open", .action_name = "file.open",
          .accelerator = cgpui::NativeMenuAccelerator{.key_code = VK_F12}}}}}};
}

// Snapshot identity and geometry, not a claim that an opaque client menu has
// correct layout. Refusal must preserve it rather than silently hide/replace it.
struct RefusalNativeSnapshot {
  HWND hwnd = nullptr;
  HMENU menu = nullptr, submenu = nullptr;
  RECT window{}, client{};
  POINT origin{};
  LONG_PTR style = 0, extended = 0;
  UINT command = UINT(-1);
  bool capture(const cgpui::PlatformWindow& target) {
    hwnd = ordinary_hwnd(target);
    menu = GetMenu(hwnd);
    submenu = menu ? GetSubMenu(menu, 0) : nullptr;
    command = submenu ? GetMenuItemID(submenu, 0) : UINT(-1);
    style = GetWindowLongPtrW(hwnd, GWL_STYLE);
    extended = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    return hwnd && GetWindowRect(hwnd, &window) && GetClientRect(hwnd, &client) &&
        ClientToScreen(hwnd, &origin) &&
        (!menu || (IsMenu(menu) && submenu && GetMenuItemCount(menu) == 1 &&
                   GetMenuItemCount(submenu) == 1 && command != UINT(-1)));
  }
  bool same(const cgpui::PlatformWindow& target) const {
    RefusalNativeSnapshot other;
    return other.capture(target) && hwnd == other.hwnd && menu == other.menu && submenu == other.submenu &&
        command == other.command && style == other.style && extended == other.extended &&
        EqualRect(&window, &other.window) && EqualRect(&client, &other.client) &&
        origin.x == other.origin.x && origin.y == other.origin.y;
  }
};

inline bool client_menu_alpha_refused(const cgpui::PlatformWindow& window) {
  RefusalNativeSnapshot before;
  if (!before.capture(window) || !before.menu) return false;
  auto renderer = ordinary_renderer(window, true);
  return !renderer && renderer.error().code == cgpui::ErrorCode::unsupported_platform && before.same(window);
}

inline bool refusal_paint(cgpui::Renderer& renderer, const cgpui::PlatformWindow& window, bool blue) {
  const auto state = window.state();
  return renderer.resize(state.framebuffer_size, state.scale).has_value() &&
      ordinary_composition(renderer.surface_composition(), true) &&
      ordinary_draw_bands(renderer, window, blue) && ordinary_desktop_bands(window, blue, true);
}

struct RefusalMenuCommands {
  unsigned accelerator_count = 0;
  std::string last_action;
  void observe(const cgpui::PlatformEvent& event) {
    if (const auto* command = std::get_if<cgpui::NativeMenuCommand>(&event);
        command && command->source == cgpui::NativeMenuCommandSource::accelerator) {
      ++accelerator_count;
      last_action = command->action_name;
    }
  }
};

inline void CALLBACK stop_refusal_accelerator_loop(HWND, UINT, UINT_PTR, DWORD) { PostQuitMessage(0); }
inline bool refusal_accelerator(cgpui::PlatformApplication& app, HWND hwnd) {
  // Use the public app's real TranslateAccelerator path. No synthesized
  // WM_COMMAND or global keyboard state; F12 has no modifier requirement.
  const UINT_PTR timer = SetTimer(nullptr, 0, 100, stop_refusal_accelerator_loop);
  if (!timer) return false;
  const bool posted = PostMessageW(hwnd, WM_KEYDOWN, VK_F12, 0) &&
      PostMessageW(hwnd, WM_KEYUP, VK_F12, 0);
  const int result = posted ? app.run() : -1;
  KillTimer(nullptr, timer);
  return posted && result == 0;
}
} // namespace cgpui_test
