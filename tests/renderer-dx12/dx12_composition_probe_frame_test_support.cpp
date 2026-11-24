#include "dx12_composition_probe_test_support.hpp"
#include "../../src/renderer/dx12/dx12_composition_internal.hpp"
#include <cstdio>

namespace cgpui_test {
bool Dx12CompositionProbe::record_pixels(bool blue) {
  using namespace cgpui;
  if (!result("pre_record_idle", dx12_wait_idle(*state)) ||
      !result("prepare_capture", dx12_prepare_capture(*state)) ||
      !native("allocator_reset", state->allocator->Reset()) ||
      !native("command_reset", state->commands->Reset(state->allocator.Get(), nullptr))) return false;
  const auto index = state->swapchain->GetCurrentBackBufferIndex();
  auto& target = *state->targets[index].Get();
  auto rtv = state->rtv_heap->GetCPUDescriptorHandleForHeapStart();
  rtv.ptr += std::size_t(index) * state->rtv_stride;
  dx12_transition(*state->commands.Get(), target, D3D12_RESOURCE_STATE_PRESENT,
      D3D12_RESOURCE_STATE_RENDER_TARGET);
  const float clear[] = {0, 0, 0, 0};
  const float half[] = {blue ? 0.0F : 0.5F, 0, blue ? 0.5F : 0.0F, 0.5F};
  const float full[] = {blue ? 0.0F : 1.0F, 0, blue ? 1.0F : 0.0F, 1};
  const D3D12_RECT left{0, 0, 20, static_cast<LONG>(state->height)};
  const D3D12_RECT right{40, 0, static_cast<LONG>(state->width), static_cast<LONG>(state->height)};
  state->commands->ClearRenderTargetView(rtv, clear, 0, nullptr);
  state->commands->ClearRenderTargetView(rtv, half, 1, &left);
  state->commands->ClearRenderTargetView(rtv, full, 1, &right);
  dx12_transition(*state->commands.Get(), target, D3D12_RESOURCE_STATE_RENDER_TARGET,
      D3D12_RESOURCE_STATE_COPY_SOURCE);
  dx12_record_capture(*state, target);
  dx12_transition(*state->commands.Get(), target, D3D12_RESOURCE_STATE_COPY_SOURCE,
      D3D12_RESOURCE_STATE_PRESENT);
  return native("command_close", state->commands->Close());
}
bool Dx12CompositionProbe::present_pixels(bool blue) {
  using namespace cgpui;
  if (!await_admission(!state->composition_host->committed) || !record_pixels(blue)) return false;
  ID3D12CommandList* lists[] = {state->commands.Get()};
  state->queue->ExecuteCommandLists(1, lists);
  state->submitted = true;
  const HRESULT presented = state->swapchain->Present(1, 0);
  state->frame_pacing->presentation_attempted(); // Always after the actual attempt, including errors.
  composition_probe_hresult("Present", presented);
  if (!result("post_present_retirement", dx12_wait_idle(*state))) return false;
  if (FAILED(presented)) { outcome = CompositionProbeExit::unmeasured; return false; }
  auto pixels = dx12_read_capture(*state);
  if (!pixels) return result("capture_readback", std::unexpected(pixels.error()));
  composition_probe_note("capture_readback", "ok");
  auto& host = *state->composition_host;
  if (!host.committed) {
    if (!native("Commit_after_first_Present", host.device->Commit())) return false;
    host.committed = true;
  }
  if (!debug_clean()) return false;
  state->pixels = std::move(*pixels);
  return verify_pixels(blue) &&
      invariant(windows.visibility_unchanged(), "presentation changed hidden/visible state");
}
} // namespace cgpui_test
