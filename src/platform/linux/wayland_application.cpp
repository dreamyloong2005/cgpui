#include "cgpui/platform/platform.hpp"

#include <wayland-client.h>

#include <expected>
#include <memory>

namespace cgpui {
namespace {

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

    return std::unexpected(Error{
        .code = ErrorCode::window_creation_failed,
        .message = "Wayland surface creation is not implemented yet"});
  }

  int run() override {
    while (running_ && display_ != nullptr) {
      if (wl_display_dispatch(display_) == -1) {
        return 1;
      }
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
