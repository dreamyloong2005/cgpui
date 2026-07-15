#include "macos_application_internal.hpp"
#include "macos_window_internal.hpp"

namespace cgpui {

void MacOSApplication::dispatch_wakeup() {
  for (MacOSWindow* window : windows_) {
    if (window != nullptr) window->wakeup_requested();
  }
}

void macos_request_wakeup(CGPUIMacOSWakeupTarget* target) {
  [target performSelectorOnMainThread:@selector(dispatchWakeup)
                           withObject:nil
                        waitUntilDone:NO];
}

void macos_request_wakeup_after(
    CGPUIMacOSWakeupTarget* target,
    NSTimer* __strong* timer,
    std::uint64_t delay_ms) {
  if (*timer != nil) [*timer invalidate];
  *timer = [NSTimer scheduledTimerWithTimeInterval:
                                  static_cast<NSTimeInterval>(delay_ms) / 1000.0
                                            target:target
                                          selector:@selector(dispatchDelayedWakeup:)
                                          userInfo:nil
                                           repeats:NO];
}

void macos_cancel_wakeup(CGPUIMacOSWakeupTarget* target, NSTimer* __strong* timer) {
  (void)target;
  if (*timer != nil) {
    [*timer invalidate];
    *timer = nil;
  }
}

void MacOSApplication::request_wakeup() { macos_request_wakeup(wakeup_target_); }

void MacOSApplication::request_wakeup_after(std::uint64_t delay_ms) {
  macos_request_wakeup_after(wakeup_target_, &wakeup_timer_, delay_ms);
}

void MacOSApplication::cancel_wakeup_after() {
  macos_cancel_wakeup(wakeup_target_, &wakeup_timer_);
}

}  // namespace cgpui
