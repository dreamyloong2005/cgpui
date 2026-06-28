#include "cgpui/platform/platform.hpp"

#include <wayland-client.h>

#include <expected>
#include <memory>
#include <string_view>
#include <utility>

namespace cgpui {
namespace {

class WaylandWindow final : public PlatformWindow {
 public:
  WaylandWindow(wl_display* display, PlatformEventCallback callback, WindowState state)
      : display_(display), callback_(std::move(callback)), state_(state) {}

  [[nodiscard]] NativeSurfaceHandle native_surface() const override {
    return WaylandSurfaceHandle{.display = display_, .surface = nullptr};
  }

  [[nodiscard]] WindowState state() const override { return state_; }

  void request_redraw() override { callback_(WindowRedrawRequested{}); }

  void set_title(std::string_view) override {}

 private:
  wl_display* display_ = nullptr;
  PlatformEventCallback callback_;
  WindowState state_;
};

class WaylandApplication final : public PlatformApplication {
 public:
  WaylandApplication() : display_(wl_display_connect(nullptr)) {}

  ~WaylandApplication() override {
    if (display_ != nullptr) {
      wl_display_disconnect(display_);
    }
  }

  Result<std::unique_ptr<PlatformWindow>> create_window(
      const WindowDescriptor& descriptor,
      PlatformEventCallback callback) override {
    if (display_ == nullptr) {
      return std::unexpected(Error{
          .code = ErrorCode::platform_initialization_failed,
          .message = "wl_display_connect failed"});
    }

    auto state = WindowState{
        .framebuffer_size = descriptor.size,
        .scale = DpiScale{1.0F},
        .close_requested = false};
    return std::make_unique<WaylandWindow>(display_, std::move(callback), state);
  }

  int run() override {
    while (running_ && display_ != nullptr) {
      wl_display_dispatch_pending(display_);
      wl_display_flush(display_);
      running_ = false;
    }
    return 0;
  }

  void quit() override { running_ = false; }

 private:
  wl_display* display_ = nullptr;
  bool running_ = true;
};

} // namespace

Result<std::unique_ptr<PlatformApplication>> create_platform_application() {
  return std::make_unique<WaylandApplication>();
}

} // namespace cgpui
