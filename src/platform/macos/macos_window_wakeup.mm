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
    std::uint64_t delay_ms) {
  if ([NSThread isMainThread]) {
    [target scheduleDelayedWakeup:@(delay_ms)];
  } else {
    [target performSelectorOnMainThread:@selector(scheduleDelayedWakeup:)
                             withObject:@(delay_ms)
                          waitUntilDone:NO];
  }
}

void macos_cancel_wakeup(CGPUIMacOSWakeupTarget* target) {
  if ([NSThread isMainThread]) {
    [target cancelDelayedWakeup];
  } else {
    [target performSelectorOnMainThread:@selector(cancelDelayedWakeup)
                             withObject:nil
                          waitUntilDone:NO];
  }
}

void MacOSApplication::request_wakeup() { macos_request_wakeup(wakeup_target_); }

void MacOSApplication::request_wakeup_after(std::uint64_t delay_ms) {
  macos_request_wakeup_after(wakeup_target_, delay_ms);
}

void MacOSApplication::cancel_wakeup_after() {
  macos_cancel_wakeup(wakeup_target_);
}

}  // namespace cgpui
