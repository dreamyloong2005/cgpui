#include "runtime_popups_internal.hpp"

#include "ui_internal.hpp"

#include <utility>
#include <variant>

namespace cgpui {
namespace {

WindowRuntimeRecord* open_popup_record(WindowRuntime& runtime, WindowRuntimeId popup) {
  const WindowRuntimeRecord* record = runtime.window_runtime_record(popup);
  if (record == nullptr || !record->active || record->window == nullptr ||
      record->descriptor.kind != WindowKind::popup) {
    return nullptr;
  }
  return const_cast<WindowRuntimeRecord*>(record);
}

}  // namespace

bool popup_windows_supported(const WindowRuntime& runtime) {
  return RuntimePopups::supported(runtime);
}

bool RuntimePopups::supported(const WindowRuntime& runtime) {
  return runtime.application_.popup_windows_supported();
}

Result<AppOpenedWindow> open_popup(
    WindowRuntime& runtime, WindowPopupPlacement placement, Size size,
    std::unique_ptr<View> view, bool takes_keyboard) {
  return RuntimePopups::open(runtime, placement, size, std::move(view), takes_keyboard);
}

PlatformPopupPlacementResult place_popup(
    WindowRuntime& runtime, WindowRuntimeId popup, WindowPopupPlacement placement) {
  WindowRuntimeRecord* record = open_popup_record(runtime, popup);
  if (record == nullptr) {
    return PlatformPopupPlacementResult{.reason = "not an open popup window"};
  }
  record->descriptor.popup = placement;
  return record->window->place_popup(placement);
}

bool request_popup_redraw(WindowRuntime& runtime, WindowRuntimeId popup) {
  WindowRuntimeRecord* record = open_popup_record(runtime, popup);
  if (record == nullptr) return false;
  record->window->request_redraw();
  return true;
}

bool close_popup(WindowRuntime& runtime, WindowRuntimeId popup) {
  return RuntimePopups::close(runtime, popup);
}

Result<AppOpenedWindow> RuntimePopups::open(
    WindowRuntime& runtime, WindowPopupPlacement placement, Size size,
    std::unique_ptr<View> view, bool takes_keyboard) {
  if (!popup_windows_supported(runtime)) {
    return std::unexpected(Error{
        .code = ErrorCode::unsupported_platform,
        .message = "popup windows are unsupported on this backend"});
  }
  auto opened = runtime.try_open_window(
      WindowOptions{}.size(size).popup(placement), std::move(view));
  if (opened && takes_keyboard) runtime.keyboard_popup_ = opened->runtime_id;
  return opened;
}

bool RuntimePopups::close(WindowRuntime& runtime, WindowRuntimeId popup) {
  WindowRuntimeRecord* record = open_popup_record(runtime, popup);
  if (record == nullptr) return false;
  if (runtime.keyboard_popup_ == popup) runtime.keyboard_popup_ = {};
  runtime.cleanup_closed_additional_window(*record);
  return true;
}

bool RuntimePopups::forward_keyboard(WindowRuntime& runtime, const PlatformEvent& event) {
  if (runtime.keyboard_popup_.value == 0 ||
      (!std::holds_alternative<KeyboardKey>(event) &&
       !std::holds_alternative<TextInput>(event))) {
    return false;
  }
  if (open_popup_record(runtime, runtime.keyboard_popup_) == nullptr) {
    runtime.keyboard_popup_ = {};
    return false;
  }
  runtime.handle_native_additional_window_event(runtime.keyboard_popup_, event);
  return true;
}

} // namespace cgpui
