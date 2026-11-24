#include "dx12_command_reuse_supplement_test_support.hpp"
#include <string_view>

using namespace cgpui;
using namespace cgpui_test;
namespace {
constexpr const char* sentinel = "CGPUI reuse late public failure sentinel";
bool only_sentinel(ID3D12InfoQueue& queue) {
  bool found = false;
  for (UINT64 i = 0; i < queue.GetNumStoredMessagesAllowedByRetrievalFilter(); ++i) {
    SIZE_T size = 0;
    if (FAILED(queue.GetMessage(i, nullptr, &size)) || size < sizeof(D3D12_MESSAGE)) return false;
    std::vector<unsigned char> bytes(size); auto* message = reinterpret_cast<D3D12_MESSAGE*>(bytes.data());
    if (FAILED(queue.GetMessage(i, message, &size)) || message->Severity == D3D12_MESSAGE_SEVERITY_CORRUPTION) return false;
    if (message->Severity == D3D12_MESSAGE_SEVERITY_ERROR) {
      if (!message->pDescription || std::string_view(message->pDescription).find(sentinel) == std::string_view::npos) return false;
      found = true;
    }
  }
  return found;
}
}
int main() {
  if (_putenv_s("CGPUI_DX12_DEBUG", "1")) return dx12_fail(1, "late publication debug environment");
  Dx12RecoveryDeadline deadline; Dx12Window window; auto state = srv_native_state(&window);
  if (!deadline.done || !state) return dx12_fail(2, "late publication native setup");
  state->command_observation = std::make_unique<Dx12CommandObservation>(32768);
  auto tail = std::make_unique<ReuseQueueTail>(); auto* verifier = tail.get();
  state->command_observation->verifier = std::move(tail);
  if (!reuse_scene(state, true, true) || !reuse_scene(state, false, false) || !reuse_last_bundle(*state))
    return dx12_fail(3, "late publication fresh prerequisite");
  auto draws = reuse_raw_scene(*state);
  if (!draws) return dx12_fail(4, "exact raw scene preparation");
  const auto pixels = state->pixels->rgba8; const auto diagnostics = *state->diagnostics;
  const auto creates = reuse_event_count(*state, Dx12CommandEventKind::bundle_created);
  const auto executes = reuse_event_count(*state, Dx12CommandEventKind::execute_bundle);
  Dx12Ptr<ID3D12InfoQueue> queue;
  if (FAILED(state->device.As(&queue)) || FAILED(queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, FALSE)) ||
      !dx12_check_debug_errors(*state) || FAILED(queue->AddApplicationMessage(D3D12_MESSAGE_SEVERITY_ERROR, sentinel)))
    return dx12_fail(5, "real InfoQueue injection unavailable");
  const auto rejected = reuse_raw_submit(*state, *draws);
  if (rejected || rejected.error().message.find(sentinel) == std::string::npos || state->submitted ||
      !reuse_trace_pixels_ready(*state, *verifier, pixels) || state->pixels->rgba8 != pixels ||
      !diagnostic_same(&*state->diagnostics, diagnostics) || !only_sentinel(*queue.Get()) ||
      reuse_event_count(*state, Dx12CommandEventKind::bundle_created) != creates ||
      reuse_event_count(*state, Dx12CommandEventKind::execute_bundle) != executes + 1 ||
      reuse_event_count(*state, Dx12CommandEventKind::bundle_released))
    return dx12_fail(6, "late public failure was not a completed real hit, or published/released success");
  queue->ClearStoredMessages(); // Only after every ERROR is proven to be the intended sentinel.
  if (!reuse_raw_submit(*state, *draws) || !reuse_trace_pixels_ready(*state, *verifier, pixels) ||
      reuse_event_count(*state, Dx12CommandEventKind::bundle_created) != creates + 1 ||
      reuse_event_count(*state, Dx12CommandEventKind::bundle_released) != 1 || !dx12_check_debug_errors(*state))
    return dx12_fail(7, "failed current attempt reused stale published success instead of creating fresh native entry");
  // Lower-level phase fixture only: no clearing state.failed/submitted and no public retry claim.
  return 0;
}
