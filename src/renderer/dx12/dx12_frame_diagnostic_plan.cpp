#include "dx12_frame_diagnostics_internal.hpp"
#include "dx12_draw_commands_internal.hpp"
#include "dx12_state_internal.hpp"
#include "cgpui/renderer/planning/clip_scissor.hpp"
#include <limits>
#include <type_traits>

namespace cgpui {
void Dx12FrameDiagnostics::plan() {
  const PixelExtent extent{state_.width, state_.height};
  groups_.reserve(draws_.size());
  for (const auto& draw : draws_) {
    const auto first_resource = planned_resources_.size();
    const auto append = [&](std::size_t local_index) {
      if (local_index > std::numeric_limits<std::size_t>::max() - draw.resource_offset) return;
      planned_resources_.push_back({draw.kind, draw.command_index, draw.resource_offset + local_index});
      renderer_add_draw_count(snapshot_.planned_draws, draw.kind);
    };
    std::visit([&](const auto& payload) {
      using Payload = std::decay_t<decltype(payload)>;
      if constexpr (std::is_same_v<Payload, RectGeometry> ||
                    std::is_same_v<Payload, BoxShadowGeometry>) {
        if (payload.vertices.empty() || payload.indices.empty()) return;
        for (std::size_t index = 0; index < payload.draws.size(); ++index) {
          const auto& range = payload.draws[index];
          if (range.index_count && resolve_clip_stack_scissor(extent, range.clip_rect, {}).visible)
            append(index);
        }
      } else if constexpr (std::is_same_v<Payload, Dx12TexturedDraw>) {
        if (!payload.vertices.empty() &&
            resolve_clip_stack_scissor(extent, payload.clip_rect, {}).visible) append(0);
      } else if constexpr (std::is_same_v<Payload, BackdropBlurPlan>) {
        if (payload.vertices.empty()) return;
        for (std::size_t index = 0; index < payload.passes.size(); ++index) {
          if (resolve_clip_stack_scissor(extent, payload.passes[index].clip_rect, {}).visible)
            append(index);
        }
      }
    }, draw.payload);
    if (planned_resources_.size() != first_resource) ++planned_work_.batch_count;
    groups_.emplace_back();
  }
}
} // namespace cgpui
