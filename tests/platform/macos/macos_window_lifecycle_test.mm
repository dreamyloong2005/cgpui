#include "cgpui/platform/platform.hpp"

#import <AppKit/AppKit.h>

#include <chrono>
#include <thread>
#include <variant>

int main() {
  @autoreleasepool {
    auto application = cgpui::create_platform_application();
    if (!application) return 1;

    bool close_requested = false;
    bool wakeup_requested = false;
    auto window = (*application)->create_window(
        cgpui::WindowDescriptor{
            .title = "CGPUI Cocoa Lifecycle Test",
            .size = cgpui::Size{320.0F, 240.0F},
            .position = cgpui::Point{80.0F, 90.0F}},
        [&](const cgpui::PlatformEvent& event) {
          close_requested = close_requested ||
              std::holds_alternative<cgpui::WindowCloseRequested>(event);
          wakeup_requested = wakeup_requested ||
              std::holds_alternative<cgpui::WindowWakeupRequested>(event);
        });
    if (!window) return 2;

    const auto state = (*window)->state();
    if (state.framebuffer_size.width <= 0.0F || state.framebuffer_size.height <= 0.0F ||
        state.scale.value <= 0.0F) return 3;
    if (!(*window)->position_state().supported ||
        !(*window)->position_state().position.has_value()) return 4;
    if (!std::holds_alternative<cgpui::MetalSurfaceHandle>((*window)->native_surface())) {
      return 5;
    }

    (*window)->set_title("Cocoa Lifecycle Updated");
    (*window)->set_cursor(cgpui::CursorShape::pointing_hand);
    (*window)->request_close();
    if (!close_requested || !(*window)->close_request_state().pending ||
        !(*window)->state().close_requested) return 6;
    if (!(*window)->resolve_close_request(cgpui::PlatformWindowCloseResolution::cancel) ||
        (*window)->state().close_requested || (*window)->close_request_state().pending) {
      return 7;
    }

    (*application)->request_wakeup_after(10);
    (*application)->cancel_wakeup_after();
    (*application)->request_wakeup();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    (void)wakeup_requested;
    (*application)->quit();
  }
  return 0;
}
