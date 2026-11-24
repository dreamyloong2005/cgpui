#include "dx12_command_reuse_native_test_support.hpp"
#include "../../src/renderer/dx12/dx12_glyph_command_api_internal.hpp"
#include "../../src/renderer/dx12/dx12_glyph_native_internal.hpp"
#include "../../src/renderer/dx12/dx12_texture_renderer_internal.hpp"
#include "../../src/renderer/dx12/dx12_native_retirement_internal.hpp"
#include <algorithm>

using namespace cgpui;
using namespace cgpui_test;
namespace {
struct FailedCommands final : Dx12GlyphCommandApi {
  unsigned fault = 0;
  HRESULT reset_allocator(ID3D12CommandAllocator& allocator) noexcept override {
    return fault == 1 ? E_ACCESSDENIED : dx12_native_glyph_command_api().reset_allocator(allocator);
  }
  HRESULT reset_list(ID3D12GraphicsCommandList& list, ID3D12CommandAllocator& allocator, ID3D12PipelineState* pso) noexcept override {
    return fault == 2 ? E_ACCESSDENIED : dx12_native_glyph_command_api().reset_list(list, allocator, pso);
  }
  HRESULT close(ID3D12GraphicsCommandList& list) noexcept override {
    return fault == 3 ? E_ACCESSDENIED : dx12_native_glyph_command_api().close(list);
  }
};
std::size_t count(const Dx12State& state, Dx12CommandEventKind kind) {
  return std::ranges::count(state.command_observation->events(), kind, &Dx12CommandEvent::kind);
}
int run(unsigned fault) {
  FailedCommands commands;
  Dx12Window window; auto state = srv_native_state(&window);
  if (!state) return dx12_fail(1, "reuse failure native setup");
  state->command_observation = std::make_unique<Dx12CommandObservation>(8192);
  if (!reuse_scene(state, true, true) || !reuse_scene(state, false, false) ||
      count(*state, Dx12CommandEventKind::bundle_created) != 1 || !dx12_check_debug_errors(*state))
    return dx12_fail(2, "reuse failure fresh native bundle prerequisite");
  const auto pixels = state->pixels->rgba8;
  const auto diagnostics = *state->diagnostics;
  const auto submissions = count(*state, Dx12CommandEventKind::producer_execute);
  commands.fault = fault; state->texture_renderer->glyph_native->command_api = &commands;
  if (reuse_scene(state, false, false) || state->submitted || state->pixels->rgba8 != pixels ||
      !diagnostic_same(&*state->diagnostics, diagnostics) ||
      count(*state, Dx12CommandEventKind::producer_execute) != submissions ||
      count(*state, Dx12CommandEventKind::bundle_released) || state->srv_pool->stats().live_slots != 2)
    return dx12_fail(3, "Reset/Close failure published, executed or freed an old parent bundle/slot");
  // Actual old submission completed; failed current attempt never executed. No public retry.
  dx12_release_native_objects(*state);
  state->texture_renderer->glyph_native->command_api = nullptr;
  const auto events = state->command_observation->events();
  auto parent = std::find_if(events.begin(), events.end(), [](const auto& e) { return e.kind == Dx12CommandEventKind::parent_released; });
  auto bundle = std::find_if(events.begin(), events.end(), [](const auto& e) { return e.kind == Dx12CommandEventKind::bundle_released; });
  if (parent == events.end() || bundle == events.end() || parent >= bundle || state->srv_pool->stats().live_slots || state->device)
    return dx12_fail(4, "native parent-first release did not retire bundle before native slot/pool/device");
  return 0;
}
}
int main() {
  Dx12RecoveryDeadline deadline;
  if (!deadline.done) return dx12_fail(90, "reuse failure deadline unavailable");
  for (unsigned fault = 1; fault <= 3; ++fault) if (const auto result = run(fault)) return result;
  return 0;
}
