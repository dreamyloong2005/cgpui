#include "cgpui/renderer/renderer_types.hpp"

namespace cgpui {

bool RendererCompositionStackRecord::empty() const { return full_depth == 0; }

RendererCompositionStackRecord renderer_composition_stack_record(
    std::span<const PaintMetadata> entries) {
  RendererCompositionStackRecord record;
  record.full_depth = entries.size();
  if (entries.empty()) {
    return record;
  }

  record.current_metadata = entries.back();
  record.truncated = entries.size() > kRendererCompositionStackMaxDepth;
  const std::size_t first_kept =
      record.truncated ? entries.size() - kRendererCompositionStackMaxDepth : 0;
  record.entries.assign(entries.begin() + first_kept, entries.end());
  return record;
}

} // namespace cgpui
