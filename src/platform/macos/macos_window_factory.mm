#include "macos_application_internal.hpp"
#include "macos_input_internal.hpp"
#include "macos_window_internal.hpp"

namespace cgpui {
namespace {

NSString* ns_string(std::string_view value) {
  return [[NSString alloc] initWithBytes:value.data()
                                  length:value.size()
                                encoding:NSUTF8StringEncoding];
}

NSUInteger style_for(WindowChromeOptions options) {
  NSUInteger style = NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;
  if (options.titlebar_visible) style |= NSWindowStyleMaskTitled;
  if (options.resizable) style |= NSWindowStyleMaskResizable;
  if (!options.decorations) style = NSWindowStyleMaskBorderless;
  return style;
}

}  // namespace

Result<std::unique_ptr<MacOSWindow>> create_macos_window(
    const WindowDescriptor& descriptor,
    NSWindow* parent,
    PlatformEventCallback callback,
    std::function<void(MacOSWindow*)> unregister) {
  NSRect frame = NSMakeRect(
      descriptor.position.has_value() ? descriptor.position->x : 0.0,
      descriptor.position.has_value() ? descriptor.position->y : 0.0,
      descriptor.size.width,
      descriptor.size.height);
  NSWindow* window = [[NSWindow alloc] initWithContentRect:frame
                                                 styleMask:style_for(descriptor.chrome)
                                                   backing:NSBackingStoreBuffered
                                                     defer:NO];
  if (window == nil) {
    return std::unexpected(Error{
        .code = ErrorCode::window_creation_failed,
        .message = "NSWindow creation failed"});
  }
  [window setReleasedWhenClosed:NO];
  NSString* title = ns_string(descriptor.title);
  [window setTitle:title == nil ? @"" : title];
  CGPUIMacOSContentView* content = [[CGPUIMacOSContentView alloc]
      initWithFrame:NSMakeRect(0.0, 0.0, descriptor.size.width, descriptor.size.height)
      windowAdapter:nil];
  if (content == nil) {
    return std::unexpected(Error{
        .code = ErrorCode::window_creation_failed,
        .message = "Cocoa content view creation failed"});
  }
  [window setContentView:content];
  [content setWantsLayer:YES];
  CAMetalLayer* layer = [CAMetalLayer layer];
  layer.opaque = YES;
  layer.contentsScale = [window backingScaleFactor];
  content.layer = layer;
  WindowState state = make_macos_window_state(window);
  layer.drawableSize = CGSizeMake(
      state.framebuffer_size.width, state.framebuffer_size.height);
  CGPUIMacOSWindowDelegate* delegate =
      [[CGPUIMacOSWindowDelegate alloc] initWithWindow:nil];
  auto result = std::make_unique<MacOSWindow>(
      window,
      content,
      layer,
      delegate,
      std::move(callback),
      state,
      std::move(unregister));
  [content attachWindowAdapter:result.get()];
  [delegate attachWindow:result.get()];
  [window setDelegate:delegate];
  if (parent != nil) [parent addChildWindow:window ordered:NSWindowAbove];
  if (!descriptor.position.has_value()) [window center];
  [window makeKeyAndOrderFront:nil];
  [window makeFirstResponder:content];
  [[NSApplication sharedApplication] activateIgnoringOtherApps:YES];
  return result;
}

}  // namespace cgpui
