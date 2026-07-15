#include "macos_input_internal.hpp"

#import <AppKit/AppKit.h>

#include <cmath>
#include <memory>
#include <variant>
#include <vector>

namespace {

bool nearly_equal(float lhs, float rhs) {
  return std::fabs(lhs - rhs) < 0.01F;
}

bool point_equal(cgpui::Point lhs, cgpui::Point rhs) {
  return nearly_equal(lhs.x, rhs.x) && nearly_equal(lhs.y, rhs.y);
}

}  // namespace

int main() {
  @autoreleasepool {
    const auto position =
        cgpui::macos_event_position_in_framebuffer(10.0, 20.0, 100.0, 2.0F);
    if (!point_equal(position, {10.0F, 80.0F})) return 1;
    if (cgpui::macos_mouse_button(0) != cgpui::MouseButton::left ||
        cgpui::macos_mouse_button(1) != cgpui::MouseButton::right ||
        cgpui::macos_mouse_button(2) != cgpui::MouseButton::middle ||
        cgpui::macos_mouse_button(3) != cgpui::MouseButton::back ||
        cgpui::macos_mouse_button(4) != cgpui::MouseButton::forward ||
        cgpui::macos_mouse_button(8) != cgpui::MouseButton::other) {
      return 2;
    }

    const auto precise = cgpui::macos_translate_scroll_event(
        12.0, -6.0, true, NSEventPhaseChanged, NSEventPhaseNone, 2.0F);
    if (!point_equal(precise.delta, {12.0F, -6.0F}) || !precise.precise) {
      return 3;
    }
    const auto line = cgpui::macos_translate_scroll_event(
        1.0, -2.0, false, NSEventPhaseNone, NSEventPhaseNone, 1.0F);
    if (!point_equal(line.delta, {1.0F, -2.0F}) || line.precise) return 4;
    if (!cgpui::macos_scroll_phase_active(NSEventPhaseBegan) ||
        !cgpui::macos_scroll_phase_active(NSEventPhaseChanged) ||
        cgpui::macos_scroll_phase_active(NSEventPhaseEnded) ||
        cgpui::macos_scroll_phase_active(NSEventPhaseNone)) {
      return 5;
    }

    const auto key = cgpui::macos_translate_key_event(
        0,
        "a",
        NSEventModifierFlagShift | NSEventModifierFlagCommand,
        true,
        true);
    if (key.key_code != 'A' || key.scan_code != 0 ||
        key.action != cgpui::KeyAction::pressed || !key.modifiers.shift ||
        !key.modifiers.super || key.modifiers.control || key.modifiers.alt ||
        !key.repeated || key.repeat_count != 2) {
      return 6;
    }
    const auto left = cgpui::macos_translate_key_event(
        123, "", 0, false, false);
    if (left.key_code != 0x25 || left.scan_code != 123 ||
        left.action != cgpui::KeyAction::released || left.repeated) {
      return 7;
    }

    const auto unicode = cgpui::macos_translate_key_event(
        14, "\xC3\xA9", 0, true, false);
    if (unicode.key_code != 0xE9) return 8;
    if (!cgpui::macos_modifier_key_pressed(
            56, NSEventModifierFlagShift) ||
        cgpui::macos_modifier_key_pressed(56, 0) ||
        !cgpui::macos_modifier_key_pressed(
            55, NSEventModifierFlagCommand)) {
      return 9;
    }

    auto application = cgpui::create_platform_application();
    if (!application) return 10;
    std::vector<cgpui::PointerMoved> moves;
    std::vector<cgpui::PointerButton> buttons;
    std::vector<cgpui::KeyboardKey> keys;
    std::vector<cgpui::PointerCaptureChanged> captures;
    auto window = (*application)->create_window(
        cgpui::WindowDescriptor{
            .title = "CGPUI macOS Input Event Test",
            .size = cgpui::Size{200.0F, 100.0F}},
        [&](const cgpui::PlatformEvent& event) {
          if (const auto* value = std::get_if<cgpui::PointerMoved>(&event)) {
            moves.push_back(*value);
          } else if (const auto* value =
                         std::get_if<cgpui::PointerButton>(&event)) {
            buttons.push_back(*value);
          } else if (const auto* value =
                         std::get_if<cgpui::KeyboardKey>(&event)) {
            keys.push_back(*value);
          } else if (const auto* value =
                         std::get_if<cgpui::PointerCaptureChanged>(&event)) {
            captures.push_back(*value);
          }
        });
    if (!window) return 11;

    NSWindow* native_window = nil;
    for (NSWindow* candidate in [NSApp windows]) {
      if ([[candidate title] isEqualToString:@"CGPUI macOS Input Event Test"]) {
        native_window = candidate;
        break;
      }
    }
    auto* content = (CGPUIMacOSContentView*)[native_window contentView];
    if (native_window == nil ||
        ![content isKindOfClass:[CGPUIMacOSContentView class]] ||
        ![content acceptsFirstResponder] ||
        [native_window firstResponder] != content) {
      return 12;
    }

    NSEvent* move = [NSEvent
        mouseEventWithType:NSEventTypeMouseMoved
                  location:NSMakePoint(10.0, 20.0)
             modifierFlags:0
                 timestamp:0.0
              windowNumber:[native_window windowNumber]
                   context:nil
               eventNumber:1
                clickCount:0
                  pressure:0.0];
    [content mouseMoved:move];
    [content mouseDragged:move];
    NSEvent* down = [NSEvent
        mouseEventWithType:NSEventTypeLeftMouseDown
                  location:NSMakePoint(10.0, 20.0)
             modifierFlags:0
                 timestamp:0.0
              windowNumber:[native_window windowNumber]
                   context:nil
               eventNumber:2
                clickCount:2
                  pressure:1.0];
    [content mouseDown:down];
    [content mouseUp:down];
    if (moves.size() != 2 ||
        !point_equal(moves.back().position, {10.0F, 80.0F}) ||
        buttons.size() != 2 || buttons[0].button != cgpui::MouseButton::left ||
        !buttons[0].pressed || buttons[1].pressed ||
        buttons[0].click_count != 2) {
      return 13;
    }

    NSEvent* key_down = [NSEvent
        keyEventWithType:NSEventTypeKeyDown
                 location:NSZeroPoint
            modifierFlags:NSEventModifierFlagControl
                timestamp:0.0
             windowNumber:[native_window windowNumber]
                  context:nil
               characters:@"b"
      charactersIgnoringModifiers:@"b"
                isARepeat:NO
                  keyCode:11];
    [content keyDown:key_down];
    [content keyUp:key_down];
    if (keys.size() != 2 || keys[0].key_code != 'B' ||
        !keys[0].modifiers.control ||
        keys[0].action != cgpui::KeyAction::pressed ||
        keys[1].action != cgpui::KeyAction::released) {
      return 14;
    }

    (*window)->set_cursor(cgpui::CursorShape::pointing_hand);
    if (cgpui::macos_cursor_for_shape(cgpui::CursorShape::pointing_hand) !=
        [NSCursor pointingHandCursor]) {
      return 15;
    }
    if (!(*window)->pointer_capture_state().supported ||
        (*window)->pointer_capture_state().captured) {
      return 16;
    }
    (*window)->set_pointer_capture(true);
    (*window)->set_pointer_capture(true);
    (*window)->set_pointer_capture(false);
    (*window)->set_pointer_capture(false);
    if (captures.size() != 2 || !captures[0].captured ||
        captures[1].captured || (*window)->pointer_capture_state().captured) {
      return 17;
    }
    (*window)->set_pointer_capture(true);
    (*window)->request_close();
    if (!(*window)->resolve_close_request(
            cgpui::PlatformWindowCloseResolution::accept) ||
        (*window)->pointer_capture_state().captured || captures.size() != 4 ||
        !captures[2].captured || captures[3].captured) {
      return 18;
    }
  }
  return 0;
}
