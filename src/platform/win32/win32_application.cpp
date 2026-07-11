#include "win32_window_factory_internal.hpp"
#include "win32_message_dialog_internal.hpp"
#include "win32_open_url_internal.hpp"

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
    return create_window_with_owner(descriptor, std::move(callback), nullptr);
  }

  Result<std::unique_ptr<PlatformWindow>> create_child_window(
      const WindowDescriptor& descriptor,
      PlatformWindow& parent,
      PlatformEventCallback callback) override {
    const NativeSurfaceHandle native_parent = parent.native_surface();
    const auto* parent_surface =
        std::get_if<Win32SurfaceHandle>(&native_parent);
    if (parent_surface == nullptr || parent_surface->hwnd == nullptr) {
      return std::unexpected(Error{
          .code = ErrorCode::invalid_argument,
          .message = "Win32 child window requires a Win32 parent"});
    }
    return create_window_with_owner(
        descriptor,
        std::move(callback),
        static_cast<HWND>(parent_surface->hwnd));
  }

  int run() override {
    running_thread_id_ = GetCurrentThreadId();
    MSG message{};
    while (running_ && GetMessageW(&message, nullptr, 0, 0) > 0) {
      if (message.message == cgpui_wakeup_message) {
        dispatch_wakeup();
        continue;
      }
      if (native_menu_state_.translate_accelerator(message)) continue;
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
    for (Win32Window* window : windows_) {
      (void)win32_apply_native_menu(window_handle(window), nullptr);
    }
    last_menu_installation_ =
        native_menu_state_.install_native_menu(std::move(menu));
    for (Win32Window* window : windows_) {
      apply_native_menu(window);
    }
    return last_menu_installation_;
  }

  NativeFileDialogResult show_native_file_dialog(
      NativeFileDialogOptions options) override {
    last_file_dialog_result_ =
        native_file_dialog_state_.show_native_file_dialog(std::move(options));
    return last_file_dialog_result_;
  }

  NativeMessageDialogResult show_native_message_dialog(
      NativeMessageDialogOptions options) override {
    return show_win32_native_message_dialog(options);
  }

  PlatformOpenUrlResult open_url(std::string url) override {
    return win32_open_url(url);
  }

  PlatformReopenResult request_reopen() override {
    return dispatch_reopen("win32");
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
  Result<std::unique_ptr<PlatformWindow>> create_window_with_owner(
      const WindowDescriptor& descriptor,
      PlatformEventCallback callback,
      HWND owner) {
    auto window =
        create_win32_window(instance_, descriptor, std::move(callback), owner);
    if (!window) {
      return std::unexpected(window.error());
    }
    apply_native_menu(window->get());
    windows_.push_back(window->get());
    return std::unique_ptr<PlatformWindow>(std::move(*window));
  }

  [[nodiscard]] static HWND window_handle(Win32Window* window) {
    if (window == nullptr) return nullptr;
    const NativeSurfaceHandle surface = window->native_surface();
    const auto* win32_surface = std::get_if<Win32SurfaceHandle>(&surface);
    return win32_surface == nullptr
        ? nullptr
        : static_cast<HWND>(win32_surface->hwnd);
  }

  void apply_native_menu(Win32Window* window) {
    if (window != nullptr) {
      window->set_native_menu_commands(native_menu_state_.command_map());
    }
    if (native_menu_state_.native_menu() == nullptr) return;
    (void)win32_apply_native_menu(
        window_handle(window),
        native_menu_state_.native_menu());
  }

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
