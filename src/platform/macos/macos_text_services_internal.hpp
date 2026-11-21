#pragma once

#include "cgpui/platform/platform_font_discovery.hpp"
#include "cgpui/platform/platform.hpp"

#import <AppKit/AppKit.h>

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace cgpui {

struct MacOSTextServicesDiagnostics {
  std::size_t committed_text_count = 0;
  std::size_t marked_text_update_count = 0;
  std::size_t unmark_count = 0;
  std::size_t delete_surrounding_count = 0;
};

struct MacOSTextInputState {
  std::string marked_text;
  std::size_t marked_cursor = 0;
  std::size_t marked_document_location_utf16 = 0;
  std::size_t marked_selection_location_utf16 = 0;
  std::size_t marked_selection_length_utf16 = 0;
  MacOSTextServicesDiagnostics diagnostics;
};

[[nodiscard]] PlatformFontDiscoveryResult macos_discover_fonts();
[[nodiscard]] std::pair<std::size_t, std::size_t> macos_nsrange_to_utf8_bytes(
    std::string_view text,
    std::size_t location,
    std::size_t length);
// Clause spans of marked text (NSTextInputClient setMarkedText:). Runs of
// NSMarkedClauseSegmentAttributeName (else of NSUnderlineStyleAttributeName)
// become clauses: a thick underline is the selected converted clause, other
// clauses of a segmented or multi-run string are converted, and a single
// unsegmented run (or no attributes) is one input clause. Spans beyond
// kImePreeditStyleSpanCapacity merge into the last one.
void macos_append_ime_clauses(
    ImeComposition& composition,
    NSAttributedString* marked);
[[nodiscard]] Rect macos_candidate_rect_to_screen(
    Rect rect,
    Size framebuffer_size,
    DpiScale scale,
    Point window_origin);

[[nodiscard]] MacOSTextServicesDiagnostics macos_text_services_diagnostics(
    const PlatformWindow& window);

}  // namespace cgpui
