#include "win32_window_factory_internal.hpp"

namespace cgpui {

Result<std::unique_ptr<Win32Window>> create_win32_window(
    HINSTANCE instance,
    const WindowDescriptor& descriptor,
    PlatformEventCallback callback,
    HWND owner) {
  constexpr const wchar_t* class_name = L"CGPUIWindow";
  WNDCLASSEXW window_class{};
  window_class.cbSize = sizeof(WNDCLASSEXW);
  window_class.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
  window_class.lpfnWndProc = win32_window_proc;
  window_class.hInstance = instance;
  window_class.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
  window_class.lpszClassName = class_name;
  if (RegisterClassExW(&window_class) == 0 &&
      GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
    return std::unexpected(Error{
        .code = ErrorCode::platform_initialization_failed,
        .message = "RegisterClassExW failed"});
  }

  auto window = std::make_unique<Win32Window>(
      instance,
      std::move(callback),
      WindowState{
          .framebuffer_size = descriptor.size,
          .scale = DpiScale{1.0F},
          .close_requested = false,
          .ime_text_input_support = ImeTextInputSupport::available});
  const auto title = widen(descriptor.title);
  const DWORD style = win32_window_style_for(descriptor.chrome);
  const DWORD extended_style = win32_window_extended_style_for(descriptor.chrome);
  const int initial_x = descriptor.position.has_value()
      ? static_cast<int>(descriptor.position->x)
      : CW_USEDEFAULT;
  const int initial_y = descriptor.position.has_value()
      ? static_cast<int>(descriptor.position->y)
      : CW_USEDEFAULT;
  HWND hwnd = CreateWindowExW(
      extended_style, class_name, title.c_str(), style, initial_x, initial_y,
      static_cast<int>(descriptor.size.width),
      static_cast<int>(descriptor.size.height), owner, nullptr, instance,
      static_cast<Win32WindowMessageTarget*>(window.get()));
  if (hwnd == nullptr) {
    return std::unexpected(Error{
        .code = ErrorCode::window_creation_failed,
        .message = "CreateWindowExW failed"});
  }
  window->apply_window_chrome(descriptor.chrome);
  ShowWindow(hwnd, SW_SHOW);
  window->update_size();
  return window;
}

} // namespace cgpui
