#include "win32_popup_window_internal.hpp"
#include "win32_dpi_awareness_internal.hpp"
#include "win32_window_client_size_internal.hpp"

#include <cwchar>
#include <utility>

namespace cgpui {
namespace {

bool register_popup_class(HINSTANCE instance) {
  WNDCLASSEXW window_class{};
  window_class.cbSize = sizeof(WNDCLASSEXW);
  // As CGPUIWindow: a resize repaints the whole client (a new frame).
  window_class.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
  window_class.lpfnWndProc = win32_window_proc;
  window_class.cbWndExtra = win32_popup_extra_bytes;
  window_class.hInstance = instance;
  window_class.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
  window_class.lpszClassName = win32_popup_window_class;
  return RegisterClassExW(&window_class) != 0 ||
         GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

} // namespace

Result<std::unique_ptr<Win32Window>> create_win32_popup_window(
    HINSTANCE instance,
    const WindowDescriptor& descriptor,
    PlatformEventCallback callback,
    HWND owner) {
  if (owner == nullptr || IsWindow(owner) == FALSE) {
    return std::unexpected(Error{
        .code = ErrorCode::invalid_argument,
        .message = "a popup window needs a parent window"});
  }
  (void)win32_declare_dpi_awareness();
  if (!register_popup_class(instance)) {
    return std::unexpected(Error{
        .code = ErrorCode::platform_initialization_failed,
        .message = "RegisterClassExW failed for popup windows"});
  }
  auto window = std::make_unique<Win32Window>(
      instance, std::move(callback),
      WindowState{.framebuffer_size = descriptor.size, .scale = DpiScale{1.0F}});
  // Created on the owner's monitor, so it starts at the owner's scale.
  POINT origin{};
  ClientToScreen(owner, &origin);
  const bool pointer_transparent = descriptor.popup_options.pointer_transparent;
  HWND hwnd = CreateWindowExW(
      win32_popup_extended_style |
          (pointer_transparent ? win32_popup_pointer_transparent_style : 0),
      win32_popup_window_class, L"", win32_popup_style, origin.x, origin.y, 1, 1,
      owner, nullptr, instance, static_cast<Win32WindowMessageTarget*>(window.get()));
  if (hwnd == nullptr) {
    return std::unexpected(Error{
        .code = ErrorCode::window_creation_failed,
        .message = "CreateWindowExW failed for a popup window"});
  }
  // A layered window is shown once its attributes are set; alpha 255 keeps
  // the composited content as presented.
  if (pointer_transparent) SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
  (void)win32_set_logical_client_size(hwnd, descriptor.size);
  if (descriptor.popup.has_value()) (void)window->place_popup(*descriptor.popup);
  ShowWindow(hwnd, SW_SHOWNOACTIVATE);
  window->update_size();
  return window;
}

bool win32_is_popup_window(HWND hwnd) {
  wchar_t name[32]{};
  return hwnd != nullptr && GetClassNameW(hwnd, name, 32) > 0 &&
         std::wcscmp(name, win32_popup_window_class) == 0;
}

PlatformPopupPlacementResult Win32Window::place_popup(WindowPopupPlacement placement) {
  return win32_place_popup(hwnd_, placement);
}

} // namespace cgpui
