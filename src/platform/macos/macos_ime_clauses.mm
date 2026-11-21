#include "macos_text_services_internal.hpp"

#include <algorithm>
#include <limits>
#include <vector>

namespace cgpui {
namespace {

struct MarkedRun {
  NSRange range{};
  NSInteger segment = -1;  // -1: no clause segment attribute
  NSInteger underline = 0;
};

std::vector<MarkedRun> marked_runs(NSAttributedString* marked) {
  __block std::vector<MarkedRun> runs;
  [marked enumerateAttributesInRange:NSMakeRange(0, marked.length)
                             options:0
                          usingBlock:^(NSDictionary<NSAttributedStringKey, id>* attributes,
                                       NSRange range, BOOL*) {
    id segment = attributes[NSMarkedClauseSegmentAttributeName];
    id underline = attributes[NSUnderlineStyleAttributeName];
    const MarkedRun run{
        .range = range,
        .segment = [segment isKindOfClass:[NSNumber class]] ? [segment integerValue] : -1,
        .underline = [underline isKindOfClass:[NSNumber class]] ? [underline integerValue] : 0};
    if (!runs.empty() && runs.back().segment == run.segment &&
        runs.back().underline == run.underline &&
        NSMaxRange(runs.back().range) == range.location) {
      runs.back().range.length += range.length;
    } else {
      runs.push_back(run);
    }
  }];
  return runs;
}

}  // namespace

void macos_append_ime_clauses(
    ImeComposition& composition,
    NSAttributedString* marked) {
  if (marked == nil || marked.length == 0 || composition.text.empty()) {
    (void)append_ime_default_preedit_style(composition);
    return;
  }
  const std::vector<MarkedRun> runs = marked_runs(marked);
  const bool segmented = std::ranges::any_of(
      runs, [](const MarkedRun& run) { return run.segment >= 0; });
  for (const MarkedRun& run : runs) {
    // NSUnderlineStyle: the low byte is the line style; Thick is 0x02.
    const NSInteger line = run.underline & 0xFF;
    ImeClauseStyle style = ImeClauseStyle::input;
    if (line >= NSUnderlineStyleThick) {
      style = ImeClauseStyle::selected_converted;
    } else if (segmented || runs.size() > 1) {
      style = ImeClauseStyle::converted;
    }
    const auto [offset, length] = macos_nsrange_to_utf8_bytes(
        composition.text, run.range.location, run.range.length);
    if (length == 0) continue;
    const ImePreeditStyleSpan span{
        .byte_offset = static_cast<std::uint32_t>(offset),
        .byte_length = static_cast<std::uint32_t>(length),
        .clause = style};
    if (!append_ime_preedit_style(composition, span)) {
      ImePreeditStyleSpan& last =
          composition.preedit_styles[composition.preedit_style_count - 1];
      last.byte_length = static_cast<std::uint32_t>(offset + length - last.byte_offset);
    }
  }
  if (composition.preedit_style_count == 0) {
    (void)append_ime_default_preedit_style(composition);
  }
}

}  // namespace cgpui
