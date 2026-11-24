#include "dx12_command_reuse_native_test_support.hpp"
#include <array>
#include <algorithm>
#include <cstdlib>
#include <string_view>

int main() {
  using namespace cgpui;
  using namespace cgpui_test;
  Dx12RecoveryDeadline deadline;
  Dx12Window window;
  auto state = srv_native_state(&window);
  if (!deadline.done || !state || !state->info.adapter_luid || !state->info.device_generation)
    return dx12_fail(1, "reuse native setup/identity unavailable, not semantic RED");
  const char* requested = std::getenv("CGPUI_DX12_ADAPTER");
  if (requested && std::string_view(requested) == "warp" && !state->info.software_adapter)
    return dx12_fail(1, "requested WARP did not select a software adapter");
  Dx12Ptr<ID3D12InfoQueue> debug;
  if (dx12_recovery_debug_requested() && FAILED(state->device.As(&debug)))
    return dx12_fail(1, "requested native debug info queue absent");
  state->command_observation = std::make_unique<Dx12CommandObservation>(32768);
  auto tail = std::make_unique<ReuseQueueTail>(); auto* verifier = tail.get();
  state->command_observation->verifier = std::move(tail);
  if (!reuse_scene(state, true, true) || !state->pixels || !dx12_check_debug_errors(*state) ||
      !srv_topology(*state, 2, 1)) return dx12_fail(2, "mixed setup capture/native SRV prerequisite failed");
  const auto expected = state->pixels->rgba8;
  if (!dx12_pixel(&*state->pixels, 6, 6, {255, 0, 0, 255}, 0) ||
      !dx12_pixel(&*state->pixels, 16, 15, {0, 255, 0, 255}, 0) ||
      !dx12_pixel(&*state->pixels, 21, 20, {64, 128, 128, 255}, 2))
    return dx12_fail(3, "distinguishable overlapping image setup pixels wrong");
  std::array<std::uint64_t, 3> parents{};
  for (std::size_t index = 0; index < parents.size(); ++index) {
    if (!reuse_scene(state, false, false) || !reuse_trace_pixels_ready(*state, *verifier, expected) ||
        !dx12_check_debug_errors(*state) || state->command_observation->overflowed())
      return dx12_fail(4, "selected uncaptured producer frame/tail/debug failed, not semantic RED");
    parents[index] = verifier->parent;
  }
  std::cout << "same-frame queue-tail pixels passed; producer plus verifier are TWO queue submissions\n";
  if (!reuse_native_history(state->command_observation->events(), parents))
    return dx12_fail(40, "requires one actually closed immutable BUNDLE then two later native executions without rerecording");
  // CPU oracle mutation controls over copied actual events, NOT additional native execution evidence.
  std::vector<Dx12CommandEvent> mutated(state->command_observation->events().begin(), state->command_observation->events().end());
  auto execute = std::find_if(mutated.begin(), mutated.end(), [&](const auto& e) {
    return e.parent == parents[1] && e.kind == Dx12CommandEventKind::execute_bundle;
  });
  if (execute == mutated.end()) return dx12_fail(41, "missing observed hit");
  execute->object = nullptr;
  if (reuse_native_history(mutated, parents)) return dx12_fail(41, "oracle accepted wrong native bundle identity");
  mutated.assign(state->command_observation->events().begin(), state->command_observation->events().end());
  auto table = std::find_if(mutated.begin(), mutated.end(), [&](const auto& e) {
    return e.parent == parents[0] && e.type == D3D12_COMMAND_LIST_TYPE_BUNDLE && e.kind == Dx12CommandEventKind::table && e.a == 1;
  });
  if (table == mutated.end()) return dx12_fail(42, "missing observed image table");
  table->b += 1;
  if (reuse_native_history(mutated, parents)) return dx12_fail(42, "oracle accepted corrupted image table");
  mutated.assign(state->command_observation->events().begin(), state->command_observation->events().end());
  auto sampler = std::find_if(mutated.begin(), mutated.end(), [&](const auto& e) {
    return e.parent == parents[0] && e.type == D3D12_COMMAND_LIST_TYPE_BUNDLE && e.kind == Dx12CommandEventKind::table && e.a == 2;
  });
  if (sampler == mutated.end()) return dx12_fail(43, "missing observed sampler table");
  sampler->b += 1;
  if (reuse_native_history(mutated, parents)) return dx12_fail(43, "oracle accepted corrupted sampler table");
  std::cout << "native fresh plus two immutable BUNDLE hits passed; no speedup or full-frame reuse claim\n";
  return 0;
}
