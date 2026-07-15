#include "macos_window_internal.hpp"

#include <algorithm>
#include <string>

namespace cgpui {
namespace {

std::string utf8(NSString* value) {
  if (value == nil || [value length] == 0) return {};
  const char* bytes = [value UTF8String];
  return bytes == nullptr ? std::string{} : std::string(bytes);
}

std::size_t utf16_cursor(std::string_view value, std::size_t byte_index) {
  const std::size_t count = std::min(byte_index, value.size());
  NSString* prefix = [[NSString alloc] initWithBytes:value.data()
                                               length:count
                                             encoding:NSUTF8StringEncoding];
  return prefix == nil ? 0 : [prefix length];
}

}  // namespace

std::pair<std::size_t, std::size_t> macos_nsrange_to_utf8_bytes(
    std::string_view text,
    std::size_t location,
    std::size_t length) {
  NSString* value = [[NSString alloc] initWithBytes:text.data()
                                              length:text.size()
                                            encoding:NSUTF8StringEncoding];
  if (value == nil) return {0, 0};
  const std::size_t total = [value length];
  const std::size_t start = std::min(location, total);
  const std::size_t count = std::min(length, total - start);
  const std::size_t offset = [[value substringToIndex:start]
      lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
  const std::size_t bytes = [[value substringWithRange:NSMakeRange(start, count)]
      lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
  return {offset, bytes};
}

void MacOSWindow::text_insert(NSString* string, NSRange replacement_range) {
  if (replacement_range.location != NSNotFound &&
      state_.ime_text_input_placement.has_value()) {
    const auto& placement = *state_.ime_text_input_placement;
    const auto range = macos_nsrange_to_utf8_bytes(
        placement.surrounding_text,
        replacement_range.location,
        replacement_range.length);
    const std::size_t before = placement.byte_offset > range.first
        ? placement.byte_offset - range.first : 0;
    const std::size_t end = range.first + range.second;
    const std::size_t after = end > placement.byte_offset
        ? end - placement.byte_offset : 0;
    if (before != 0 || after != 0) {
      ++text_input_state_.diagnostics.delete_surrounding_count;
      callback_(ImeDeleteSurroundingText{
          .before_length = static_cast<std::uint32_t>(before),
          .after_length = static_cast<std::uint32_t>(after)});
    }
  }
  const std::string value = utf8(string);
  if (!text_input_state_.marked_text.empty()) {
    callback_(ImeComposition{
        .phase = ImeCompositionPhase::commit,
        .text = value});
    text_input_state_.marked_text.clear();
    text_input_state_.marked_cursor = 0;
    text_input_state_.marked_selection_location_utf16 = 0;
    text_input_state_.marked_selection_length_utf16 = 0;
    ++text_input_state_.diagnostics.committed_text_count;
    return;
  }
  if (!value.empty()) {
    callback_(TextInput{.text = value});
    ++text_input_state_.diagnostics.committed_text_count;
  }
}

void MacOSWindow::text_set_marked(
    NSString* string,
    NSRange selected_range,
    NSRange replacement_range) {
  if (replacement_range.location != NSNotFound) {
    text_insert(@"", replacement_range);
  }
  text_input_state_.marked_text = utf8(string);
  const auto selected_bytes = macos_nsrange_to_utf8_bytes(
      text_input_state_.marked_text,
      selected_range.location,
      selected_range.length);
  text_input_state_.marked_cursor = selected_bytes.first;
  text_input_state_.marked_selection_location_utf16 = selected_range.location;
  text_input_state_.marked_selection_length_utf16 = selected_range.length;
  text_input_state_.marked_document_location_utf16 =
      state_.ime_text_input_placement.has_value()
      ? utf16_cursor(
            state_.ime_text_input_placement->surrounding_text,
            state_.ime_text_input_placement->byte_offset)
      : 0;
  ++text_input_state_.diagnostics.marked_text_update_count;
  ImeComposition composition{
      .phase = ImeCompositionPhase::update,
      .text = text_input_state_.marked_text,
      .preedit_cursor_begin = static_cast<std::int32_t>(text_input_state_.marked_cursor),
      .preedit_cursor_end = static_cast<std::int32_t>(
          selected_bytes.first + selected_bytes.second)};
  (void)append_ime_default_preedit_style(composition);
  callback_(composition);
}

void MacOSWindow::text_unmark() {
  if (text_input_state_.marked_text.empty()) return;
  text_input_state_.marked_text.clear();
  text_input_state_.marked_cursor = 0;
  text_input_state_.marked_selection_location_utf16 = 0;
  text_input_state_.marked_selection_length_utf16 = 0;
  ++text_input_state_.diagnostics.unmark_count;
  callback_(ImeComposition{.phase = ImeCompositionPhase::cancel});
}

void MacOSWindow::text_delete_backward() {
  if (!state_.ime_text_input_placement.has_value()) return;
  const auto& placement = *state_.ime_text_input_placement;
  const std::size_t offset = std::min(placement.byte_offset, placement.surrounding_text.size());
  std::size_t start = offset;
  while (start > 0 && (static_cast<unsigned char>(placement.surrounding_text[start - 1]) & 0xC0) == 0x80) {
    --start;
  }
  if (start > 0) --start;
  const std::size_t length = offset - start;
  ++text_input_state_.diagnostics.delete_surrounding_count;
  callback_(ImeDeleteSurroundingText{.before_length = static_cast<std::uint32_t>(length)});
}

BOOL MacOSWindow::text_has_marked() const { return !text_input_state_.marked_text.empty(); }
NSRange MacOSWindow::text_marked_range() const {
  if (!text_has_marked()) return NSMakeRange(NSNotFound, 0);
  NSString* value = [[NSString alloc] initWithBytes:text_input_state_.marked_text.data()
                                              length:text_input_state_.marked_text.size()
                                            encoding:NSUTF8StringEncoding];
  return NSMakeRange(text_input_state_.marked_document_location_utf16, [value length]);
}
NSRange MacOSWindow::text_selected_range() const {
  if (text_has_marked()) {
    return NSMakeRange(
        text_input_state_.marked_document_location_utf16 +
            text_input_state_.marked_selection_location_utf16,
        text_input_state_.marked_selection_length_utf16);
  }
  if (!state_.ime_text_input_placement.has_value()) return NSMakeRange(0, 0);
  const auto& placement = *state_.ime_text_input_placement;
  const std::size_t cursor = utf16_cursor(
      placement.surrounding_text, placement.byte_offset);
  const std::size_t anchor = utf16_cursor(
      placement.surrounding_text, placement.selection_anchor);
  return NSMakeRange(std::min(cursor, anchor),
                     cursor > anchor ? cursor - anchor : anchor - cursor);
}
NSAttributedString* MacOSWindow::text_substring(NSRange proposed, NSRange* actual) const {
  NSString* value = [[NSString alloc] initWithBytes:
      (state_.ime_text_input_placement ? state_.ime_text_input_placement->surrounding_text.data() : "")
      length:(state_.ime_text_input_placement ? state_.ime_text_input_placement->surrounding_text.size() : 0)
      encoding:NSUTF8StringEncoding];
  const NSUInteger count = value == nil ? 0 : [value length];
  const NSUInteger start = std::min(proposed.location, count);
  const NSUInteger length = std::min(proposed.length, count - start);
  if (actual != nullptr) *actual = NSMakeRange(start, length);
  return [[NSAttributedString alloc] initWithString:[value substringWithRange:NSMakeRange(start, length)]];
}
NSRect MacOSWindow::text_first_rect(NSRange, NSRange* actual) const {
  if (actual != nullptr) *actual = NSMakeRange(0, 0);
  if (window_ == nil) return NSZeroRect;
  const auto& placement = state_.ime_text_input_placement;
  if (!placement.has_value()) return NSZeroRect;
  const Rect rect = placement->candidate_rect.value_or(placement->rect);
  const Rect screen = macos_candidate_rect_to_screen(
      rect, state_.framebuffer_size, state_.scale,
      Point{static_cast<float>([window_ frame].origin.x), static_cast<float>([window_ frame].origin.y)});
  return NSMakeRect(screen.origin.x, screen.origin.y, screen.size.width, screen.size.height);
}
NSUInteger MacOSWindow::text_character_index(NSPoint) const {
  return text_selected_range().location;
}

}  // namespace cgpui
