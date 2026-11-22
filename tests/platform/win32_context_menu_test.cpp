#include "cgpui/platform/platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <variant>
#include <vector>

namespace {

// What the timer does once the menu window (class #32768) is open: check
// the HMENU, then post keys to it; with no keys left EndMenu cancels.
struct MenuScript {
  std::vector<WPARAM> keys;
  bool opened = false;
  bool states_ok = false;
  UINT_PTR timer = 0;
};
MenuScript script;

HWND open_menu_window() {
  HWND found = nullptr;
  EnumThreadWindows(GetCurrentThreadId(), [](HWND hwnd, LPARAM out) -> BOOL {
    wchar_t name[16]{};
    GetClassNameW(hwnd, name, 16);
    if (lstrcmpW(name, L"#32768") != 0 || IsWindowVisible(hwnd) == FALSE) return TRUE;
    *reinterpret_cast<HWND*>(out) = hwnd;
    return FALSE;
  }, reinterpret_cast<LPARAM>(&found));
  return found;
}

UINT item_state(HMENU menu, UINT position, UINT& type, HMENU& submenu) {
  MENUITEMINFOW info{.cbSize = sizeof(info),
                     .fMask = MIIM_FTYPE | MIIM_STATE | MIIM_SUBMENU};
  GetMenuItemInfoW(menu, position, TRUE, &info);
  type = info.fType;
  submenu = info.hSubMenu;
  return info.fState;
}

bool check_states(HMENU menu) {
  UINT type = 0;
  HMENU submenu = nullptr;
  if (menu == nullptr || GetMenuItemCount(menu) != 6) return false;
  const UINT first = item_state(menu, 0, type, submenu);
  if ((first & (MFS_GRAYED | MFS_CHECKED)) != 0) return false;
  item_state(menu, 1, type, submenu);
  if ((type & MFT_SEPARATOR) == 0) return false;
  if ((item_state(menu, 3, type, submenu) & MFS_CHECKED) == 0) return false;
  if ((item_state(menu, 4, type, submenu) & MFS_GRAYED) == 0) return false;
  item_state(menu, 5, type, submenu);
  return submenu != nullptr && GetMenuItemCount(submenu) == 1;
}

// One step per tick, so the menu loop has handled the previous key (a
// submenu opens) before the next one arrives.
void CALLBACK drive_menu(HWND, UINT, UINT_PTR, DWORD) {
  const HWND menu_window = open_menu_window();
  if (menu_window == nullptr) return;
  if (!script.opened) {
    script.opened = true;
    script.states_ok = check_states(reinterpret_cast<HMENU>(
        SendMessageW(menu_window, MN_GETHMENU, 0, 0)));
  }
  if (script.keys.empty()) {
    EndMenu();
    return;
  }
  PostMessageW(menu_window, WM_KEYDOWN, script.keys.front(), 0);
  script.keys.erase(script.keys.begin());
}

cgpui::PlatformContextMenuResult show(cgpui::PlatformWindow& window,
                                      std::vector<WPARAM> keys) {
  script = MenuScript{.keys = std::move(keys)};
  script.timer = SetTimer(nullptr, 0, 50, drive_menu);
  auto result = window.show_context_menu(
      cgpui::NativeMenuModel{.items = {
          {.title = "New File...", .action_name = "file.new"},
          {.kind = cgpui::NativeMenuItemKind::separator},
          {.title = "Rename", .action_name = "edit.rename"},
          {.title = "Word Wrap", .action_name = "view.wrap", .checked = true},
          {.title = "Delete", .action_name = "edit.delete", .enabled = false},
          {.kind = cgpui::NativeMenuItemKind::submenu, .title = "Open With",
           .children = {{.title = "Text Editor", .action_name = "open.text"}}}}},
      {40.0F, 60.0F});
  KillTimer(nullptr, script.timer);
  return result;
}

void pump() {
  MSG message{};
  while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE) != FALSE) {
    TranslateMessage(&message);
    DispatchMessageW(&message);
  }
}

const cgpui::NativeMenuCommand* last_command(const std::vector<cgpui::PlatformEvent>& events) {
  const cgpui::NativeMenuCommand* command = nullptr;
  for (const auto& event : events) {
    if (const auto* value = std::get_if<cgpui::NativeMenuCommand>(&event)) command = value;
  }
  return command;
}

} // namespace

int main() {
  auto app = cgpui::create_platform_application();
  if (!app) return 1;
  std::vector<cgpui::PlatformEvent> events;
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{.title = "CGPUI Win32 Context Menu Test",
                              .size = {320.0F, 240.0F}},
      [&](const cgpui::PlatformEvent& event) { events.push_back(event); });
  if (!window) return 2;
  pump();

  // Cancelled (EndMenu): shown, nothing chosen, no command.
  events.clear();
  const auto cancelled = show(**window, {});
  pump();
  if (!script.opened || !script.states_ok) return 3;
  if (!cancelled.supported || !cancelled.shown || cancelled.backend != "win32" ||
      cancelled.action_name.has_value() || last_command(events) != nullptr) {
    return 4;
  }

  // Down, Down (over the separator), Enter chooses "Rename". The command
  // follows one message loop turn later, not inside the call.
  events.clear();
  const auto chosen = show(**window, {VK_DOWN, VK_DOWN, VK_RETURN});
  if (!script.opened || chosen.action_name != "edit.rename" ||
      last_command(events) != nullptr) {
    return 5;
  }
  pump();
  const auto* command = last_command(events);
  if (command == nullptr || command->action_name != "edit.rename" ||
      command->source != cgpui::NativeMenuCommandSource::context_menu) {
    return 6;
  }

  // Up (to the last item), Right (into the submenu), Enter.
  events.clear();
  const auto nested = show(**window, {VK_UP, VK_RIGHT, VK_RETURN});
  pump();
  command = last_command(events);
  if (nested.action_name != "open.text" || command == nullptr ||
      command->action_name != "open.text") {
    return 7;
  }

  const auto empty = (*window)->show_context_menu({}, {0.0F, 0.0F});
  if (!empty.supported || empty.shown || empty.reason.empty()) return 8;

  // A window destroyed before the command is delivered sends nothing.
  events.clear();
  const auto last = show(**window, {VK_DOWN, VK_RETURN});
  window->reset();
  pump();
  return last.action_name == "file.new" && last_command(events) == nullptr ? 0 : 9;
}
