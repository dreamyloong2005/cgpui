#pragma once

#include "dx12_ordinary_menu_pixels_support.hpp"
#include "cgpui/platform/platform.hpp"
#include <optional>

namespace cgpui_test {

struct MenuCommands {
  unsigned commands = 0;
  std::optional<cgpui::NativeMenuCommand> last;
  void observe(const cgpui::PlatformEvent& event) {
    if (const auto* command = std::get_if<cgpui::NativeMenuCommand>(&event)) {
      ++commands;
      last = *command;
    }
  }
};

struct MenuLoopObservation {
  MenuPixels before, opened, after;
  MenuFocus before_focus, opened_focus, after_focus;
  bool ok = false;
};

struct OrdinaryMenuScript {
  HWND target = nullptr;
  HMENU submenu = nullptr;
  UINT command = 0;
  MenuLoopObservation* observation = nullptr;
  const char* label = nullptr;
  ULONGLONG deadline = 0;
  unsigned stable = 0;
  bool home_posted = false, selected = false, captured = false, timed_out = false, native_ok = true;
  bool cancel = false, second_escape_posted = false;
  MenuPixels previous;
  MenuFocus previous_focus;
};
inline thread_local OrdinaryMenuScript* ordinary_menu_script = nullptr;

inline HWND own_native_menu_window(HMENU expected) {
  struct Search { HMENU menu; HWND found = nullptr; } search{expected};
  EnumThreadWindows(GetCurrentThreadId(), [](HWND hwnd, LPARAM data) -> BOOL {
    auto& search = *reinterpret_cast<Search*>(data);
    wchar_t name[16]{};
    GetClassNameW(hwnd, name, 16);
    if (!own_menu_target(hwnd) || lstrcmpW(name, L"#32768") != 0 || !IsWindowVisible(hwnd)) return TRUE;
    if (reinterpret_cast<HMENU>(SendMessageW(hwnd, MN_GETHMENU, 0, 0)) != search.menu) return TRUE;
    search.found = hwnd;
    return FALSE;
  }, reinterpret_cast<LPARAM>(&search));
  return search.found;
}

inline void CALLBACK drive_ordinary_native_menu(HWND, UINT, UINT_PTR, DWORD) {
  auto* script = ordinary_menu_script;
  if (!script) return;
  if (GetTickCount64() >= script->deadline) {
    script->timed_out = true;
    EndMenu();
    return;
  }
  const HWND popup = own_native_menu_window(script->submenu);
  if (script->selected) {
    // Escape first closes the popup, then leaves keyboard menu-bar navigation.
    if (script->cancel && !popup && !script->second_escape_posted)
      script->second_escape_posted = PostMessageW(script->target, WM_KEYDOWN, VK_ESCAPE, 0) != FALSE;
    return;
  }
  if (!popup) return;
  if (GetMenuItemCount(script->submenu) != 1 ||
      GetMenuItemID(script->submenu, 0) != script->command) {
    script->native_ok = false;
    EndMenu();
    return;
  }
  if (!script->home_posted) {
    script->home_posted = PostMessageW(popup, WM_KEYDOWN, VK_HOME, 0) != FALSE;
    return;
  }
  const UINT item = GetMenuState(script->submenu, 0, MF_BYPOSITION);
  const UINT file = GetMenuState(GetMenu(script->target), 0, MF_BYPOSITION);
  if (item == UINT(-1) || file == UINT(-1) || !(item & MF_HILITE) || !(file & MF_HILITE) ||
      (item & (MF_DISABLED | MF_GRAYED))) return;
  DwmFlush();
  MenuPixels pixels;
  if (!pixels.capture(script->target)) { script->native_ok = false; EndMenu(); return; }
  const auto focus = menu_focus(script->target);
  script->stable = script->previous.different(pixels) == 0 && focus == script->previous_focus
      ? script->stable + 1 : 0;
  script->previous = std::move(pixels);
  script->previous_focus = focus;
  if (script->stable != 3) return;
  script->observation->opened = std::move(script->previous);
  script->observation->opened_focus = focus;
  script->captured = true;
  std::cout << "NATIVE_LOOP_OPEN " << script->label << " command=" << script->command
            << " native_popup=1 highlighted=1 focus=";
  print_focus(focus);
  std::cout << std::endl;
  // Only native menu keyboard input; never synthesize WM_COMMAND or repair pixels.
  script->selected = PostMessageW(popup, WM_KEYDOWN, script->cancel ? VK_ESCAPE : VK_RETURN, 0) != FALSE;
}

inline MenuLoopObservation native_menu_loop(HWND hwnd, MenuCommands& events, const char* label,
                                            bool cancel = false) {
  MenuLoopObservation result;
  if (!own_menu_target(hwnd) || ordinary_menu_script) return result;
  const HMENU bar = GetMenu(hwnd), submenu = bar ? GetSubMenu(bar, 0) : nullptr;
  const UINT command = submenu ? GetMenuItemID(submenu, 0) : UINT(-1);
  if (!submenu || command == UINT(-1) || !command) return result;
  SetActiveWindow(hwnd);
  SetFocus(hwnd);
  if (!stable_menu(hwnd, result.before, result.before_focus)) return result;
  std::cout << "NATIVE_LOOP_BEGIN " << label << " command=" << command << " focus=";
  print_focus(result.before_focus);
  std::cout << std::endl;
  if ((GetWindowLongPtrW(hwnd, GWL_EXSTYLE) & WS_EX_NOACTIVATE) || !result.before_focus.active ||
      !result.before_focus.focused) return result;
  const unsigned before = events.commands;
  OrdinaryMenuScript script{.target = hwnd, .submenu = submenu, .command = command,
      .observation = &result, .label = label, .deadline = GetTickCount64() + 4000, .cancel = cancel};
  ordinary_menu_script = &script;
  const UINT_PTR timer = SetTimer(nullptr, 0, 100, drive_ordinary_native_menu);
  if (!timer) { ordinary_menu_script = nullptr; return result; }
  // The OS runs its real menu loop and returns only once that menu has closed.
  SendMessageW(hwnd, WM_SYSCOMMAND, SC_KEYMENU, L'f');
  KillTimer(nullptr, timer);
  ordinary_menu_script = nullptr;
  const bool settled = stable_menu(hwnd, result.after, result.after_focus);
  const bool command_ok = cancel ? events.commands == before :
      events.commands == before + 1 && events.last && events.last->command_id == command &&
      events.last->action_name == "file.open" && events.last->source == cgpui::NativeMenuCommandSource::menu;
  result.ok = settled && script.native_ok && !script.timed_out && script.captured && script.selected && command_ok;
  std::cout << "NATIVE_LOOP_CLOSED " << label << " ok=" << result.ok << " timeout=" << script.timed_out
            << " commands=" << events.commands - before << " focus=";
  print_focus(result.after_focus);
  std::cout << std::endl;
  return result;
}

} // namespace cgpui_test
