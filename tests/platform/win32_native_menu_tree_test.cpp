#include "cgpui/platform/platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string>

namespace {

std::wstring menu_title(HMENU menu, UINT position) {
  wchar_t buffer[128]{};
  const int length = GetMenuStringW(
      menu,
      position,
      buffer,
      static_cast<int>(std::size(buffer)),
      MF_BYPOSITION);
  return length > 0 ? std::wstring(buffer, static_cast<std::size_t>(length))
                    : std::wstring{};
}

} // namespace

int main() {
  auto app = cgpui::create_platform_application();
  if (!app) return 1;
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{
          .title = "CGPUI Win32 Native Menu Tree Test",
          .size = {420.0F, 280.0F}},
      [](const cgpui::PlatformEvent&) {});
  if (!window) return 2;

  cgpui::NativeMenuModel model{
      .items = {
          cgpui::NativeMenuItem{
              .kind = cgpui::NativeMenuItemKind::submenu,
              .title = "\xE6\x96\x87\xE4\xBB\xB6",
              .children = {
                  cgpui::NativeMenuItem{
                      .title = "Open",
                      .action_name = "file.open",
                      .enabled = false,
                  },
                  cgpui::NativeMenuItem{
                      .kind = cgpui::NativeMenuItemKind::separator,
                  },
                  cgpui::NativeMenuItem{
                      .kind = cgpui::NativeMenuItemKind::submenu,
                      .title = "Recent",
                      .children = {
                          cgpui::NativeMenuItem{
                              .title = "One",
                              .action_name = "file.open_recent",
                          },
                      },
                  },
                  cgpui::NativeMenuItem{
                      .title = "Autosave",
                      .action_name = "file.autosave",
                      .checked = true,
                  },
                  cgpui::NativeMenuItem{
                      .title = "Mode A",
                      .action_name = "mode.a",
                      .checked = true,
                      .radio = true,
                  },
                  cgpui::NativeMenuItem{
                      .title = "Quit",
                      .action_name = "app.quit",
                  },
              },
          },
      },
  };
  const auto installed = (*app)->install_native_menu(std::move(model));
  if (!installed.supported || installed.backend != "win32" ||
      installed.menu_count != 1 || installed.item_count != 8) return 3;

  const auto native_surface = (*window)->native_surface();
  const auto* surface = std::get_if<cgpui::Win32SurfaceHandle>(&native_surface);
  if (surface == nullptr || surface->hwnd == nullptr) return 4;
  const HMENU root = GetMenu(static_cast<HWND>(surface->hwnd));
  if (root == nullptr || GetMenuItemCount(root) != 1 ||
      menu_title(root, 0) != L"\x6587\x4EF6") return 5;
  const HMENU file = GetSubMenu(root, 0);
  if (file == nullptr || GetMenuItemCount(file) != 6 ||
      menu_title(file, 0) != L"Open" ||
      (GetMenuState(file, 0, MF_BYPOSITION) & (MF_DISABLED | MF_GRAYED)) == 0 ||
      (GetMenuState(file, 1, MF_BYPOSITION) & MF_SEPARATOR) == 0 ||
      menu_title(file, 2) != L"Recent" ||
      (GetMenuState(file, 3, MF_BYPOSITION) & MF_CHECKED) == 0 ||
      menu_title(file, 5) != L"Quit") {
    return 6;
  }
  const HMENU recent = GetSubMenu(file, 2);
  if (recent == nullptr || GetMenuItemCount(recent) != 1 ||
      menu_title(recent, 0) != L"One") return 7;
  MENUITEMINFOW radio_info{.cbSize = sizeof(MENUITEMINFOW)};
  radio_info.fMask = MIIM_FTYPE | MIIM_STATE;
  if (GetMenuItemInfoW(file, 4, TRUE, &radio_info) == FALSE ||
      (radio_info.fType & MFT_RADIOCHECK) == 0 ||
      (radio_info.fState & MFS_CHECKED) == 0) return 8;
  auto future_window = (*app)->create_window(
      cgpui::WindowDescriptor{
          .title = "CGPUI Win32 Future Menu Test",
          .size = {320.0F, 200.0F}},
      [](const cgpui::PlatformEvent&) {});
  if (!future_window) return 9;
  const auto future_surface_handle = (*future_window)->native_surface();
  const auto* future_surface =
      std::get_if<cgpui::Win32SurfaceHandle>(&future_surface_handle);
  if (future_surface == nullptr || future_surface->hwnd == nullptr ||
      GetMenu(static_cast<HWND>(future_surface->hwnd)) != root) return 10;
  return 0;
}
