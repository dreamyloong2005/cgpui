#include "macos_window_internal.hpp"

#include <dispatch/dispatch.h>

#include <optional>

namespace cgpui {
namespace {

NSWindowStyleMask style_mask(WindowChromeOptions options) {
  if (!options.decorations) return NSWindowStyleMaskBorderless;
  NSWindowStyleMask mask = NSWindowStyleMaskClosable |
      NSWindowStyleMaskMiniaturizable;
  if (options.titlebar_visible) mask |= NSWindowStyleMaskTitled;
  if (options.resizable) mask |= NSWindowStyleMaskResizable;
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
  [window_ setTitleVisibility:options.titlebar_visible
      ? NSWindowTitleVisible : NSWindowTitleHidden];
  [window_ setTitlebarAppearsTransparent:!options.titlebar_visible];
  [window_ setOpaque:!options.transparent_background];
  [window_ setBackgroundColor:options.transparent_background
      ? [NSColor clearColor] : [NSColor windowBackgroundColor]];
  [layer_ setOpaque:!options.transparent_background];
  [window_ setContentSize:content_size];
  refresh_state();
  return result;
}

}  // namespace cgpui
