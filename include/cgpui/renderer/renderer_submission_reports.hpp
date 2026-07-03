#pragma once

#include "cgpui/renderer/renderer_commands.hpp"
#include "cgpui/renderer/renderer_text_reports.hpp"

#include <cstddef>
#include <optional>
#include <vector>

namespace cgpui {

struct RendererSubmissionPlanKey {
  RendererPrimitiveKind primitive_kind = RendererPrimitiveKind::solid_rect;
  std::optional<Rect> clip_rect;
  RendererClipStackRecord clip_stack;
  std::optional<std::size_t> atlas_page_index;
};

struct RendererSubmissionPlanRecord {
  RendererSubmissionPlanKey key;
  TextSamplerPipelineDescriptor pipeline;
  std::size_t batch_count = 0;
  std::size_t command_count = 0;
  std::size_t glyph_quad_count = 0;
  std::vector<std::size_t> batch_indices;
  std::vector<std::size_t> command_indices;
};

} // namespace cgpui
