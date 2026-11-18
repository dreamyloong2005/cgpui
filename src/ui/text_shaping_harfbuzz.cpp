#include "text_shaping_internal.hpp"

#if defined(CGPUI_HAS_HARFBUZZ_SHAPING_BACKEND)
#include "text_fontconfig_linux_internal.hpp"

#include <hb.h>
#include <hb-ot.h>

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>
#endif

#include <utility>

namespace cgpui {
namespace {

TextShapeRun shape_text_with_harfbuzz_failure(
    TextShapingRequest request,
    TextShapingFallbackReason reason) {
  request.backend.used = TextShapingBackend::deterministic_fallback;
  request.backend.fallback_reason = reason;
  return shape_text_with_deterministic_fallback(std::move(request));
}

#if defined(CGPUI_HAS_HARFBUZZ_SHAPING_BACKEND)

struct HbFaceDeleter {
  void operator()(hb_face_t* face) const { hb_face_destroy(face); }
};
struct HbFontDeleter {
  void operator()(hb_font_t* font) const { hb_font_destroy(font); }
};
struct HbBufferDeleter {
  void operator()(hb_buffer_t* buffer) const { hb_buffer_destroy(buffer); }
};

using HbFacePtr = std::unique_ptr<hb_face_t, HbFaceDeleter>;
using HbFontPtr = std::unique_ptr<hb_font_t, HbFontDeleter>;
using HbBufferPtr = std::unique_ptr<hb_buffer_t, HbBufferDeleter>;
using FontFileBytes = std::vector<char>;

void destroy_font_file_bytes(void* value) {
  delete static_cast<FontFileBytes*>(value);
}

bool is_file_backed_font_path(const std::string& path) {
  return !path.empty() && path.find("://") == std::string::npos;
}

std::string harfbuzz_file_backed_face_path(const FontFaceDescriptor& face) {
  return is_file_backed_font_path(face.path) ? face.path : std::string{};
}

hb_blob_t* harfbuzz_blob_from_file(const std::string& path) {
  std::ifstream file(path, std::ios::binary);
  if (!file) return nullptr;
  auto bytes = std::make_unique<FontFileBytes>(
      std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
  if (bytes->empty()) return nullptr;
  const auto size = static_cast<unsigned int>(bytes->size());
  char* data = bytes->data();
  return hb_blob_create(data, size, HB_MEMORY_MODE_READONLY, bytes.release(),
                        destroy_font_file_bytes);
}

// Immutable hb_face_t per font file and collection index, shared by threads.
hb_face_t* harfbuzz_face_for(const FontFaceDescriptor& face) {
  static std::mutex mutex;
  static std::map<std::pair<std::string, std::uint32_t>, HbFacePtr> faces;
  const std::lock_guard lock(mutex);
  auto key = std::make_pair(face.path, face.face_index);
  if (const auto found = faces.find(key); found != faces.end()) {
    return found->second.get();
  }
  const std::string path = harfbuzz_file_backed_face_path(face);
  hb_blob_t* blob = path.empty() ? nullptr : harfbuzz_blob_from_file(path);
  if (blob == nullptr) return nullptr;
  hb_face_t* created = hb_face_create(blob, face.face_index & 0xFFFFU);
  hb_blob_destroy(blob);
  return faces.emplace(std::move(key), HbFacePtr(created)).first->second.get();
}

bool is_extender(hb_unicode_funcs_t* funcs, char32_t codepoint) {
  const hb_unicode_general_category_t category =
      hb_unicode_general_category(funcs, codepoint);
  return category == HB_UNICODE_GENERAL_CATEGORY_NON_SPACING_MARK ||
      category == HB_UNICODE_GENERAL_CATEGORY_SPACING_MARK ||
      category == HB_UNICODE_GENERAL_CATEGORY_ENCLOSING_MARK ||
      codepoint == 0x200DU || (0xFE00U <= codepoint && codepoint <= 0xFE0FU);
}

bool is_neutral_script(hb_script_t script) {
  return script == HB_SCRIPT_COMMON || script == HB_SCRIPT_INHERITED ||
      script == HB_SCRIPT_UNKNOWN;
}

struct HbRun {
  std::size_t start = 0;
  std::size_t end = 0;
  std::size_t candidate = 0;
  hb_script_t script = HB_SCRIPT_COMMON;
};

// Splits text into runs of one fallback face and one script. Marks and
// joiners stay with their base; other characters take the first face in
// fontconfig order whose charset covers them.
std::vector<HbRun> itemize(std::string_view text, const std::vector<LinuxFontFace>& faces) {
  hb_unicode_funcs_t* funcs = hb_unicode_funcs_get_default();
  std::vector<HbRun> runs;
  std::size_t offset = 0;
  while (offset < text.size()) {
    std::size_t length = 0;
    const char32_t codepoint = text_shaping_decode_utf8(text, offset, &length);
    const hb_script_t script = hb_unicode_script(funcs, codepoint);
    std::size_t candidate = runs.empty() ? 0U : runs.back().candidate;
    if (runs.empty() || !is_extender(funcs, codepoint)) {
      const auto covering = std::ranges::find_if(faces, [&](const LinuxFontFace& face) {
        return linux_face_covers(face, codepoint);
      });
      if (covering != faces.end()) {
        candidate = static_cast<std::size_t>(covering - faces.begin());
      }
    }
    const bool script_conflict = !runs.empty() && !is_neutral_script(script) &&
        !is_neutral_script(runs.back().script) && runs.back().script != script;
    if (runs.empty() || runs.back().candidate != candidate || script_conflict) {
      runs.push_back(HbRun{offset, offset, candidate, script});
    }
    HbRun& run = runs.back();
    if (is_neutral_script(run.script) && !is_neutral_script(script)) run.script = script;
    run.end = offset + length;
    offset += length;
  }
  return runs;
}

bool shape_run(
    const TextShapingRequest& request,
    const HbRun& run,
    const FontFaceDescriptor& face,
    NativeShapeBuilder& builder,
    std::size_t slot) {
  hb_face_t* hb_face = harfbuzz_face_for(face);
  if (hb_face == nullptr) return false;
  HbFontPtr font(hb_font_create(hb_face));
  HbBufferPtr buffer(hb_buffer_create());
  const int scale = static_cast<int>(request.font_size * 64.0F);
  hb_font_set_scale(font.get(), scale, scale);
  hb_ot_font_set_funcs(font.get());
  if ((face.face_index >> 16U) != 0U) {
    hb_font_set_var_named_instance(font.get(), (face.face_index >> 16U) - 1U);
  }
  hb_buffer_add_utf8(buffer.get(), request.text.data(),
                     static_cast<int>(request.text.size()),
                     static_cast<unsigned int>(run.start),
                     static_cast<int>(run.end - run.start));
  hb_direction_t direction = hb_script_get_horizontal_direction(run.script);
  if (is_neutral_script(run.script)) {
    direction = request.direction == TextShapingDirection::right_to_left
        ? HB_DIRECTION_RTL : HB_DIRECTION_LTR;
  }
  hb_buffer_set_direction(buffer.get(), direction);
  hb_buffer_set_script(buffer.get(), run.script);
  if (!request.language.empty()) {
    hb_buffer_set_language(buffer.get(), hb_language_from_string(
        request.language.data(), static_cast<int>(request.language.size())));
  }
  hb_buffer_guess_segment_properties(buffer.get());
  std::vector<hb_feature_t> features;
  for (const FontFeature& feature : native_font_features(request.font, face.monospaced)) {
    features.push_back(hb_feature_t{
        HB_TAG(feature.tag[0], feature.tag[1], feature.tag[2], feature.tag[3]),
        feature.value, HB_FEATURE_GLOBAL_START, HB_FEATURE_GLOBAL_END});
  }
  hb_shape(font.get(), buffer.get(), features.data(),
           static_cast<unsigned int>(features.size()));
  if (HB_DIRECTION_IS_BACKWARD(hb_buffer_get_direction(buffer.get()))) {
    hb_buffer_reverse(buffer.get());  // keep glyphs in logical order
  }
  unsigned int count = 0;
  const hb_glyph_info_t* infos = hb_buffer_get_glyph_infos(buffer.get(), &count);
  const hb_glyph_position_t* positions = hb_buffer_get_glyph_positions(buffer.get(), nullptr);
  if (infos == nullptr || positions == nullptr) return count == 0U;
  std::vector<std::size_t> starts;
  for (unsigned int index = 0; index < count; ++index) starts.push_back(infos[index].cluster);
  std::ranges::sort(starts);
  for (unsigned int index = 0; index < count; ++index) {
    const std::size_t start = infos[index].cluster;
    const auto next = std::ranges::upper_bound(starts, start);
    builder.add_glyph(slot, NativeShapedGlyph{
        .glyph_id = infos[index].codepoint,
        .cluster_start = start,
        .cluster_end = next == starts.end() ? run.end : *next,
        .advance = static_cast<float>(positions[index].x_advance) / 64.0F,
        .offset = {.x = static_cast<float>(positions[index].x_offset) / 64.0F,
                   .y = -static_cast<float>(positions[index].y_offset) / 64.0F},
    });
  }
  return true;
}

#endif

} // namespace

TextShapeRun shape_text_with_harfbuzz(TextShapingRequest request) {
#if defined(CGPUI_HAS_HARFBUZZ_SHAPING_BACKEND)
  const std::vector<LinuxFontFace>& faces = fontconfig_fallback_faces(request.font);
  if (faces.empty()) {
    return shape_text_with_harfbuzz_failure(
        std::move(request), TextShapingFallbackReason::backend_unavailable);
  }
  NativeShapedFace primary;
  if (!linux_native_face(faces.front().face, request.font_size, primary)) {
    return shape_text_with_harfbuzz_failure(
        std::move(request), TextShapingFallbackReason::shaping_failed);
  }
  NativeShapeBuilder builder(request, TextShapingBackend::harfbuzz);
  std::vector<std::size_t> slots(faces.size(), faces.size());
  slots.front() = builder.add_face(std::move(primary));
  for (const HbRun& run : itemize(request.text, faces)) {
    std::size_t candidate = run.candidate;
    if (slots[candidate] == faces.size()) {
      NativeShapedFace face;
      if (linux_native_face(faces[candidate].face, request.font_size, face)) {
        slots[candidate] = builder.add_face(std::move(face));
      } else {
        candidate = 0;
      }
    }
    if (!shape_run(request, run, faces[candidate].face, builder, slots[candidate])) {
      return shape_text_with_harfbuzz_failure(
          std::move(request), TextShapingFallbackReason::shaping_failed);
    }
  }
  return builder.finish();
#else
  return shape_text_with_harfbuzz_failure(
      std::move(request), TextShapingFallbackReason::backend_unavailable);
#endif
}

} // namespace cgpui
