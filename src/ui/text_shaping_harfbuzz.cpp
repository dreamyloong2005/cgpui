#include "text_shaping_internal.hpp"

#if defined(CGPUI_HAS_HARFBUZZ_SHAPING_BACKEND)
#include <hb.h>
#include <hb-ot.h>

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <memory>
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

struct HbBlobDeleter {
  void operator()(hb_blob_t* blob) const {
    if (blob != nullptr) {
      hb_blob_destroy(blob);
    }
  }
};

struct HbFaceDeleter {
  void operator()(hb_face_t* face) const {
    if (face != nullptr) {
      hb_face_destroy(face);
    }
  }
};

struct HbFontDeleter {
  void operator()(hb_font_t* font) const {
    if (font != nullptr) {
      hb_font_destroy(font);
    }
  }
};

struct HbBufferDeleter {
  void operator()(hb_buffer_t* buffer) const {
    if (buffer != nullptr) {
      hb_buffer_destroy(buffer);
    }
  }
};

using HbBlobPtr = std::unique_ptr<hb_blob_t, HbBlobDeleter>;
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

std::size_t harfbuzz_file_backed_face_index(
    const std::vector<FontFaceDescriptor>& faces) {
  for (std::size_t index = 0; index < faces.size(); ++index) {
    if (is_file_backed_font_path(faces[index].path)) {
      return index;
    }
  }
  return faces.size();
}

std::string harfbuzz_file_backed_face_path(
    const std::vector<FontFaceDescriptor>& faces,
    std::size_t index) {
  return index < faces.size() && is_file_backed_font_path(faces[index].path)
      ? faces[index].path
      : std::string{};
}

HbBlobPtr harfbuzz_blob_from_file(const std::string& path) {
  std::ifstream file(path, std::ios::binary);
  if (!file) {
    return {};
  }

  auto bytes = std::make_unique<FontFileBytes>(
      std::istreambuf_iterator<char>(file),
      std::istreambuf_iterator<char>());
  if (bytes->empty()) {
    return {};
  }

  hb_blob_t* blob = hb_blob_create(
      bytes->data(),
      static_cast<unsigned int>(bytes->size()),
      HB_MEMORY_MODE_READONLY,
      bytes.release(),
      destroy_font_file_bytes);
  return HbBlobPtr(blob);
}

HbFacePtr harfbuzz_face_from_request(
    const TextShapingRequest& request,
    std::size_t face_index) {
  const std::string path =
      harfbuzz_file_backed_face_path(request.font_fallback_faces, face_index);
  if (!path.empty()) {
    HbBlobPtr blob = harfbuzz_blob_from_file(path);
    if (blob) {
      hb_face_t* face = hb_face_create(blob.get(), 0);
      if (face != nullptr) {
        return HbFacePtr(face);
      }
    }
  }

  return HbFacePtr(hb_face_reference(hb_face_get_empty()));
}

hb_direction_t harfbuzz_direction(TextShapingDirection direction) {
  switch (direction) {
    case TextShapingDirection::right_to_left:
      return HB_DIRECTION_RTL;
    case TextShapingDirection::top_to_bottom:
      return HB_DIRECTION_TTB;
    case TextShapingDirection::bottom_to_top:
      return HB_DIRECTION_BTT;
    case TextShapingDirection::auto_direction:
    case TextShapingDirection::left_to_right:
      return HB_DIRECTION_LTR;
  }
  return HB_DIRECTION_LTR;
}

