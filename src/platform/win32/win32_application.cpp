#include "win32_window_factory_internal.hpp"
#include "win32_popup_window_internal.hpp"
#include "win32_system_application_internal.hpp"
#include "win32_message_dialog_internal.hpp"
#include "win32_open_url_internal.hpp"
#include "win32_application_initialization_internal.hpp"
#include "win32_window_registry_internal.hpp"
#include "win32_native_menu_policy_internal.hpp"

#include <memory>
#include <utility>
#include <vector>

namespace cgpui {
namespace {

class Win32Application final : public Win32SystemApplication {
 public:
  explicit Win32Application(Win32TimerWakeup::CreateTimer create_timer)
      : instance_(GetModuleHandleW(nullptr)), timer_wakeup_(create_timer) {}
  Result<void> initialization_result() const { return timer_wakeup_.initialization_result(); }

  ~Win32Application() override { if (ole_initialized_) OleUninitialize(); }

  Result<std::unique_ptr<PlatformWindow>> create_window(
      const WindowDescriptor& descriptor,
      PlatformEventCallback callback) override {
    return create_window_with_owner(descriptor, std::move(callback), nullptr);
  }

  Result<std::unique_ptr<PlatformWindow>> create_child_window(
      const WindowDescriptor& descriptor,
      PlatformWindow& parent,
      PlatformEventCallback callback) override {
    const HWND owner = win32_window_hwnd(parent);
    if (owner == nullptr) {
      return std::unexpected(Error{
          .code = ErrorCode::invalid_argument,
          .message = "Win32 child window requires a Win32 parent"});
    }
    return create_window_with_owner(descriptor, std::move(callback), owner);
  }

  int run() override {
    running_thread_id_ = GetCurrentThreadId(); timer_wakeup_.set_thread_id(running_thread_id_);
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
    running_thread_id_ = 0; timer_wakeup_.set_thread_id(0);
    return 0;
  }

  void request_wakeup() override { timer_wakeup_.request_now(); }

  PlatformMenuInstallationResult install_native_menu(
      NativeMenuModel menu) override {
    Win32NativeMenuInstallationGuard installation(windows_, !menu.items.empty());
    if (!installation.allowed()) return {.supported = false, .backend = "win32"};
    windows_.visit([&](Win32Window* window) {
      (void)win32_apply_native_menu(window_handle(window), nullptr);
    });
    last_menu_installation_ =
        native_menu_state_.install_native_menu(std::move(menu));
    windows_.visit([&](Win32Window* window) {
      apply_native_menu(window);
    });
    return last_menu_installation_;
  }

  NativeFileDialogResult show_native_file_dialog(
      NativeFileDialogOptions options) override {
    last_file_dialog_result_ =
        native_file_dialog_state_.show_native_file_dialog(std::move(options));
    return last_file_dialog_result_;
  }

  void request_wakeup_after(std::uint64_t delay_ms) override { timer_wakeup_.request(delay_ms); }

  void cancel_wakeup_after() override { timer_wakeup_.cancel(); }

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
    const bool popup = descriptor.kind == WindowKind::popup;
    auto window = popup
        ? create_win32_popup_window(instance_, descriptor, std::move(callback), owner)
        : create_win32_window(instance_, descriptor, std::move(callback), owner);
    if (!window) {
      return std::unexpected(window.error());
    }
    // Popups take neither the menu bar nor wakeups (their root window
    // does), so the many short-lived ones never stay in windows_.
    if (popup) return std::unique_ptr<PlatformWindow>(std::move(*window));
    apply_native_menu(window->get());
    windows_.add(**window);
    return std::unique_ptr<PlatformWindow>(std::move(*window));
  }

  [[nodiscard]] static HWND window_handle(Win32Window* window) {
    return window == nullptr ? nullptr : win32_window_hwnd(*window);
  }

  void apply_native_menu(Win32Window* window) {
    if (window != nullptr)
      window->set_native_menu_commands(native_menu_state_.command_map());
    if (native_menu_state_.native_menu() == nullptr) return;
    (void)win32_apply_native_menu(
        window_handle(window),
        native_menu_state_.native_menu());
  }

  void dispatch_wakeup() {
    windows_.visit([](Win32Window* window) { window->wakeup_requested(); });
  }

  HINSTANCE instance_ = nullptr;
  // OLE (drag and drop, clipboard) is initialized with the application and
  // released by the destructor when this initialization succeeded.
  bool ole_initialized_ = SUCCEEDED(OleInitialize(nullptr));
  Win32NativeMenuState native_menu_state_;
  PlatformMenuInstallationResult last_menu_installation_;
  Win32NativeFileDialogState native_file_dialog_state_;
  NativeFileDialogResult last_file_dialog_result_;
  DWORD running_thread_id_ = 0; Win32TimerWakeup timer_wakeup_; bool running_ = true;
  Win32WindowRegistry windows_;
};

} // namespace

Result<std::unique_ptr<PlatformApplication>> create_win32_application(
    Win32TimerWakeup::CreateTimer create_timer) {
  auto application = std::make_unique<Win32Application>(create_timer);
  if (auto ready = application->initialization_result(); !ready)
    return std::unexpected(ready.error());
  return application;
}

} // namespace cgpui
