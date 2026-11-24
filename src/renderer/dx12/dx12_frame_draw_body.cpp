#include "dx12_frame_draw_body_internal.hpp"
#include "dx12_frame_command_reuse_internal.hpp"
#include "dx12_state_internal.hpp"
#include "dx12_frame_diagnostics_internal.hpp"

namespace cgpui {
Result<void> dx12_record_frame_draws(Dx12State& state, Color clear,
    std::span<const Dx12DrawCommand> draws, bool capture, bool mutations) {
  auto reused = state.command_reuse->record(state, clear, draws, capture, mutations);
  if (!reused) return std::unexpected(reused.error());
  if (*reused) return {};
  for (std::size_t group = 0; group < draws.size(); ++group) {
    if (state.active_diagnostics) state.active_diagnostics->begin_draw(group);
    if (auto recorded = dx12_record_draw(state, draws[group]); !recorded) return recorded;
  }
  return {};
}
} // namespace cgpui