hb_script_t harfbuzz_script(TextShapingScript script) {
  switch (script) {
    case TextShapingScript::inherited:
      return HB_SCRIPT_INHERITED;
    case TextShapingScript::latin:
      return HB_SCRIPT_LATIN;
    case TextShapingScript::han:
      return HB_SCRIPT_HAN;
    case TextShapingScript::hiragana:
      return HB_SCRIPT_HIRAGANA;
    case TextShapingScript::katakana:
      return HB_SCRIPT_KATAKANA;
    case TextShapingScript::hangul:
      return HB_SCRIPT_HANGUL;
    case TextShapingScript::arabic:
      return HB_SCRIPT_ARABIC;
    case TextShapingScript::hebrew:
      return HB_SCRIPT_HEBREW;
    case TextShapingScript::devanagari:
      return HB_SCRIPT_DEVANAGARI;
    case TextShapingScript::common:
    case TextShapingScript::emoji:
      return HB_SCRIPT_COMMON;
    case TextShapingScript::auto_script:
      return HB_SCRIPT_UNKNOWN;
  }
  return HB_SCRIPT_UNKNOWN;
}

std::size_t next_cluster_after(
    const hb_glyph_info_t* infos,
    unsigned int glyph_count,
    std::size_t cluster,
    std::size_t text_size) {
  std::size_t next = text_size;
  for (unsigned int index = 0; index < glyph_count; ++index) {
    const std::size_t candidate = infos[index].cluster;
    if (cluster < candidate && candidate < next) {
      next = candidate;
    }
  }
  return std::max(cluster + 1, next);
}

void append_harfbuzz_font_run_span(
    TextShapeRun& run,
    std::size_t font_fallback_face_index,
    std::size_t glyph_index,
    std::size_t byte_offset,
    std::size_t byte_length,
    float advance,
    float device_advance) {
  if (run.font_runs.empty() ||
      run.font_runs.back().font_fallback_face_index !=
          font_fallback_face_index) {
    run.font_runs.push_back(TextFontFallbackRun{
        .font_fallback_face_index = font_fallback_face_index,
        .glyph_start = glyph_index,
        .glyph_end = glyph_index,
        .byte_start = byte_offset,
        .byte_end = byte_offset,
    });
  }

  TextFontFallbackRun& font_run = run.font_runs.back();
  font_run.glyph_end = glyph_index + 1;
  font_run.byte_end = byte_offset + byte_length;
  font_run.advance += advance;
  font_run.device_advance += device_advance;
}

void append_harfbuzz_script_run_span(
    TextShapeRun& run,
    std::size_t glyph_index,
    std::size_t byte_offset,
    std::size_t byte_length,
    float advance,
    float device_advance) {
  if (run.script_runs.empty()) {
    run.script_runs.push_back(TextScriptRun{
        .script = run.script,
        .glyph_start = glyph_index,
        .glyph_end = glyph_index,
        .byte_start = byte_offset,
        .byte_end = byte_offset,
    });
  }

  TextScriptRun& script_run = run.script_runs.back();
  script_run.glyph_end = glyph_index + 1;
  script_run.byte_end = byte_offset + byte_length;
  script_run.advance += advance;
  script_run.device_advance += device_advance;
}

