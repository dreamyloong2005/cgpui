#pragma once

#include "cgpui/platform/platform.hpp"

#import <AppKit/AppKit.h>

#include <cstdint>
#include <string>

@interface CGPUIMacOSContentView : NSView <NSTextInputClient> {
 @private
  void* window_adapter_;
  __strong NSTrackingArea* tracking_area_;
}
- (instancetype)initWithFrame:(NSRect)frame windowAdapter:(void*)windowAdapter;
- (void)attachWindowAdapter:(void*)windowAdapter;
- (void)detachWindowAdapter;
- (void)dispatchCapturedMouseEvent:(NSEvent*)event;
@end

namespace cgpui {

[[nodiscard]] Point macos_event_position_in_framebuffer(
    double x,
    double y,
    double view_height,
    float scale);
[[nodiscard]] MouseButton macos_mouse_button(std::uint64_t button_number);
[[nodiscard]] bool macos_scroll_phase_active(std::uint64_t phase);
[[nodiscard]] PointerScrolled macos_translate_scroll_event(
    double dx,
    double dy,
    bool precise,
    std::uint64_t phase,
    std::uint64_t momentum,
    float scale);
[[nodiscard]] KeyboardKey macos_translate_key_event(
    std::uint16_t key_code,
    std::string text,
    std::uint64_t flags,
    bool pressed,
    bool repeat);
[[nodiscard]] bool macos_modifier_key_pressed(
    std::uint16_t key_code,
    std::uint64_t flags);
[[nodiscard]] NSCursor* macos_cursor_for_shape(CursorShape cursor_shape);

}  // namespace cgpui
