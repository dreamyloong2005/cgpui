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
      cgpui::WindowDescriptor{.title = "Menu Accelerator Display", .size = {420, 280}},
      [](const cgpui::PlatformEvent&) {});
  if (!window) return 2;

  cgpui::NativeMenuModel menu{
      .items = {cgpui::NativeMenuItem{
          .kind = cgpui::NativeMenuItemKind::submenu,
          .title = "File",
          .children = {
              cgpui::NativeMenuItem{
                  .title = "Open",
                  .action_name = "file.open",
                  .accelerator = cgpui::NativeMenuAccelerator{
                      .key_code = 'O',
                      .modifiers = {.shift = true, .control = true},
                  },
              },
              cgpui::NativeMenuItem{
                  .title = "Close",
                  .action_name = "file.close",
                  .accelerator = cgpui::NativeMenuAccelerator{
                      .key_code = VK_F4,
                      .modifiers = {.alt = true},
                  },
              },
              cgpui::NativeMenuItem{
                  .title = "Delete",
                  .action_name = "edit.delete",
                  .accelerator = cgpui::NativeMenuAccelerator{
                      .key_code = VK_DELETE,
                      .modifiers = {.control = true},
                  },
              },
              cgpui::NativeMenuItem{
                  .title = "Workspace 1",
                  .action_name = "workspace.one",
                  .accelerator = cgpui::NativeMenuAccelerator{
                      .key_code = '1',
                      .modifiers = {.super = true},
                  },
              },
          },
      }},
  };
  const auto installed = (*app)->install_native_menu(std::move(menu));
  if (!installed.supported || installed.accelerator_count != 4) return 3;

  const auto native = (*window)->native_surface();
  const auto* surface = std::get_if<cgpui::Win32SurfaceHandle>(&native);
  const HMENU root = surface == nullptr
      ? nullptr
      : GetMenu(static_cast<HWND>(surface->hwnd));
  const HMENU popup = root == nullptr ? nullptr : GetSubMenu(root, 0);
  if (popup == nullptr || menu_title(popup, 0) != L"Open\tCtrl+Shift+O" ||
      menu_title(popup, 1) != L"Close\tAlt+F4" ||
      menu_title(popup, 2) != L"Delete\tCtrl+Del" ||
      menu_title(popup, 3) != L"Workspace 1\tWin+1") return 4;
  return 0;
}
