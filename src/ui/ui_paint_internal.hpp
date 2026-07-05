#pragma once

#include "cgpui/ui/runtime.hpp"

#include <algorithm>
#include <optional>
#include <span>
#include <vector>

namespace cgpui {
namespace {

inline PaintMetadata compose_paint_metadata(
    PaintMetadata parent,
    PaintMetadata child) {
  return PaintMetadata{
      .opacity = parent.opacity * child.opacity,
      .transform = compose(parent.transform, child.transform),
  };
}

inline PaintMetadata paint_metadata_for_style(const Style& style) {
  return PaintMetadata{
      .opacity = style.opacity,
      .transform = style.transform,
  };
}

inline std::optional<Rect> current_clip_rect_for(const std::vector<Rect>& clips) {
  return clips.empty() ? std::optional<Rect>{}
                       : std::optional<Rect>{clips.back()};
}

inline RendererClipStackRecord clip_stack_record_for(
    const std::vector<Rect>& clips) {
  return renderer_clip_stack_record(
      std::span<const Rect>{clips.data(), clips.size()});
}

inline RendererCompositionStackRecord composition_stack_record_for(
    const std::vector<PaintMetadata>& entries) {
  return renderer_composition_stack_record(
      std::span<const PaintMetadata>{entries.data(), entries.size()});
}

inline void record_clip_stack_statistics(
    FrameStatistics& statistics,
    const PaintCommand& command) {
  if (command.clip_stack.empty()) {
    return;
  }
  statistics.clip_stack_command_count += 1;
  statistics.max_clip_stack_depth = std::max(
      statistics.max_clip_stack_depth,
      command.clip_stack.full_depth);
}

inline void record_composition_stack_statistics(
    FrameStatistics& statistics,
    const PaintCommand& command) {
  if (command.composition_stack.empty()) {
    return;
  }
  statistics.composition_stack_command_count += 1;
  statistics.max_composition_stack_depth = std::max(
      statistics.max_composition_stack_depth,
      command.composition_stack.full_depth);
}

inline void paint_styled_box_base(
    PaintList& paint_list,
    const std::optional<Rect>& bounds,
    const Style& style) {
  if (bounds.has_value() && style.box_shadow.has_value()) {
    paint_list.draw_box_shadow(*bounds, *style.box_shadow, style.border_radius);
  }
  if (bounds.has_value() && style.background_color.has_value()) {
    const BorderRadii radius = style.border_radius;
    if (radius.top_left > 0.0F || radius.top_right > 0.0F ||
        radius.bottom_right > 0.0F || radius.bottom_left > 0.0F) {
      paint_list.fill_rounded_rect(*bounds, *style.background_color, radius);
    } else {
      paint_list.fill_rect(*bounds, *style.background_color);
    }
  }
  if (bounds.has_value() && style.border_color.has_value()) {
    const Rect rect = *bounds;
    const Color color = *style.border_color;
    const float top = style.border_width.top;
    const float right = style.border_width.right;
    const float bottom = style.border_width.bottom;
    const float left = style.border_width.left;
    const float vertical_side_height =
        std::max(0.0F, rect.size.height - top - bottom);

    if (top > 0.0F) {
      paint_list.fill_rect(
          Rect{
              .origin = rect.origin,
              .size = {.width = rect.size.width, .height = top},
          },
          color);
    }
    if (right > 0.0F && vertical_side_height > 0.0F) {
      paint_list.fill_rect(
          Rect{
              .origin =
                  {
                      .x = rect.origin.x + rect.size.width - right,
                      .y = rect.origin.y + top,
                  },
              .size = {.width = right, .height = vertical_side_height},
          },
          color);
    }
    if (bottom > 0.0F) {
      paint_list.fill_rect(
          Rect{
              .origin =
                  {
                      .x = rect.origin.x,
                      .y = rect.origin.y + rect.size.height - bottom,
                  },
              .size = {.width = rect.size.width, .height = bottom},
          },
          color);
    }
    if (left > 0.0F && vertical_side_height > 0.0F) {
      paint_list.fill_rect(
          Rect{
              .origin = {.x = rect.origin.x, .y = rect.origin.y + top},
              .size = {.width = left, .height = vertical_side_height},
          },
          color);
    }
  }
}

} // namespace

void submit_paint_command_to_frame(
    RenderFrame& frame,
    const PaintCommand& command,
    FrameStatistics* statistics);

} // namespace cgpui
