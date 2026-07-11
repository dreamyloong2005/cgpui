#include "cgpui/platform/platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <optional>
#include <string>

int main() {
  auto app = cgpui::create_platform_application();
  if (!app) return 1;
  std::optional<cgpui::NativeMenuCommand> observed;
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{
          .title = "Native Menu Command",
          .size = {420, 280}},
      [&](const cgpui::PlatformEvent& event) {
        if (const auto* command = std::get_if<cgpui::NativeMenuCommand>(&event);
            command != nullptr) {
          observed = *command;
        }
      });
  if (!window) return 2;

  const auto installed = (*app)->install_native_menu(cgpui::NativeMenuModel{
      .items = {cgpui::NativeMenuItem{
          .kind = cgpui::NativeMenuItemKind::submenu,
          .title = "File",
          .children = {cgpui::NativeMenuItem{
              .title = "Open",
              .action_name = "file.open",
              .accelerator = cgpui::NativeMenuAccelerator{.key_code = 'O'},
          }},
      }},
  });
  if (!installed.supported) return 3;

  const auto native = (*window)->native_surface();
  const auto* surface = std::get_if<cgpui::Win32SurfaceHandle>(&native);
  const HWND hwnd = surface == nullptr
      ? nullptr
      : static_cast<HWND>(surface->hwnd);
  if (hwnd == nullptr) return 4;

  SendMessageW(hwnd, WM_COMMAND, MAKEWPARAM(0x1000U, 0U), 0);
  if (!observed.has_value() || observed->command_id != 0x1000U ||
      observed->action_name != "file.open" ||
      observed->source != cgpui::NativeMenuCommandSource::menu) return 5;

  observed.reset();
  SendMessageW(hwnd, WM_COMMAND, MAKEWPARAM(0x1000U, 1U), 0);
  if (!observed.has_value() ||
      observed->source != cgpui::NativeMenuCommandSource::accelerator) return 6;
  return 0;
}