TextShapeRun harfbuzz_shape_with_buffer(
    TextShapingRequest request,
    HbBufferPtr buffer,
    HbFontPtr font,
    std::size_t font_fallback_face_index) {
  unsigned int glyph_count = 0;
  hb_glyph_info_t* infos =
      hb_buffer_get_glyph_infos(buffer.get(), &glyph_count);
  hb_glyph_position_t* positions =
      hb_buffer_get_glyph_positions(buffer.get(), nullptr);
  if ((glyph_count == 0 && !request.text.empty()) || infos == nullptr ||
      positions == nullptr) {
    return shape_text_with_harfbuzz_failure(
        std::move(request),
        TextShapingFallbackReason::shaping_failed);
  }

  const float scale_value = normalized_scale(request.scale);
  TextShapeRun run{
      .text = std::string(request.text),
      .font = std::move(request.font),
      .font_fallback_faces = std::move(request.font_fallback_faces),
      .font_size = request.font_size,
      .scale = request.scale,
      .byte_length = request.text.size(),
      .line_height = request.font_size,
      .device_font_size = request.font_size * scale_value,
      .device_line_height = request.font_size * scale_value,
      .requested_backend = request.backend.requested,
      .used_backend = TextShapingBackend::harfbuzz,
      .fallback_reason = TextShapingFallbackReason::none,
      .backend_capabilities = request.backend.capabilities,
      .direction = request.direction,
      .script = request.script,
      .language = std::move(request.language),
  };

  run.glyphs.reserve(glyph_count);
  run.font_runs.reserve(glyph_count);
  run.script_runs.reserve(1);
  for (unsigned int index = 0; index < glyph_count; ++index) {
    const std::size_t byte_offset =
        std::min<std::size_t>(infos[index].cluster, run.text.size());
    const std::size_t byte_end =
        std::min(next_cluster_after(infos, glyph_count, byte_offset, run.text.size()),
                 run.text.size());
    const std::size_t byte_length = byte_end - byte_offset;
    const float device_advance =
        static_cast<float>(positions[index].x_advance) / 64.0F;
    const float advance = device_advance / scale_value;
    const float device_offset_x =
        static_cast<float>(positions[index].x_offset) / 64.0F;
    const float device_offset_y =
        static_cast<float>(positions[index].y_offset) / 64.0F;

    run.glyphs.push_back(TextGlyphRun{
        .glyph_id = infos[index].codepoint,
        .byte_offset = byte_offset,
        .byte_length = byte_length,
        .font_fallback_face_index = font_fallback_face_index,
        .advance = advance,
        .offset =
            cgpui::Point{
                .x = device_offset_x / scale_value,
                .y = device_offset_y / scale_value,
            },
    });
    run.total_advance += advance;
    run.device_total_advance += device_advance;
    append_harfbuzz_font_run_span(
        run,
        font_fallback_face_index,
        index,
        byte_offset,
        byte_length,
        advance,
        device_advance);
    append_harfbuzz_script_run_span(
        run,
        index,
        byte_offset,
        byte_length,
        advance,
        device_advance);
  }

  (void)font;
  return run;
}

#endif

} // namespace

TextShapeRun shape_text_with_harfbuzz(TextShapingRequest request) {
#if defined(CGPUI_HAS_HARFBUZZ_SHAPING_BACKEND)
  const std::size_t face_index =
      harfbuzz_file_backed_face_index(request.font_fallback_faces);
  HbFacePtr face = harfbuzz_face_from_request(request, face_index);
  HbFontPtr font(face ? hb_font_create(face.get()) : nullptr);
  HbBufferPtr buffer(hb_buffer_create());
  if (!face || !font || !buffer) {
    return shape_text_with_harfbuzz_failure(
        std::move(request),
        TextShapingFallbackReason::shaping_failed);
  }

  const float scale_value = normalized_scale(request.scale);
  const int font_scale =
      static_cast<int>(request.font_size * scale_value * 64.0F);
  hb_font_set_scale(font.get(), font_scale, font_scale);
  hb_ot_font_set_funcs(font.get());
  hb_buffer_add_utf8(buffer.get(),
                     request.text.data(),
                     static_cast<int>(request.text.size()),
                     0,
                     static_cast<int>(request.text.size()));
  hb_buffer_set_direction(buffer.get(), harfbuzz_direction(request.direction));
  hb_buffer_set_script(buffer.get(), harfbuzz_script(request.script));
  if (!request.language.empty()) {
    hb_buffer_set_language(
        buffer.get(),
        hb_language_from_string(
            request.language.data(),
            static_cast<int>(request.language.size())));
  }
  hb_buffer_guess_segment_properties(buffer.get());
  hb_shape(font.get(), buffer.get(), nullptr, 0);

  return harfbuzz_shape_with_buffer(
      std::move(request),
      std::move(buffer),
      std::move(font),
      face_index);
#else
  return shape_text_with_harfbuzz_failure(
      std::move(request),
      TextShapingFallbackReason::backend_unavailable);
#endif
}

} // namespace cgpui
