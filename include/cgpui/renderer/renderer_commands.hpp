#pragma once

#include "cgpui/renderer/renderer_types.hpp"
#include "cgpui/ui/text.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace cgpui {

struct SolidRect {
  Rect rect;
  Color color;
  std::optional<Rect> clip_rect;
  RendererClipStackRecord clip_stack;
  RendererCompositionStackRecord composition_stack;
  PaintMetadata metadata;
};

struct RoundedRectDraw {
  Rect rect;
  Color color;
  BorderRadii radius;
  bool fill_enabled = true;
  std::optional<Color> border_color;
  float border_width = 0.0F;
  std::optional<Rect> clip_rect;
  RendererClipStackRecord clip_stack;
  RendererCompositionStackRecord composition_stack;
  PaintMetadata metadata;
};

struct TextDraw {
  Rect bounds;
  Color color;
  FontDescriptor font;
  std::string content;
  std::size_t byte_length = 0;
  float font_size = 16.0F;
  DpiScale scale;
  float device_font_size = 16.0F;
  std::vector<TextGlyphPaint> glyphs;
  std::vector<TextWrapLine> lines;
  std::vector<RichTextRun> rich_text_runs;
  std::vector<RichTextInlineImageRun> rich_text_inline_images;
  std::optional<Rect> clip_rect;
  RendererClipStackRecord clip_stack;
  RendererCompositionStackRecord composition_stack;
  PaintMetadata metadata;
};

struct TextSelectionDraw {
  Rect rect;
  Color color;
  TextSelectionRange range;
  float font_size = 16.0F;
  std::optional<Rect> clip_rect;
  RendererClipStackRecord clip_stack;
  RendererCompositionStackRecord composition_stack;
  PaintMetadata metadata;
};

struct TextCaretDraw {
  Rect rect;
  Color color;
  std::size_t byte_offset = 0;
  float font_size = 16.0F;
  std::optional<Rect> clip_rect;
  RendererClipStackRecord clip_stack;
  RendererCompositionStackRecord composition_stack;
  PaintMetadata metadata;
};

struct ImageDraw {
  Rect bounds;
  ImageAssetDescriptor asset;
  std::optional<Rect> source_rect;
  std::optional<Color> tint;
  std::optional<Rect> clip_rect;
  RendererClipStackRecord clip_stack;
  RendererCompositionStackRecord composition_stack;
  PaintMetadata metadata;
};

enum class RendererPrimitiveKind {
  solid_rect,
  rounded_rect,
  text,
  text_selection,
  text_caret,
  image,
};

[[nodiscard]] constexpr std::string_view renderer_primitive_kind_name(
    RendererPrimitiveKind kind) {
  switch (kind) {
    case RendererPrimitiveKind::solid_rect:
      return "solid_rect";
    case RendererPrimitiveKind::rounded_rect:
      return "rounded_rect";
    case RendererPrimitiveKind::text:
      return "text";
    case RendererPrimitiveKind::text_selection:
      return "text_selection";
    case RendererPrimitiveKind::text_caret:
      return "text_caret";
    case RendererPrimitiveKind::image:
      return "image";
  }

  return "unknown";
}

struct RendererCommandBatchKey {
  RendererPrimitiveKind primitive_kind = RendererPrimitiveKind::solid_rect;
  std::optional<Rect> clip_rect;
  RendererClipStackRecord clip_stack;
  RendererCompositionStackRecord composition_stack;
  PaintMetadata metadata;
};

struct RendererCommandBatch {
  RendererCommandBatchKey key;
  std::size_t command_count = 0;
  std::vector<std::size_t> command_indices;
};

struct RendererCommandStreamItem {
  RendererPrimitiveKind primitive_kind = RendererPrimitiveKind::solid_rect;
  std::size_t command_index = 0;
  std::optional<Rect> clip_rect;
  RendererClipStackRecord clip_stack;
  RendererCompositionStackRecord composition_stack;
  PaintMetadata metadata;
};

enum class RendererUnsupportedCommandReason {
  unsupported_primitive,
};

struct RendererUnsupportedCommandDiagnostic {
  RendererPrimitiveKind primitive_kind = RendererPrimitiveKind::solid_rect;
  std::size_t command_index = 0;
  RendererUnsupportedCommandReason reason =
      RendererUnsupportedCommandReason::unsupported_primitive;
  std::string message;
};

} // namespace cgpui
