#include "cgpui/platform/platform.hpp"
#include "cgpui/renderer/renderer.hpp"
#include "macos_input_internal.hpp"
#include "macos_window_internal.hpp"

#import <AppKit/AppKit.h>

#include <chrono>
#include <thread>
#include <variant>

namespace {

void pump(std::chrono::milliseconds duration) {
  [[NSRunLoop mainRunLoop]
      runUntilDate:[NSDate dateWithTimeIntervalSinceNow:
          static_cast<NSTimeInterval>(duration.count()) / 1000.0]];
}

}  // namespace

int main() {
  @autoreleasepool {
    auto application = cgpui::create_platform_application();
    if (!application) return 1;
    bool pointer_seen = false;
    bool wakeup_seen = false;
    bool close_seen = false;
    auto window = (*application)->create_window(
        {.title = "CGPUI macOS Example Smoke", .size = {160.0F, 120.0F}},
        [&](const cgpui::PlatformEvent& event) {
          pointer_seen |= std::holds_alternative<cgpui::PointerButton>(event);
          wakeup_seen |=
              std::holds_alternative<cgpui::WindowWakeupRequested>(event);
          close_seen |=
              std::holds_alternative<cgpui::WindowCloseRequested>(event);
        });
    if (!window) return 2;
    const cgpui::WindowState state = (*window)->state();
    auto renderer = cgpui::create_renderer({.native_surface = (*window)->native_surface(),
                                            .framebuffer_size = state.framebuffer_size,
                                            .scale = state.scale});
    if (!renderer) return 3;
    auto frame = (*renderer)->begin_frame();
    if (!frame) return 4;
    (*frame)->clear({.r = 0.1F, .g = 0.2F, .b = 0.3F, .a = 1.0F});
    (*frame)->request_pixel_capture();
    if (!(*frame)->present()) return 5;
    if (!(*renderer)->resize({128.0F, 96.0F}, cgpui::DpiScale{1.0F})) return 6;
    frame = (*renderer)->begin_frame();
    if (!frame) return 7;
    (*frame)->clear({.r = 0.2F, .g = 0.3F, .b = 0.4F, .a = 1.0F});
    if (!(*frame)->present()) return 8;

    auto* native = dynamic_cast<cgpui::MacOSWindow*>(window->get());
    auto* content = (CGPUIMacOSContentView*)[native->native_window() contentView];
    NSEvent* mouse = [NSEvent mouseEventWithType:NSEventTypeLeftMouseDown
                                         location:NSMakePoint(20.0, 20.0)
                                    modifierFlags:0
                                        timestamp:0
                                     windowNumber:native->native_window().windowNumber
                                          context:nil
                                      eventNumber:1
                                       clickCount:1
                                         pressure:1.0];
    [content mouseDown:mouse];
    (*application)->request_wakeup_after(1);
    const auto deadline = std::chrono::steady_clock::now() +
                          std::chrono::milliseconds{500};
    while (!wakeup_seen && std::chrono::steady_clock::now() < deadline) {
      pump(std::chrono::milliseconds{5});
    }
    (*window)->request_close();
    if (!pointer_seen || !wakeup_seen || !close_seen ||
        !(*window)->resolve_close_request(
            cgpui::PlatformWindowCloseResolution::accept)) {
      return 9;
    }
  }
  return 0;
}
