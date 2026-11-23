#pragma once

#include "cgpui/renderer/renderer_commands.hpp"
#include "cgpui/renderer/planning/pixel_scissor.hpp"

namespace cgpui {

struct ClipScissorResolution {
  PixelScissor scissor;
  bool visible = false;
  bool clipped = false;
};

[[nodiscard]] std::optional<Rect> resolve_effective_clip_rect(
    std::optional<Rect> clip_rect, const RendererClipStackRecord& clip_stack);
[[nodiscard]] ClipScissorResolution resolve_clip_stack_scissor(
    PixelExtent extent, std::optional<Rect> clip_rect,
    const RendererClipStackRecord& clip_stack);

} // namespace cgpui
