#include "cgpui/app/window.hpp"

#include "cgpui/ui/window_runtime.hpp"

namespace cgpui {

PlatformContextMenuResult Window::show_context_menu(
    const NativeMenuModel& menu, Point position) const {
  const WindowRuntimeRecord* runtime_record = record();
  if (runtime_record == nullptr || runtime_record->window == nullptr) {
    return PlatformContextMenuResult{.reason = "native window is not active"};
  }
  return runtime_record->window->show_context_menu(menu, position);
}

} // namespace cgpui
