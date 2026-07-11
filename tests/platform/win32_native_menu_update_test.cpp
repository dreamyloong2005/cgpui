#include "cgpui/platform/platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string>

namespace {

HWND window_handle(cgpui::PlatformWindow& window) {
  const auto native = window.native_surface();
  const auto* surface = std::get_if<cgpui::Win32SurfaceHandle>(&native);
  return surface == nullptr ? nullptr : static_cast<HWND>(surface->hwnd);
}

std::wstring menu_title(HMENU menu, UINT position) {
  wchar_t buffer[64]{};
  const int length = GetMenuStringW(
      menu,
      position,
      buffer,
      static_cast<int>(std::size(buffer)),
      MF_BYPOSITION);
  return length > 0 ? std::wstring(buffer, static_cast<std::size_t>(length))
                    : std::wstring{};
}

cgpui::NativeMenuModel menu_model(
    std::string title,
    std::string first,
    std::string second) {
  return cgpui::NativeMenuModel{
      .items = {cgpui::NativeMenuItem{
          .kind = cgpui::NativeMenuItemKind::submenu,
          .title = std::move(title),
          .children = {
              cgpui::NativeMenuItem{
                  .title = std::move(first),
                  .action_name = "first",
              },
              cgpui::NativeMenuItem{
                  .title = std::move(second),
                  .action_name = "second",
              },
          },
      }},
  };
}

} // namespace

int main() {
  auto app = cgpui::create_platform_application();
  if (!app) return 1;
  auto first_window = (*app)->create_window(
      cgpui::WindowDescriptor{.title = "Menu Update One", .size = {320, 200}},
      [](const cgpui::PlatformEvent&) {});
  auto second_window = (*app)->create_window(
      cgpui::WindowDescriptor{.title = "Menu Update Two", .size = {320, 200}},
      [](const cgpui::PlatformEvent&) {});
  if (!first_window || !second_window) return 2;
  const HWND first_hwnd = window_handle(**first_window);
  const HWND second_hwnd = window_handle(**second_window);
  if (first_hwnd == nullptr || second_hwnd == nullptr) return 3;

  const auto initial = (*app)->install_native_menu(
      menu_model("File", "Open", "Quit"));
  const HMENU initial_root = GetMenu(first_hwnd);
  if (!initial.supported || initial.item_count != 3 || initial_root == nullptr ||
      GetMenu(second_hwnd) != initial_root || menu_title(initial_root, 0) != L"File") {
    return 4;
  }

  const auto updated = (*app)->install_native_menu(
      menu_model("Edit", "Undo", "Redo"));
  const HMENU updated_root = GetMenu(first_hwnd);
  const HMENU updated_popup = GetSubMenu(updated_root, 0);
  if (!updated.supported || updated.item_count != 3 || updated_root == nullptr ||
      updated_root == initial_root || GetMenu(second_hwnd) != updated_root ||
      menu_title(updated_root, 0) != L"Edit" || updated_popup == nullptr ||
      menu_title(updated_popup, 0) != L"Undo" ||
      menu_title(updated_popup, 1) != L"Redo") return 5;

  const auto cleared = (*app)->install_native_menu({});
  if (!cleared.supported || cleared.menu_count != 0 || cleared.item_count != 0 ||
      GetMenu(first_hwnd) != nullptr || GetMenu(second_hwnd) != nullptr) return 6;

  auto future_window = (*app)->create_window(
      cgpui::WindowDescriptor{.title = "Menu Update Future", .size = {320, 200}},
      [](const cgpui::PlatformEvent&) {});
  if (!future_window || GetMenu(window_handle(**future_window)) != nullptr) return 7;
  return 0;
}
