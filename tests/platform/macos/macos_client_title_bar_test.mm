#include "macos_input_internal.hpp"
#include "macos_window_internal.hpp"
#include "macos_window_title_bar_internal.hpp"

#include "cgpui/platform/platform.hpp"

#import <AppKit/AppKit.h>

#include <variant>
#include <vector>

namespace {

using cgpui::WindowControlArea;

NSEvent* mouse(NSWindow* window, NSEventType type, cgpui::Point logical) {
  const CGFloat height = NSHeight([[window contentView] bounds]);
  return [NSEvent mouseEventWithType:type
                            location:NSMakePoint(logical.x, height - logical.y)
                       modifierFlags:0
                           timestamp:0
                        windowNumber:[window windowNumber]
                             context:nil
                         eventNumber:0
                          clickCount:1
                            pressure:1];
}

int test_helpers() {
  const std::vector<cgpui::WindowControlRegion> regions{
      {WindowControlArea::caption, {{0, 0}, {200, 32}}},
      {WindowControlArea::close_button, {{160, 0}, {40, 32}}},
  };
  using cgpui::macos_window_control_area_at;
  if (macos_window_control_area_at(regions, {10, 10}) != WindowControlArea::caption ||
      macos_window_control_area_at(regions, {170, 10}) != WindowControlArea::close_button ||
      macos_window_control_area_at(regions, {10, 40}).has_value()) {
    return 1;
  }
  using cgpui::MacOSTitleBarDoubleClick;
  using cgpui::macos_title_bar_double_click_action;
  return macos_title_bar_double_click_action(nil) == MacOSTitleBarDoubleClick::zoom &&
                 macos_title_bar_double_click_action(@"Maximize") ==
                     MacOSTitleBarDoubleClick::zoom &&
                 macos_title_bar_double_click_action(@"Minimize") ==
                     MacOSTitleBarDoubleClick::minimize &&
                 macos_title_bar_double_click_action(@"None") ==
                     MacOSTitleBarDoubleClick::none
             ? 0
             : 2;
}

}  // namespace

int main() {
  @autoreleasepool {
    if (const int helpers = test_helpers(); helpers != 0) return helpers;
    auto application = cgpui::create_platform_application();
    if (!application) return 10;
    std::vector<cgpui::PlatformEvent> events;
    auto window = (*application)->create_window(
        cgpui::WindowDescriptor{
            .title = "CGPUI macOS Title Bar Test",
            .size = {320.0F, 200.0F},
            .chrome = {.title_bar = cgpui::WindowTitleBarMode::client}},
        [&](const cgpui::PlatformEvent& event) { events.push_back(event); });
    if (!window) return 11;
    auto* macos = dynamic_cast<cgpui::MacOSWindow*>(window->get());
    NSWindow* native = macos->native_window();
    NSView* content = [native contentView];

    // The content view covers the whole frame; the title is hidden and the
    // traffic lights stay native.
    if (([native styleMask] & NSWindowStyleMaskFullSizeContentView) == 0 ||
        [native titleVisibility] != NSWindowTitleHidden ||
        ![native titlebarAppearsTransparent] ||
        NSHeight([content bounds]) != NSHeight([native frame]) ||
        NSWidth([content bounds]) != 320.0 ||
        [[native standardWindowButton:NSWindowCloseButton] isHidden]) {
      return 12;
    }
    const cgpui::PlatformWindowFrameMetrics metrics = (*window)->frame_metrics();
    if (!metrics.supported || !metrics.client_title_bar || metrics.backend != "macos" ||
        metrics.caption_height < 20.0F || metrics.caption_height > 60.0F ||
        metrics.caption_button_size.width <= 0.0F ||
        metrics.caption_buttons_leading_reserved_width < 40.0F ||
        metrics.caption_buttons_leading_reserved_width > 120.0F ||
        metrics.caption_buttons_reserved_width != 0.0F ||
        metrics.client_resize_border.top != 0.0F) {
      return 13;
    }
    if ((*window)->perform_window_action({.action = cgpui::PlatformWindowAction::show_window_menu})
            .supported ||
        (*window)->perform_window_action({.action = cgpui::PlatformWindowAction::begin_move})
            .performed) {
      return 14;
    }

    // A close button release over the region it was pressed on closes
    // through the close policy; a release elsewhere does nothing. The app
    // sees both pointer events first.
    (*window)->set_window_control_regions({
        {WindowControlArea::caption, {{0, 0}, {320, 32}}},
        {WindowControlArea::close_button, {{280, 0}, {40, 32}}},
    });
    auto* view = (CGPUIMacOSContentView*)content;
    [view mouseDown:mouse(native, NSEventTypeLeftMouseDown, {300, 16})];
    [view mouseUp:mouse(native, NSEventTypeLeftMouseUp, {300, 60})];
    const auto closes = [&] {
      std::size_t count = 0;
      for (const auto& event : events) {
        count += std::holds_alternative<cgpui::WindowCloseRequested>(event) ? 1 : 0;
      }
      return count;
    };
    if (closes() != 0) return 15;
    events.clear();
    [view mouseDown:mouse(native, NSEventTypeLeftMouseDown, {300, 16})];
    [view mouseUp:mouse(native, NSEventTypeLeftMouseUp, {305, 20})];
    std::size_t buttons = 0;
    for (const auto& event : events) {
      buttons += std::holds_alternative<cgpui::PointerButton>(event) ? 1 : 0;
    }
    if (buttons != 2 || closes() != 1) return 16;

    // Back to the system title bar.
    (void)(*window)->apply_window_chrome({});
    return ([native styleMask] & NSWindowStyleMaskFullSizeContentView) == 0 &&
                   [native titleVisibility] == NSWindowTitleVisible &&
                   !(*window)->frame_metrics().client_title_bar &&
                   (*window)->frame_metrics().caption_buttons_leading_reserved_width == 0.0F
               ? 0
               : 17;
  }
}
