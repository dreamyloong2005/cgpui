#include "cgpui/platform/platform.hpp"

#import <AppKit/AppKit.h>
#import <QuartzCore/CAMetalLayer.h>

#include <chrono>
#include <memory>
#include <optional>
#include <thread>
#include <variant>

namespace {

void pump_run_loop(std::chrono::milliseconds duration) {
  const auto seconds = static_cast<NSTimeInterval>(duration.count()) / 1000.0;
  [[NSRunLoop mainRunLoop]
      runUntilDate:[NSDate dateWithTimeIntervalSinceNow:seconds]];
}

}  // namespace

int main() {
  @autoreleasepool {
    auto application = cgpui::create_platform_application();
    if (!application) return 1;

    bool close_requested = false;
    int wakeup_count = 0;
    int secondary_wakeup_count = 0;
    bool destroy_secondary_on_wakeup = false;
    std::unique_ptr<cgpui::PlatformWindow> secondary;
    std::optional<cgpui::WindowResized> resized;
    std::optional<cgpui::WindowActivated> activated;
    std::optional<cgpui::WindowFocused> focused;
    auto window = (*application)->create_window(
        cgpui::WindowDescriptor{
            .title = "CGPUI Cocoa Lifecycle Test",
            .size = cgpui::Size{320.0F, 240.0F},
            .position = cgpui::Point{80.0F, 90.0F}},
        [&](const cgpui::PlatformEvent& event) {
          close_requested = close_requested ||
              std::holds_alternative<cgpui::WindowCloseRequested>(event);
          if (std::holds_alternative<cgpui::WindowWakeupRequested>(event)) {
            ++wakeup_count;
            if (destroy_secondary_on_wakeup) secondary.reset();
          }
          if (const auto* value = std::get_if<cgpui::WindowResized>(&event)) {
            resized = *value;
          }
          if (const auto* value = std::get_if<cgpui::WindowActivated>(&event)) {
            activated = *value;
          }
          if (const auto* value = std::get_if<cgpui::WindowFocused>(&event)) {
            focused = *value;
          }
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
    if ((*window)->apply_window_chrome(cgpui::WindowChromeOptions{}).supported) {
      return 19;
    }

    const auto surface = std::get<cgpui::MetalSurfaceHandle>((*window)->native_surface());
    auto* layer = (__bridge CAMetalLayer*)surface.layer;
    NSWindow* native_window = nil;
    for (NSWindow* candidate in [NSApp windows]) {
      if ([[candidate title] isEqualToString:@"CGPUI Cocoa Lifecycle Test"]) {
        native_window = candidate;
        break;
      }
    }
    if (native_window == nil || layer == nil) return 6;
    id<NSWindowDelegate> delegate = [native_window delegate];
    [delegate windowDidResize:[NSNotification notificationWithName:NSWindowDidResizeNotification
                                                              object:native_window]];
    [delegate windowDidBecomeMain:[NSNotification notificationWithName:NSWindowDidBecomeMainNotification
                                                                  object:native_window]];
    [delegate windowDidBecomeKey:[NSNotification notificationWithName:NSWindowDidBecomeKeyNotification
                                                                 object:native_window]];
    if (!resized.has_value() || resized->size.width <= 0.0F ||
        resized->size.height <= 0.0F || resized->scale.value <= 0.0F) return 7;
    if (!activated.has_value() || !activated->active ||
        !focused.has_value() || !focused->focused) return 8;

    (*window)->set_title("Cocoa Lifecycle Updated");
    (*window)->set_cursor(cgpui::CursorShape::pointing_hand);
    (*window)->request_close();
    if (!close_requested || !(*window)->close_request_state().pending ||
        !(*window)->state().close_requested) return 9;
    if (!(*window)->resolve_close_request(cgpui::PlatformWindowCloseResolution::cancel) ||
        (*window)->state().close_requested || (*window)->close_request_state().pending) {
      return 10;
    }

    (*application)->request_wakeup_after(1);
    pump_run_loop(std::chrono::milliseconds(20));
    if (wakeup_count != 1) return 11;
    (*application)->request_wakeup_after(20);
    (*application)->cancel_wakeup_after();
    pump_run_loop(std::chrono::milliseconds(30));
    if (wakeup_count != 1) return 12;
    (*application)->request_wakeup();
    pump_run_loop(std::chrono::milliseconds(20));
    if (wakeup_count != 2) return 13;

    std::thread worker([&] { (*application)->request_wakeup_after(1); });
    worker.join();
    pump_run_loop(std::chrono::milliseconds(20));
    if (wakeup_count != 3) return 14;

    auto secondary_result = (*application)->create_window(
        cgpui::WindowDescriptor{
            .title = "CGPUI Cocoa Secondary Window",
            .size = cgpui::Size{160.0F, 120.0F}},
        [&](const cgpui::PlatformEvent& event) {
          if (std::holds_alternative<cgpui::WindowWakeupRequested>(event)) {
            ++secondary_wakeup_count;
          }
        });
    if (!secondary_result) return 15;
    secondary = std::move(*secondary_result);
    destroy_secondary_on_wakeup = true;
    (*application)->request_wakeup();
    pump_run_loop(std::chrono::milliseconds(20));
    if (secondary != nullptr || secondary_wakeup_count != 0 ||
        wakeup_count != 4) return 16;

    (*window)->request_close();
    if (!(*window)->resolve_close_request(
            cgpui::PlatformWindowCloseResolution::accept)) return 17;
    if ((*window)->lifecycle_state().native_window_created) return 18;
  }
  return 0;
}
