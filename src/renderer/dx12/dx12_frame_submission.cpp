#include "dx12_frame_submission_internal.hpp"
#include "dx12_frame_command_reuse_internal.hpp"
#include "dx12_state_internal.hpp"
#include "dx12_glyph_recording_scope_internal.hpp"
#include "dx12_command_observation_internal.hpp"

namespace cgpui {
Result<void> dx12_execute_frame(Dx12State& state, Dx12GlyphRecordingScope& recording, ID3D12Resource& target) {
  auto* observation = state.command_observation.get();
  ID3D12CommandList* tail = observation ? observation->prepare_tail(state, target) : nullptr;
  ID3D12CommandList* lists[] = {state.commands.Get()};
  if (!state.command_reuse->prearm(state))
    return std::unexpected(dx12_error(E_UNEXPECTED, "reuse submission prearm"));
  state.queue->ExecuteCommandLists(1, lists);
  recording.executed(); // Original glyph no-throw ownership mark, before ANY observation or tail.
  dx12_observe_command(state, {.kind = Dx12CommandEventKind::producer_execute,
      .list = state.commands.Get(), .object = state.queue.Get(), .object2 = &target});
  if (tail) {
    state.queue->ExecuteCommandLists(1, &tail); // Separate verifier submission, same queue, before original fence.
    observation->tail_executed();
    dx12_observe_command(state, {.kind = Dx12CommandEventKind::tail_execute,
        .object = tail, .object2 = state.queue.Get()});
  }
  return {};
}
} // namespace cgpui
