#include "macos_window_internal.hpp"
#include "macos_application_internal.hpp"
#include "macos_input_internal.hpp"

#include <algorithm>

namespace cgpui {

WindowState make_macos_window_state(NSWindow* window) {
  const NSRect points = [[window contentView] bounds];
  const NSRect pixels = [[window contentView] convertRectToBacking:points];
  return WindowState{
      .framebuffer_size = Size{
          static_cast<float>(NSWidth(pixels)),
          static_cast<float>(NSHeight(pixels))},
      .scale = DpiScale{static_cast<float>([window backingScaleFactor])},
      .ime_text_input_support = ImeTextInputSupport::available};
}

MacOSWindow::MacOSWindow(
    NSWindow* window,
    CGPUIMacOSContentView* content_view,
    CAMetalLayer* layer,
    CGPUIMacOSWindowDelegate* delegate,
    PlatformEventCallback callback,
    WindowState state,
    std::function<void(MacOSWindow*)> unregister)
    : window_(window),
      content_view_(content_view),
      layer_(layer),
      delegate_(delegate),
      callback_(std::move(callback)),
      state_(state),
      unregister_(std::move(unregister)) {}

MacOSWindow::~MacOSWindow() {
  if (unregister_) unregister_(this);
  release_pointer_capture(false);
  [content_view_ detachWindowAdapter];
  if (delegate_ != nil) [delegate_ detachWindow];
  [window_ setDelegate:nil];
  [window_ orderOut:nil];
  [window_ close];
}

NativeSurfaceHandle MacOSWindow::native_surface() const {
  return MetalSurfaceHandle{.layer = (__bridge void*)layer_};
}

WindowState MacOSWindow::state() const { return state_; }

void MacOSWindow::refresh_state() {
  const bool close_requested = state_.close_requested;
  const auto placement = state_.ime_text_input_placement;
  state_ = make_macos_window_state(window_);
  state_.close_requested = close_requested;
  state_.ime_text_input_placement = placement;
  [layer_ setContentsScale:[window_ backingScaleFactor]];
  [layer_ setDrawableSize:CGSizeMake(
      state_.framebuffer_size.width, state_.framebuffer_size.height)];
}

void MacOSWindow::request_redraw() {
  [[window_ contentView] setNeedsDisplay:YES];
  callback_(WindowRedrawRequested{});
}

void MacOSWindow::request_close() { close_requested(WindowCloseRequestSource::application); }

void MacOSWindow::set_title(std::string_view title) {
  NSString* value = [[NSString alloc] initWithBytes:title.data()
                                                length:title.size()
                                              encoding:NSUTF8StringEncoding];
  [window_ setTitle:value == nil ? @"" : value];
}

void MacOSWindow::set_ime_text_input_placement(
    std::optional<ImeTextInputPlacement> placement) {
  state_.ime_text_input_placement = std::move(placement);
  if (content_view_ != nil) [content_view_ setNeedsDisplay:YES];
}

void MacOSWindow::wakeup_requested() {
  auto callback = callback_;
  callback(WindowWakeupRequested{});
}

}  // namespace cgpui
