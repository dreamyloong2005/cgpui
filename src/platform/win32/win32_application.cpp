#include "win32_window_internal.hpp"

#include <memory>
#include <utility>
#include <vector>

namespace cgpui {
namespace {

class Win32Application final : public PlatformApplication {
 public:
  Win32Application() : instance_(GetModuleHandleW(nullptr)) {
    ole_initialization_result_ = OleInitialize(nullptr);
    ole_initialized_ = SUCCEEDED(ole_initialization_result_);
  }

  ~Win32Application() override {
    if (ole_initialized_) {
      OleUninitialize();
    }
  }

  Result<std::unique_ptr<PlatformWindow>> create_window(
      const WindowDescriptor& descriptor,
      PlatformEventCallback callback) override {
    const wchar_t* class_name = L"CGPUIWindow";

    WNDCLASSEXW window_class{};
    window_class.cbSize = sizeof(WNDCLASSEXW);
    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = win32_window_proc;
    window_class.hInstance = instance_;
    window_class.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
    window_class.lpszClassName = class_name;

    if (RegisterClassExW(&window_class) == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
      return std::unexpected(Error{
          .code = ErrorCode::platform_initialization_failed,
          .message = "RegisterClassExW failed"});
    }

    auto state = WindowState{
        .framebuffer_size = descriptor.size,
        .scale = DpiScale{1.0F},
        .close_requested = false,
        .ime_text_input_support = ImeTextInputSupport::available};
    auto window = std::make_unique<Win32Window>(instance_, std::move(callback), state);

    const auto title = widen(descriptor.title);
    const DWORD style = win32_window_style_for(descriptor.chrome);
    const DWORD extended_style =
        win32_window_extended_style_for(descriptor.chrome);
    const int initial_x = descriptor.position.has_value()
        ? static_cast<int>(descriptor.position->x)
        : CW_USEDEFAULT;
    const int initial_y = descriptor.position.has_value()
        ? static_cast<int>(descriptor.position->y)
        : CW_USEDEFAULT;
    HWND hwnd = CreateWindowExW(
        extended_style,
        class_name,
        title.c_str(),
        style,
        initial_x,
        initial_y,
        static_cast<int>(descriptor.size.width),
        static_cast<int>(descriptor.size.height),
        nullptr,
        nullptr,
        instance_,
        static_cast<Win32WindowMessageTarget*>(window.get()));
    if (hwnd == nullptr) {
      return std::unexpected(Error{
          .code = ErrorCode::window_creation_failed,
          .message = "CreateWindowExW failed"});
    }

    window->apply_window_chrome(descriptor.chrome);
    ShowWindow(hwnd, SW_SHOW);
    window->update_size();
    windows_.push_back(window.get());
    return window;
  }

  int run() override {
    running_thread_id_ = GetCurrentThreadId();
    MSG message{};
    while (running_ && GetMessageW(&message, nullptr, 0, 0) > 0) {
      if (message.message == cgpui_wakeup_message) {
        dispatch_wakeup();
        continue;
      }
      TranslateMessage(&message);
      DispatchMessageW(&message);
    }
    running_thread_id_ = 0;
    windows_.clear();
    return 0;
  }

  void request_wakeup() override {
    if (running_thread_id_ != 0) {
      PostThreadMessageW(running_thread_id_, cgpui_wakeup_message, 0, 0);
    }
  }

  PlatformMenuInstallationResult install_native_menu(
      NativeMenuModel menu) override {
    last_menu_installation_ =
        native_menu_state_.install_native_menu(std::move(menu));
    return last_menu_installation_;
  }

  NativeFileDialogResult show_native_file_dialog(
      NativeFileDialogOptions options) override {
    last_file_dialog_result_ =
        native_file_dialog_state_.show_native_file_dialog(std::move(options));
    return last_file_dialog_result_;
  }

  void quit() override {
    running_ = false;
    PostQuitMessage(0);
  }

  [[nodiscard]] PlatformFontDiscoveryResult discover_font_discovery()
      const override {
    return win32_discover_fonts();
  }

 private:
  void dispatch_wakeup() {
    for (Win32Window* window : windows_) {
      if (window != nullptr) {
        window->wakeup_requested();
      }
    }
  }

  HINSTANCE instance_ = nullptr;
  HRESULT ole_initialization_result_ = S_FALSE;
  bool ole_initialized_ = false;
  Win32NativeMenuState native_menu_state_;
  PlatformMenuInstallationResult last_menu_installation_;
  Win32NativeFileDialogState native_file_dialog_state_;
  NativeFileDialogResult last_file_dialog_result_;
  DWORD running_thread_id_ = 0;
  bool running_ = true;
  std::vector<Win32Window*> windows_;
};

} // namespace

Result<std::unique_ptr<PlatformApplication>> create_platform_application() {
  return std::make_unique<Win32Application>();
}

} // namespace cgpui
