#pragma once

#include "cgpui/platform/platform.hpp"

#import <AppKit/AppKit.h>

#include <memory>
#include <functional>
#include <vector>

@class CGPUIMacOSApplicationDelegate;
@interface CGPUIMacOSWakeupTarget : NSObject
@property(nonatomic, assign) void* application;
- (void)dispatchWakeup;
- (void)dispatchDelayedWakeup:(NSTimer*)timer;
@end

namespace cgpui {

class MacOSWindow;

class MacOSApplication final : public PlatformApplication {
 public:
  MacOSApplication();
  ~MacOSApplication() override;

  Result<std::unique_ptr<PlatformWindow>> create_window(
      const WindowDescriptor& descriptor,
      PlatformEventCallback callback) override;
  Result<std::unique_ptr<PlatformWindow>> create_child_window(
      const WindowDescriptor& descriptor,
      PlatformWindow& parent,
      PlatformEventCallback callback) override;
  std::uint64_t monotonic_time_ms() const override;
  void request_wakeup() override;
  void request_wakeup_after(std::uint64_t delay_ms) override;
  void cancel_wakeup_after() override;
  PlatformReopenResult request_reopen() override;
  int run() override;
  void quit() override;

  void dispatch_wakeup();
  void reopened();
  void register_window(MacOSWindow* window);
  void unregister_window(MacOSWindow* window);

 private:
  std::vector<MacOSWindow*> windows_;
  __strong CGPUIMacOSApplicationDelegate* delegate_ = nil;
  __strong CGPUIMacOSWakeupTarget* wakeup_target_ = nil;
  __strong NSTimer* wakeup_timer_ = nil;
  bool running_ = true;
  bool launched_ = false;
};

Result<std::unique_ptr<MacOSWindow>> create_macos_window(
    const WindowDescriptor& descriptor,
    NSWindow* parent,
    PlatformEventCallback callback,
    std::function<void(MacOSWindow*)> unregister);
int macos_run_event_loop(bool& launched);
void macos_stop_event_loop();
void macos_request_wakeup(CGPUIMacOSWakeupTarget* target);
void macos_request_wakeup_after(
    CGPUIMacOSWakeupTarget* target,
    NSTimer* __strong* timer,
    std::uint64_t delay_ms);
void macos_cancel_wakeup(CGPUIMacOSWakeupTarget* target, NSTimer* __strong* timer);

Result<std::unique_ptr<PlatformApplication>> create_platform_application();

}  // namespace cgpui
