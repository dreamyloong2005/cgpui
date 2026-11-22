#include "text_shaping_internal.hpp"

#include "text_coretext_macos_internal.hpp"
#include "text_shaping_native_internal.hpp"

#include <algorithm>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace cgpui {
namespace {

template <typename T>
struct Cf {
  T value = nullptr;
  explicit Cf(T held = nullptr) : value(held) {}
  ~Cf() { if (value != nullptr) CFRelease(value); }
  Cf(const Cf&) = delete;
  Cf& operator=(const Cf&) = delete;
  [[nodiscard]] T get() const { return value; }
};

TextShapeRun core_text_failure(TextShapingRequest request,
                               TextShapingFallbackReason reason) {
  request.backend.used = TextShapingBackend::deterministic_fallback;
  request.backend.fallback_reason = reason;
  return shape_text_with_deterministic_fallback(std::move(request));
}

// UTF-16 units of the text and the UTF-8 byte offset of every unit
// (offsets[units.size()] is the text size).
struct Utf16Text {
  std::vector<UniChar> units;
  std::vector<std::size_t> offsets;
};

Utf16Text to_utf16(std::string_view text) {
  Utf16Text result;
  std::size_t offset = 0;
  while (offset < text.size()) {
    std::size_t length = 0;
    char32_t codepoint = text_shaping_decode_utf8(text, offset, &length);
    if (length == 0) length = 1;
    if (codepoint > 0x10FFFFU) codepoint = 0xFFFDU;
    if (codepoint >= 0x10000U) {
      codepoint -= 0x10000U;
      result.units.push_back(static_cast<UniChar>(0xD800U + (codepoint >> 10U)));
      result.offsets.push_back(offset);
      result.units.push_back(static_cast<UniChar>(0xDC00U + (codepoint & 0x3FFU)));
    } else {
      result.units.push_back(static_cast<UniChar>(codepoint));
    }
    result.offsets.push_back(offset);
    offset += length;
  }
  result.offsets.push_back(text.size());
  return result;
}

NativeShapedFace shaped_face(CTFontRef font, const FontDescriptor& requested) {
  NativeShapedFace face{.face = core_text_face_descriptor(font, requested),
                        .metrics = core_text_font_metrics(font)};
  const UniChar space = u' ';
  CGGlyph glyph = 0;
  if (CTFontGetGlyphsForCharacters(font, &space, &glyph, 1)) {
    CGSize advance{};
    CTFontGetAdvancesForGlyphs(font, kCTFontOrientationHorizontal, &glyph, &advance, 1);
    face.space_glyph_id = glyph;
    face.space_advance = static_cast<float>(advance.width);
  }
  return face;
}

struct PendingGlyph {
  std::size_t slot = 0;
  NativeShapedGlyph glyph;
};

}  // namespace

TextShapeRun shape_text_with_core_text(TextShapingRequest request) {
  Cf<CTFontRef> primary(core_text_primary_font(request.font, request.font_size));
  if (primary.get() == nullptr) {
    return core_text_failure(std::move(request),
                             TextShapingFallbackReason::backend_unavailable);
  }
  NativeShapeBuilder builder(request, TextShapingBackend::core_text);
  std::map<std::uint32_t, std::size_t> slots;
  const auto slot_for = [&](CTFontRef font) {
    NativeShapedFace face = shaped_face(font, request.font);
    const std::uint32_t index = face.face.face_index;
    if (const auto found = slots.find(index); found != slots.end()) return found->second;
    const std::size_t slot = builder.add_face(std::move(face));
    slots.emplace(index, slot);
    return slot;
  };
  (void)slot_for(primary.get());
  if (request.text.empty()) return builder.finish();

  const Utf16Text text = to_utf16(request.text);
  Cf<CFStringRef> string(CFStringCreateWithCharactersNoCopy(
      nullptr, text.units.data(), static_cast<CFIndex>(text.units.size()), kCFAllocatorNull));
  Cf<CFStringRef> language(request.language.empty()
      ? nullptr
      : CFStringCreateWithBytes(nullptr,
                                reinterpret_cast<const UInt8*>(request.language.data()),
                                static_cast<CFIndex>(request.language.size()),
                                kCFStringEncodingUTF8, false));
  const void* keys[]{kCTFontAttributeName, kCTLanguageAttributeName};
  const void* values[]{primary.get(), language.get()};
  Cf<CFDictionaryRef> attributes(CFDictionaryCreate(
      nullptr, keys, values, language.get() == nullptr ? 1 : 2,
      &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));
  Cf<CFAttributedStringRef> attributed(
      CFAttributedStringCreate(nullptr, string.get(), attributes.get()));
  Cf<CTLineRef> line(CTLineCreateWithAttributedString(attributed.get()));
  if (string.get() == nullptr || line.get() == nullptr) {
    return core_text_failure(std::move(request), TextShapingFallbackReason::shaping_failed);
  }

  // Core Text resolves fallback fonts and bidi itself. Its runs are in
  // visual order; glyphs are collected and put back in logical order.
  std::vector<PendingGlyph> glyphs;
  CFArrayRef runs = CTLineGetGlyphRuns(line.get());
  for (CFIndex run_index = 0; run_index < CFArrayGetCount(runs); ++run_index) {
    auto run = static_cast<CTRunRef>(CFArrayGetValueAtIndex(runs, run_index));
    const CFIndex count = CTRunGetGlyphCount(run);
    if (count <= 0) continue;
    auto font = static_cast<CTFontRef>(
        CFDictionaryGetValue(CTRunGetAttributes(run), kCTFontAttributeName));
    const std::size_t slot = slot_for(font != nullptr ? font : primary.get());
    std::vector<CGGlyph> ids(static_cast<std::size_t>(count));
    std::vector<CGPoint> positions(ids.size());
    std::vector<CGSize> advances(ids.size());
    std::vector<CFIndex> indices(ids.size());
    const CFRange all = CFRangeMake(0, 0);
    CTRunGetGlyphs(run, all, ids.data());
    CTRunGetPositions(run, all, positions.data());
    CTRunGetAdvances(run, all, advances.data());
    CTRunGetStringIndices(run, all, indices.data());
    double pen = positions.front().x;
    for (std::size_t index = 0; index < ids.size(); ++index) {
      const auto unit = static_cast<std::size_t>(
          std::clamp<CFIndex>(indices[index], 0, static_cast<CFIndex>(text.units.size())));
      glyphs.push_back(PendingGlyph{
          .slot = slot,
          .glyph = {.glyph_id = ids[index],
                    .cluster_start = text.offsets[unit],
                    .advance = static_cast<float>(advances[index].width),
                    .offset = {.x = static_cast<float>(positions[index].x - pen),
                               .y = -static_cast<float>(positions[index].y)}}});
      pen += advances[index].width;
    }
  }
  std::ranges::stable_sort(glyphs, {}, [](const PendingGlyph& pending) {
    return pending.glyph.cluster_start;
  });
  for (std::size_t index = 0; index < glyphs.size(); ++index) {
    std::size_t end = request.text.size();
    for (std::size_t next = index + 1; next < glyphs.size(); ++next) {
      if (glyphs[next].glyph.cluster_start > glyphs[index].glyph.cluster_start) {
        end = glyphs[next].glyph.cluster_start;
        break;
      }
    }
    glyphs[index].glyph.cluster_end = end;
    builder.add_glyph(glyphs[index].slot, glyphs[index].glyph);
  }
  return builder.finish();
}

}  // namespace cgpui
