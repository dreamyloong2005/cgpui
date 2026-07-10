#pragma once

#include "cgpui/ui/runtime.hpp"
#include "styled_box_paint_internal.hpp"

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

} // namespace

void submit_paint_command_to_frame(
    RenderFrame& frame,
    const PaintCommand& command,
    FrameStatistics* statistics);

} // namespace cgpui
