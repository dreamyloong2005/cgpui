#include "macos_window_internal.hpp"

#include <dispatch/dispatch.h>

#include <optional>

namespace cgpui {
namespace {

bool client_title_bar(WindowChromeOptions options) {
  return options.decorations && options.title_bar == WindowTitleBarMode::client;
}

NSWindowStyleMask style_mask(WindowChromeOptions options) {
  if (!options.decorations) return NSWindowStyleMaskBorderless;
  NSWindowStyleMask mask = NSWindowStyleMaskClosable |
      NSWindowStyleMaskMiniaturizable;
  if (options.titlebar_visible) mask |= NSWindowStyleMaskTitled;
  if (options.resizable) mask |= NSWindowStyleMaskResizable;
  // Client title bar: the content view covers the whole window and the
  // native traffic lights stay over it (titled, so they keep working).
  if (client_title_bar(options)) {
    mask |= NSWindowStyleMaskTitled | NSWindowStyleMaskFullSizeContentView;
  }
  return mask;
}

}  // namespace

PlatformWindowChromeState MacOSWindow::apply_window_chrome(
    WindowChromeOptions options) {
  if (![NSThread isMainThread]) {
    __block std::optional<PlatformWindowChromeState> result;
    dispatch_sync(dispatch_get_main_queue(), ^{
      result = apply_window_chrome(options);
    });
    return std::move(*result);
  }
  PlatformWindowChromeState result{
      .supported = window_ != nil,
      .decoration_control_supported = true,
      .transparency_supported = true,
      .backend = "macos",
      .requested = options,
      .applied = options};
  if (window_ == nil) {
    result.reason = "native window closed";
    return result;
  }
  const NSSize content_size = [[window_ contentView] bounds].size;
  [window_ setStyleMask:style_mask(options)];
  client_title_bar_ = client_title_bar(options);
  if (!client_title_bar_) pressed_control_.reset();
  const bool title_shown = options.titlebar_visible && !client_title_bar_;
  [window_ setTitleVisibility:title_shown ? NSWindowTitleVisible : NSWindowTitleHidden];
  [window_ setTitlebarAppearsTransparent:!title_shown];
  [window_ setOpaque:!options.transparent_background];
  [window_ setBackgroundColor:options.transparent_background
      ? [NSColor clearColor] : [NSColor windowBackgroundColor]];
  // A shown backdrop needs the renderer's transparent surface; keep it.
  if (options.transparent_background) {
    [layer_ setOpaque:NO];
  } else if (backdrop_state_.applied == WindowBackdrop::none) {
    [layer_ setOpaque:YES];
  }
  [window_ setContentSize:content_size];
  refresh_state();
  return result;
}

}  // namespace cgpui
