#include "text_shaping_internal.hpp"

#if defined(_WIN32)
#include "cgpui/renderer/text/text_font_defaults.hpp"
#include "text_dwrite_win32_internal.hpp"

#include <algorithm>
#include <map>
#endif

#include <utility>

namespace cgpui {
namespace {

TextShapeRun directwrite_failure(
    TextShapingRequest request,
    TextShapingFallbackReason reason) {
  request.backend.used = TextShapingBackend::deterministic_fallback;
  request.backend.fallback_reason = reason;
  return shape_text_with_deterministic_fallback(std::move(request));
}

#if defined(_WIN32)

struct Cluster {
  std::uint32_t text_start = 0;
  std::uint32_t text_end = 0;
  std::uint32_t glyph_start = 0;
  std::uint32_t glyph_end = 0;
};

struct Features {
  std::vector<DWRITE_FONT_FEATURE> values;
  DWRITE_TYPOGRAPHIC_FEATURES list{};
  const DWRITE_TYPOGRAPHIC_FEATURES* pointer = nullptr;
  UINT32 range_length = 0;

  Features(const std::vector<FontFeature>& features, UINT32 length) {
    for (const FontFeature& feature : features) {
      values.push_back(DWRITE_FONT_FEATURE{
          static_cast<DWRITE_FONT_FEATURE_TAG>(DWRITE_MAKE_OPENTYPE_TAG(
              feature.tag[0], feature.tag[1], feature.tag[2], feature.tag[3])),
          feature.value});
    }
    list = {values.data(), static_cast<UINT32>(values.size())};
    pointer = &list;
    range_length = length;
  }
  [[nodiscard]] const DWRITE_TYPOGRAPHIC_FEATURES** ranges() {
    return values.empty() ? nullptr : &pointer;
  }
  [[nodiscard]] const UINT32* lengths() const {
    return values.empty() ? nullptr : &range_length;
  }
  [[nodiscard]] UINT32 count() const { return values.empty() ? 0U : 1U; }
};

// Clusters in logical (text) order from a DirectWrite cluster map.
std::vector<Cluster> clusters_from_map(
    const std::vector<UINT16>& cluster_map,
    UINT32 glyph_count) {
  std::vector<Cluster> clusters;
  for (UINT32 index = 0; index < cluster_map.size(); ++index) {
    if (!clusters.empty() && cluster_map[index] == clusters.back().glyph_start) {
      clusters.back().text_end = index + 1U;
      continue;
    }
    clusters.push_back({index, index + 1U, cluster_map[index], 0});
  }
  std::vector<UINT32> starts;
  for (const Cluster& cluster : clusters) starts.push_back(cluster.glyph_start);
  std::ranges::sort(starts);
  for (Cluster& cluster : clusters) {
    const auto next = std::ranges::upper_bound(starts, cluster.glyph_start);
    cluster.glyph_end = next == starts.end() ? glyph_count : *next;
  }
  return clusters;
}

struct Segment {
  const DWriteUtf16Text& text;
  std::uint32_t start = 0;
  std::uint32_t length = 0;
  const DWriteItem& item;
  const std::wstring& locale;
  float font_size = 16.0F;
};

bool shape_segment(
    IDWriteTextAnalyzer* analyzer,
    IDWriteFontFace* face,
    const Segment& segment,
    const std::vector<FontFeature>& feature_list,
    NativeShapeBuilder& builder,
    std::size_t slot) {
  const WCHAR* text = segment.text.text.data() + segment.start;
  const BOOL rtl = (segment.item.bidi_level & 1U) != 0U;
  Features features(feature_list, segment.length);
  std::vector<UINT16> cluster_map(segment.length);
  std::vector<DWRITE_SHAPING_TEXT_PROPERTIES> text_props(segment.length);
  std::vector<UINT16> glyphs;
  std::vector<DWRITE_SHAPING_GLYPH_PROPERTIES> glyph_props;
  UINT32 capacity = segment.length * 3U / 2U + 16U;
  UINT32 count = 0;
  HRESULT result = E_NOT_SUFFICIENT_BUFFER;
  for (int attempt = 0; attempt < 6 && result == E_NOT_SUFFICIENT_BUFFER; ++attempt) {
    glyphs.resize(capacity);
    glyph_props.resize(capacity);
    result = analyzer->GetGlyphs(text, segment.length, face, FALSE, rtl,
        &segment.item.script, segment.locale.c_str(), nullptr, features.ranges(),
        features.lengths(), features.count(), capacity, cluster_map.data(),
        text_props.data(), glyphs.data(), glyph_props.data(), &count);
    capacity *= 2U;
  }
  if (FAILED(result)) return false;
  std::vector<FLOAT> advances(count);
  std::vector<DWRITE_GLYPH_OFFSET> offsets(count);
  if (FAILED(analyzer->GetGlyphPlacements(text, cluster_map.data(),
          text_props.data(), segment.length, glyphs.data(), glyph_props.data(),
          count, face, segment.font_size, FALSE, rtl, &segment.item.script,
          segment.locale.c_str(), features.ranges(), features.lengths(),
          features.count(), advances.data(), offsets.data()))) {
    return false;
  }
  const auto& utf8 = segment.text.utf8_offsets;
  for (const Cluster& cluster : clusters_from_map(cluster_map, count)) {
    const std::size_t byte_start = utf8[segment.start + cluster.text_start];
    const std::size_t byte_end = utf8[segment.start + cluster.text_end];
    for (UINT32 glyph = cluster.glyph_start; glyph < cluster.glyph_end; ++glyph) {
      const float offset_x = offsets[glyph].advanceOffset;
      builder.add_glyph(slot, NativeShapedGlyph{
          .glyph_id = glyphs[glyph],
          .cluster_start = byte_start,
          .cluster_end = byte_end,
          .advance = advances[glyph],
          .offset = {.x = rtl ? -offset_x : offset_x,
                     .y = -offsets[glyph].ascenderOffset},
      });
    }
  }
  return true;
}

#endif

} // namespace

TextShapeRun shape_text_with_directwrite(TextShapingRequest request) {
#if defined(_WIN32)
  IDWriteFactory2* factory = dwrite_factory();
  ComPtr<IDWriteTextAnalyzer> analyzer;
  if (factory == nullptr ||
      FAILED(factory->CreateTextAnalyzer(analyzer.GetAddressOf()))) {
    return directwrite_failure(std::move(request),
                               TextShapingFallbackReason::backend_unavailable);
  }
  std::string family = resolve_platform_font_family(request.font.family);
  ComPtr<IDWriteFont> base =
      dwrite_find_font(family, request.font.weight, request.font.style);
  if (!base) {
    family = resolve_platform_font_family({});
    base = dwrite_find_font(family, request.font.weight, request.font.style);
  }
  const std::optional<DWriteUtf16Text> text = dwrite_utf16_from_utf8(request.text);
  std::optional<DWriteFace> primary;
  if (base) primary = dwrite_face_for_font(base.Get(), request.font_size);
  if (!text || !primary) {
    return directwrite_failure(std::move(request),
                               TextShapingFallbackReason::shaping_failed);
  }
  const std::wstring locale =
      request.language.empty() ? std::wstring(L"en-us") : dwrite_wide(request.language);
  const std::wstring wide_family = dwrite_wide(family);
  NativeShapeBuilder builder(request, TextShapingBackend::directwrite);
  builder.add_face(primary->info);
  // Fonts stay referenced while their pointers key the face map.
  std::vector<ComPtr<IDWriteFont>> fonts;
  std::map<std::pair<IDWriteFont*, float>, std::optional<DWriteFace>> faces;
  const bool rtl = request.direction == TextShapingDirection::right_to_left;
  for (const DWriteItem& item : dwrite_itemize(text->text, locale, rtl)) {
    for (const DWriteFallbackSegment& piece :
         dwrite_map_fallback(text->text, locale, item, wide_family, base.Get())) {
      IDWriteFont* font = piece.font ? piece.font.Get() : base.Get();
      const float size = request.font_size * piece.scale;
      auto [entry, inserted] = faces.try_emplace({font, size});
      if (inserted) {
        fonts.emplace_back(font);
        entry->second = dwrite_face_for_font(font, size);
      }
      const DWriteFace& face = entry->second ? *entry->second : *primary;
      const std::size_t slot = builder.add_face(face.info);
      const Segment segment{*text, piece.start, piece.length, item, locale, size};
      if (!shape_segment(analyzer.Get(), face.face.Get(), segment,
              native_font_features(request.font, face.info.face.monospaced),
              builder, slot)) {
        return directwrite_failure(std::move(request),
                                   TextShapingFallbackReason::shaping_failed);
      }
    }
  }
  return builder.finish();
#else
  return directwrite_failure(std::move(request),
                             TextShapingFallbackReason::backend_unavailable);
#endif
}

} // namespace cgpui
