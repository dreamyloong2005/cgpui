#include "macos_input_internal.hpp"
#include "macos_text_services_internal.hpp"
#include "macos_window_internal.hpp"

#include "cgpui/platform/platform.hpp"

#import <AppKit/AppKit.h>

#include <string>
#include <variant>
#include <vector>

namespace {

using cgpui::ImeClauseStyle;

NSAttributedString* clause(NSString* text, NSInteger segment, NSUnderlineStyle underline) {
  return [[NSAttributedString alloc]
      initWithString:text
          attributes:@{NSMarkedClauseSegmentAttributeName : @(segment),
                       NSUnderlineStyleAttributeName : @(underline)}];
}

cgpui::ImeComposition composition_for(NSAttributedString* marked) {
  cgpui::ImeComposition composition{
      .text = marked == nil ? std::string("ni") : std::string([[marked string] UTF8String])};
  cgpui::macos_append_ime_clauses(composition, marked);
  return composition;
}

bool span_is(const cgpui::ImeComposition& composition, std::size_t index,
             std::uint32_t offset, std::uint32_t length, ImeClauseStyle style) {
  if (index >= composition.preedit_style_count) return false;
  const auto& span = composition.preedit_styles[index];
  return span.byte_offset == offset && span.byte_length == length && span.clause == style;
}

int test_conversion() {
  // No attributes, and a Pinyin-style single underlined run: one input clause.
  const auto plain = composition_for(nil);
  NSAttributedString* pinyin = [[NSAttributedString alloc]
      initWithString:@"ni hao"
          attributes:@{NSUnderlineStyleAttributeName : @(NSUnderlineStyleSingle)}];
  const auto typed = composition_for(pinyin);
  if (plain.preedit_style_count != 1 || !span_is(plain, 0, 0, 2, ImeClauseStyle::input) ||
      typed.preedit_style_count != 1 || !span_is(typed, 0, 0, 6, ImeClauseStyle::input)) {
    return 1;
  }
  // Japanese-style: the selected clause has a thick underline, the next one
  // a single underline. Offsets are UTF-8 bytes (3 per kana / kanji).
  NSMutableAttributedString* japanese = [[NSMutableAttributedString alloc] init];
  [japanese appendAttributedString:clause(@"日本語", 0, NSUnderlineStyleThick)];
  [japanese appendAttributedString:clause(@"を", 1, NSUnderlineStyleSingle)];
  const auto converted = composition_for(japanese);
  if (converted.preedit_style_count != 2 ||
      !span_is(converted, 0, 0, 9, ImeClauseStyle::selected_converted) ||
      !span_is(converted, 1, 9, 3, ImeClauseStyle::converted)) {
    return 2;
  }
  // More clauses than the span capacity: the rest merge into the last span.
  NSMutableAttributedString* many = [[NSMutableAttributedString alloc] init];
  for (NSInteger index = 0; index < 20; ++index) {
    [many appendAttributedString:clause(@"a", index, NSUnderlineStyleSingle)];
  }
  const auto merged = composition_for(many);
  return merged.preedit_style_count == cgpui::kImePreeditStyleSpanCapacity &&
                 span_is(merged, 15, 15, 5, ImeClauseStyle::converted)
             ? 0
             : 3;
}

}  // namespace

int main() {
  @autoreleasepool {
    if (const int conversion = test_conversion(); conversion != 0) return conversion;
    auto application = cgpui::create_platform_application();
    if (!application) return 10;
    std::vector<cgpui::PlatformEvent> events;
    auto window = (*application)->create_window(
        cgpui::WindowDescriptor{.title = "CGPUI macOS IME Clause Test",
                                .size = {240.0F, 120.0F}},
        [&](const cgpui::PlatformEvent& event) { events.push_back(event); });
    if (!window) return 11;
    auto* view = (CGPUIMacOSContentView*)[dynamic_cast<cgpui::MacOSWindow*>(window->get())
                                              ->native_window() contentView];
    NSArray* valid = [view validAttributesForMarkedText];
    if (![valid containsObject:NSUnderlineStyleAttributeName] ||
        ![valid containsObject:NSMarkedClauseSegmentAttributeName]) {
      return 12;
    }
    // The input method's attributed marked text reaches the app as clause
    // spans; the caret after the first clause is UTF-8 byte 9.
    NSMutableAttributedString* marked = [[NSMutableAttributedString alloc] init];
    [marked appendAttributedString:clause(@"日本語", 0, NSUnderlineStyleThick)];
    [marked appendAttributedString:clause(@"を", 1, NSUnderlineStyleSingle)];
    events.clear();
    [view setMarkedText:marked
          selectedRange:NSMakeRange(3, 0)
       replacementRange:NSMakeRange(NSNotFound, 0)];
    const cgpui::ImeComposition* composition = nullptr;
    for (const auto& event : events) {
      if (const auto* value = std::get_if<cgpui::ImeComposition>(&event)) composition = value;
    }
    if (composition == nullptr || composition->text != "日本語を" ||
        composition->preedit_cursor_begin != 9 || composition->preedit_cursor_end != 9 ||
        composition->preedit_style_count != 2 ||
        !span_is(*composition, 0, 0, 9, ImeClauseStyle::selected_converted) ||
        !span_is(*composition, 1, 9, 3, ImeClauseStyle::converted)) {
      return 13;
    }
    // A plain string (no attributes) still gives one input clause.
    events.clear();
    [view setMarkedText:@"ka" selectedRange:NSMakeRange(2, 0)
       replacementRange:NSMakeRange(NSNotFound, 0)];
    const auto* plain = events.empty() ? nullptr : std::get_if<cgpui::ImeComposition>(&events.back());
    return plain != nullptr && plain->preedit_style_count == 1 &&
                   span_is(*plain, 0, 0, 2, ImeClauseStyle::input)
               ? 0
               : 14;
  }
}
