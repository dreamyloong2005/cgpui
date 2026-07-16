#pragma once

#include "x11_internal.hpp"

namespace cgpui {

class X11Window;
struct X11KeyboardState;

class X11Application final : public PlatformApplication {
 public:
  static Result<std::unique_ptr<X11Application>> create();
  ~X11Application() override;

  Result<std::unique_ptr<PlatformWindow>> create_window(
      const WindowDescriptor& descriptor,
      PlatformEventCallback callback) override;
  Result<std::unique_ptr<PlatformWindow>> create_child_window(
      const WindowDescriptor& descriptor,
      PlatformWindow& parent,
      PlatformEventCallback callback) override;
  int run() override;
  void request_wakeup() override;
  void request_wakeup_after(std::uint64_t delay_ms) override;
  void cancel_wakeup_after() override;
  void quit() override;
  PlatformReopenResult request_reopen() override;
  PlatformMenuInstallationResult install_native_menu(
      NativeMenuModel menu) override;
  NativeFileDialogResult show_native_file_dialog(
      NativeFileDialogOptions options) override;
  NativeMessageDialogResult show_native_message_dialog(
      NativeMessageDialogOptions options) override;
  PlatformOpenUrlResult open_url(std::string url) override;

 private:
  X11Application() = default;
  Result<void> initialize();
  Result<std::unique_ptr<PlatformWindow>> create_window_with_parent(
      const WindowDescriptor& descriptor,
      PlatformEventCallback callback,
      xcb_window_t parent);
  void unregister_window(X11Window* window);
  void dispatch_x11_events();
  void dispatch_wakeup();
  [[nodiscard]] int poll_timeout_ms() const;
  [[nodiscard]] bool timer_due() const;

  xcb_connection_t* connection_ = nullptr;
  xcb_screen_t* screen_ = nullptr;
  X11Atoms atoms_;
  DpiScale scale_{1.0F};
  std::shared_ptr<X11KeyboardState> keyboard_;
  xcb_cursor_context_t* cursor_context_ = nullptr;
  std::vector<X11Window*> windows_;
  int wakeup_pipe_[2] = {-1, -1};
  std::atomic_bool running_{true};
  std::atomic_bool wakeup_pending_{false};
  mutable std::mutex timer_mutex_;
  std::optional<std::chrono::steady_clock::time_point> timer_deadline_;
};

Result<std::unique_ptr<PlatformApplication>> create_x11_application();

}  // namespace cgpui
