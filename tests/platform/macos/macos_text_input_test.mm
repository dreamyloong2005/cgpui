#include "macos_input_internal.hpp"
#include "macos_text_services_internal.hpp"

#include "cgpui/platform/platform.hpp"

#import <AppKit/AppKit.h>

#include <cmath>
#include <variant>
#include <vector>

namespace {

bool nearly_equal(float lhs, float rhs) {
  return std::fabs(lhs - rhs) < 0.01F;
}

}  // namespace

int main() {
  @autoreleasepool {
    const auto range = cgpui::macos_nsrange_to_utf8_bytes(
        "a\xC3\xA9\xF0\x9F\x98\x80z", 1, 3);
    if (range.first != 1 || range.second != 6) return 1;
    const cgpui::Rect screen = cgpui::macos_candidate_rect_to_screen(
        cgpui::Rect{.origin = {10.0F, 20.0F}, .size = {2.0F, 18.0F}},
        cgpui::Size{400.0F, 200.0F},
        cgpui::DpiScale{2.0F},
        cgpui::Point{100.0F, 50.0F});
    if (!nearly_equal(screen.origin.x, 110.0F) ||
        !nearly_equal(screen.origin.y, 112.0F) ||
        !nearly_equal(screen.size.width, 2.0F) ||
        !nearly_equal(screen.size.height, 18.0F)) {
      return 2;
    }

    auto application = cgpui::create_platform_application();
    if (!application) return 3;
    std::vector<cgpui::TextInput> text_events;
    std::vector<cgpui::ImeComposition> compositions;
    std::vector<cgpui::ImeDeleteSurroundingText> deletions;
    auto window = (*application)->create_window(
        cgpui::WindowDescriptor{
            .title = "CGPUI macOS Text Input Test",
            .size = {200.0F, 100.0F}},
        [&](const cgpui::PlatformEvent& event) {
          if (const auto* value = std::get_if<cgpui::TextInput>(&event)) {
            text_events.push_back(*value);
          } else if (const auto* value =
                         std::get_if<cgpui::ImeComposition>(&event)) {
            compositions.push_back(*value);
          } else if (const auto* value =
                         std::get_if<cgpui::ImeDeleteSurroundingText>(&event)) {
            deletions.push_back(*value);
          }
        });
    if (!window) return 4;
    (*window)->set_ime_text_input_placement(cgpui::ImeTextInputPlacement{
        .rect = {.origin = {10.0F, 20.0F}, .size = {2.0F, 18.0F}},
        .candidate_rect = cgpui::Rect{
            .origin = {12.0F, 22.0F}, .size = {3.0F, 16.0F}},
        .byte_offset = 3,
        .surrounding_text = "a\xC3\xA9\xF0\x9F\x98\x80z",
        .selection_anchor = 3});

    NSWindow* native_window = nil;
    for (NSWindow* candidate in [NSApp windows]) {
      if ([[candidate title] isEqualToString:@"CGPUI macOS Text Input Test"]) {
        native_window = candidate;
        break;
      }
    }
    auto* content = (CGPUIMacOSContentView*)[native_window contentView];
    if (native_window == nil ||
        ![content conformsToProtocol:@protocol(NSTextInputClient)]) {
      return 5;
    }

    [content insertText:@"X" replacementRange:NSMakeRange(1, 3)];
    if (text_events.size() != 1 || text_events[0].text != "X" ||
        deletions.size() != 1 || deletions[0].before_length != 2 ||
        deletions[0].after_length != 4) {
      return 6;
    }

    [content setMarkedText:@"\u6F22\u5B57"
             selectedRange:NSMakeRange(1, 0)
           replacementRange:NSMakeRange(NSNotFound, 0)];
    if (![content hasMarkedText] || compositions.size() != 1 ||
        compositions[0].phase != cgpui::ImeCompositionPhase::update ||
        compositions[0].text != "\xE6\xBC\xA2\xE5\xAD\x97" ||
        compositions[0].preedit_cursor_begin != 3 ||
        compositions[0].preedit_cursor_end != 3 ||
        compositions[0].preedit_style_count != 1) {
      return 7;
    }
    const NSRange marked_range = [content markedRange];
    const NSRange selected_range = [content selectedRange];
    if (marked_range.location != 2 || marked_range.length != 2 ||
        selected_range.location != 3 || selected_range.length != 0 ||
        [content characterIndexForPoint:NSMakePoint(0.0, 0.0)] != 3) {
      return 8;
    }
    [content insertText:@"\u78BA\u5B9A"
       replacementRange:NSMakeRange(NSNotFound, 0)];
    if ([content hasMarkedText] || compositions.size() != 2 ||
        compositions[1].phase != cgpui::ImeCompositionPhase::commit ||
        compositions[1].text != "\xE7\xA2\xBA\xE5\xAE\x9A" ||
        text_events.size() != 1) {
      return 9;
    }
    [content setMarkedText:@"q"
             selectedRange:NSMakeRange(1, 0)
           replacementRange:NSMakeRange(NSNotFound, 0)];
    [content unmarkText];
    if (compositions.size() != 4 ||
        compositions.back().phase != cgpui::ImeCompositionPhase::cancel) {
      return 10;
    }

    [content doCommandBySelector:@selector(deleteBackward:)];
    if (deletions.size() != 2 || deletions.back().before_length != 2 ||
        deletions.back().after_length != 0) {
      return 11;
    }
    NSRange actual = NSMakeRange(NSNotFound, 0);
    NSAttributedString* substring =
        [content attributedSubstringForProposedRange:NSMakeRange(1, 1)
                                         actualRange:&actual];
    if (![[substring string] isEqualToString:@"\u00E9"] ||
        actual.location != 1 || actual.length != 1) {
      return 12;
    }
    const NSRect candidate =
        [content firstRectForCharacterRange:NSMakeRange(0, 0)
                                 actualRange:&actual];
    if (NSIsEmptyRect(candidate)) return 13;

    const auto diagnostics = cgpui::macos_text_services_diagnostics(**window);
    if (diagnostics.committed_text_count != 2 ||
        diagnostics.marked_text_update_count != 2 ||
        diagnostics.unmark_count != 1 ||
        diagnostics.delete_surrounding_count != 2) {
      return 14;
    }
  }
  return 0;
}
