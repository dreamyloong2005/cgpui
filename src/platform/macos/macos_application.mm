#include "cgpui/platform/platform.hpp"

#import <AppKit/AppKit.h>
#import <QuartzCore/CAMetalLayer.h>

#include <expected>
#include <memory>
#include <string_view>
#include <utility>
#include <vector>

namespace cgpui {
class MacOSWindow;
} // namespace cgpui

@interface CGPUIMacOSWindowDelegate : NSObject <NSWindowDelegate> {
 @private
  void* window_;
}
- (instancetype)initWithWindow:(void*)window;
- (void)attachWindow:(void*)window;
- (void)detachWindow;
@end

namespace cgpui {
namespace {

NSString* make_ns_string(std::string_view value) {
  return [[NSString alloc] initWithBytes:value.data()
                                  length:value.size()
                                encoding:NSUTF8StringEncoding];
}

WindowState make_window_state(NSWindow* window) {
  const NSRect points = [[window contentView] bounds];
  const NSRect pixels = [[window contentView] convertRectToBacking:points];
  const CGFloat scale = [window backingScaleFactor];
  return WindowState{
      .framebuffer_size = Size{
          static_cast<float>(NSWidth(pixels)),
          static_cast<float>(NSHeight(pixels)),
      },
      .scale = DpiScale{static_cast<float>(scale)},
      .close_requested = false,
  };
}

} // namespace

class MacOSWindow final : public PlatformWindow {
 public:
  MacOSWindow(NSWindow* window,
              CAMetalLayer* layer,
              CGPUIMacOSWindowDelegate* delegate,
              PlatformEventCallback callback,
              WindowState state)
      : window_([window retain]),
        layer_([layer retain]),
        delegate_(delegate),
        callback_(std::move(callback)),
        state_(state) {}

  ~MacOSWindow() override {
    [delegate_ detachWindow];
    [window_ setDelegate:nil];
    [window_ close];
    [layer_ release];
    [window_ release];
  }

  [[nodiscard]] NativeSurfaceHandle native_surface() const override {
    return MetalSurfaceHandle{.layer = layer_};
  }

  [[nodiscard]] WindowState state() const override { return state_; }

  void request_redraw() override { redraw_requested(); }

  void set_title(std::string_view title) override {
    NSString* ns_title = make_ns_string(title);
    [window_ setTitle:ns_title];
    [ns_title release];
  }

  void refresh_state() {
    const bool close_requested = state_.close_requested;
    state_ = make_window_state(window_);
    state_.close_requested = close_requested;
    [layer_ setContentsScale:[window_ backingScaleFactor]];
    [layer_ setDrawableSize:CGSizeMake(state_.framebuffer_size.width,
                                       state_.framebuffer_size.height)];
  }

  void close_requested() {
    state_.close_requested = true;
    callback_(WindowCloseRequested{});
  }

  void resized() {
    refresh_state();
    callback_(WindowResized{.size = state_.framebuffer_size, .scale = state_.scale});
  }

  void redraw_requested() { callback_(WindowRedrawRequested{}); }

 private:
  NSWindow* window_ = nil;
  CAMetalLayer* layer_ = nil;
  CGPUIMacOSWindowDelegate* delegate_ = nil;
  PlatformEventCallback callback_;
  WindowState state_;
};

} // namespace cgpui

@implementation CGPUIMacOSWindowDelegate

- (instancetype)initWithWindow:(void*)window {
  self = [super init];
  if (self != nil) {
    window_ = window;
  }
  return self;
}

- (void)attachWindow:(void*)window {
  window_ = window;
}

- (void)detachWindow {
  window_ = nullptr;
}

- (BOOL)windowShouldClose:(id)sender {
  (void)sender;
  if (window_ != nullptr) {
    static_cast<cgpui::MacOSWindow*>(window_)->close_requested();
  }
  return NO;
}

- (void)windowDidResize:(NSNotification*)notification {
  (void)notification;
  if (window_ != nullptr) {
    static_cast<cgpui::MacOSWindow*>(window_)->resized();
  }
}

- (void)windowDidChangeBackingProperties:(NSNotification*)notification {
  (void)notification;
  if (window_ != nullptr) {
    static_cast<cgpui::MacOSWindow*>(window_)->resized();
  }
}

@end

namespace cgpui {
namespace {

class MacOSApplication final : public PlatformApplication {
 public:
  Result<std::unique_ptr<PlatformWindow>> create_window(
      const WindowDescriptor& descriptor,
      PlatformEventCallback callback) override {
    NSApplication* app = [NSApplication sharedApplication];
    [app setActivationPolicy:NSApplicationActivationPolicyRegular];

    const NSRect frame =
        NSMakeRect(0.0,
                   0.0,
                   static_cast<CGFloat>(descriptor.size.width),
                   static_cast<CGFloat>(descriptor.size.height));
    NSWindow* window =
        [[NSWindow alloc] initWithContentRect:frame
                                    styleMask:NSWindowStyleMaskTitled |
                                              NSWindowStyleMaskClosable |
                                              NSWindowStyleMaskResizable |
                                              NSWindowStyleMaskMiniaturizable
                                      backing:NSBackingStoreBuffered
                                        defer:NO];
    if (window == nil) {
      return std::unexpected(Error{
          .code = ErrorCode::window_creation_failed,
          .message = "NSWindow creation failed",
      });
    }

    [window setReleasedWhenClosed:NO];
    NSString* title = make_ns_string(descriptor.title);
    [window setTitle:title];
    [title release];

    NSView* content_view = [window contentView];
    [content_view setWantsLayer:YES];
    CAMetalLayer* metal_layer = [CAMetalLayer layer];
    [metal_layer setOpaque:YES];
    [metal_layer setContentsScale:[window backingScaleFactor]];
    [content_view setLayer:metal_layer];

    auto state = make_window_state(window);
    [metal_layer setDrawableSize:CGSizeMake(state.framebuffer_size.width,
                                           state.framebuffer_size.height)];

    auto* delegate =
        [[CGPUIMacOSWindowDelegate alloc] initWithWindow:nullptr];
    auto platform_window = std::make_unique<MacOSWindow>(
        window, metal_layer, delegate, std::move(callback), state);
    [delegate attachWindow:platform_window.get()];
    [window setDelegate:delegate];
    delegates_.emplace_back(delegate);

    [window center];
    [window makeKeyAndOrderFront:nil];
    [app activateIgnoringOtherApps:YES];

    [window release];
    return platform_window;
  }

  int run() override {
    running_ = true;
    while (running_) {
      @autoreleasepool {
        NSEvent* event =
            [NSApp nextEventMatchingMask:NSEventMaskAny
                               untilDate:[NSDate distantFuture]
                                  inMode:NSDefaultRunLoopMode
                                 dequeue:YES];
        if (event != nil) {
          [NSApp sendEvent:event];
          [NSApp updateWindows];
        }
      }
    }
    return 0;
  }

  void quit() override {
    running_ = false;
    [NSApp stop:nil];
  }

 private:
  struct DelegateReleaser {
    void operator()(CGPUIMacOSWindowDelegate* delegate) const {
      [delegate detachWindow];
      [delegate release];
    }
  };

  std::vector<std::unique_ptr<CGPUIMacOSWindowDelegate, DelegateReleaser>> delegates_;
  bool running_ = true;
};

} // namespace

Result<std::unique_ptr<PlatformApplication>> create_platform_application() {
  return std::make_unique<MacOSApplication>();
}

} // namespace cgpui
