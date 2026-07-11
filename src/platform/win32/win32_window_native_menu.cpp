#include "win32_window_internal.hpp"

#include <utility>

namespace cgpui {

void Win32Window::set_native_menu_commands(
    std::shared_ptr<const Win32NativeMenuCommandMap> commands) {
  native_menu_commands_ = std::move(commands);
}

void Win32Window::native_menu_command(
    UINT command_id,
    NativeMenuCommandSource source) {
  if (native_menu_commands_ == nullptr) return;
  const auto command = native_menu_commands_->resolve(command_id, source);
  if (command.has_value()) callback_(std::move(*command));
}

} // namespace cgpui
