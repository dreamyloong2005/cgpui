#pragma once

#include "cgpui/renderer/renderer_commands.hpp"

#include <cstddef>
#include <optional>
#include <span>
#include <vector>

namespace cgpui {

struct RoundedRectTessellationRecord {
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
  std::size_t corner_segment_count = 0;
  std::size_t vertex_count = 0;
  std::size_t triangle_count = 0;
};

struct TextSelectionGeometryRecord {
  Rect rect;
  Color color;
  TextSelectionRange range;
  float font_size = 16.0F;
  std::optional<Rect> clip_rect;
  RendererClipStackRecord clip_stack;
  RendererCompositionStackRecord composition_stack;
  PaintMetadata metadata;
  std::size_t vertex_count = 0;
  std::size_t triangle_count = 0;
};

struct TextCaretGeometryRecord {
  Rect rect;
  Color color;
  std::size_t byte_offset = 0;
  float font_size = 16.0F;
  std::optional<Rect> clip_rect;
  RendererClipStackRecord clip_stack;
  RendererCompositionStackRecord composition_stack;
  PaintMetadata metadata;
  std::size_t vertex_count = 0;
  std::size_t triangle_count = 0;
};

std::vector<RoundedRectTessellationRecord> vulkan_tessellate_rounded_rects(
    std::span<const RoundedRectDraw> rounded_rects);
std::vector<TextSelectionGeometryRecord> vulkan_build_text_selection_geometry(
    std::span<const TextSelectionDraw> selections);
std::vector<TextCaretGeometryRecord> vulkan_build_text_caret_geometry(
    std::span<const TextCaretDraw> carets);

} // namespace cgpui
