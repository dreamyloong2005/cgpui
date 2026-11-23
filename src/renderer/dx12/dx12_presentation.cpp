#include "dx12_frame_internal.hpp"

namespace cgpui {
Result<void> dx12_submit_frame(Dx12State& state, Color clear,
                              std::span<const RectGeometry> draws, bool capture) {
  if (auto idle = dx12_wait_idle(state); !idle) return idle;
  state.uploads.clear();
  state.readback.Reset();
  if (capture) {
    if (auto prepared = dx12_prepare_capture(state); !prepared) return prepared;
  }
  HRESULT result = state.allocator->Reset();
  if (FAILED(result)) return std::unexpected(dx12_error(result, "allocator reset"));
  result = state.commands->Reset(state.allocator.Get(), state.pipeline.Get());
  if (FAILED(result)) return std::unexpected(dx12_error(result, "command list reset"));
  const UINT index = state.swapchain->GetCurrentBackBufferIndex();
  auto& target = *state.targets[index].Get();
  dx12_transition(*state.commands.Get(), target, D3D12_RESOURCE_STATE_PRESENT,
                   D3D12_RESOURCE_STATE_RENDER_TARGET);
  auto handle = state.rtv_heap->GetCPUDescriptorHandleForHeapStart();
  handle.ptr += std::size_t(index) * state.rtv_stride;
  state.commands->OMSetRenderTargets(1, &handle, FALSE, nullptr);
  const float color[] = {clear.r, clear.g, clear.b, clear.a};
  state.commands->ClearRenderTargetView(handle, color, 0, nullptr);
  const D3D12_VIEWPORT viewport{0, 0, static_cast<float>(state.width),
                               static_cast<float>(state.height), 0, 1};
  state.commands->RSSetViewports(1, &viewport);
  state.commands->SetGraphicsRootSignature(state.root_signature.Get());
  const float extent[] = {static_cast<float>(state.width), static_cast<float>(state.height)};
  state.commands->SetGraphicsRoot32BitConstants(0, 2, extent, 0);
  state.commands->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  for (const auto& geometry : draws) {
    if (auto recorded = dx12_record_geometry(state, geometry); !recorded) {
      (void)state.commands->Close();
      return recorded;
    }
  }
  if (capture) {
    dx12_transition(*state.commands.Get(), target, D3D12_RESOURCE_STATE_RENDER_TARGET,
                     D3D12_RESOURCE_STATE_COPY_SOURCE);
    dx12_record_capture(state, target);
  }
  dx12_transition(*state.commands.Get(), target,
      capture ? D3D12_RESOURCE_STATE_COPY_SOURCE : D3D12_RESOURCE_STATE_RENDER_TARGET,
      D3D12_RESOURCE_STATE_PRESENT);
  result = state.commands->Close();
  if (FAILED(result)) return std::unexpected(dx12_error(result, "command list close"));
  ID3D12CommandList* lists[] = {state.commands.Get()};
  state.queue->ExecuteCommandLists(1, lists);
  state.submitted = true;
  const HRESULT presented = state.swapchain->Present(1, 0);
  // The fence follows presentation as well as drawing; resize/destruction
  // cannot retire resources while DXGI is still using this queue's frame.
  if (auto idle = dx12_wait_idle(state); !idle) return idle;
  if (FAILED(presented)) return std::unexpected(dx12_error(presented, "present"));
  if (capture) {
    if (auto read = dx12_read_capture(state); !read) return read;
  }
  if (auto checked = dx12_check_debug_errors(state); !checked) return checked;
  // DXGI_STATUS_OCCLUDED is a valid hidden-window presentation. The render
  // and optional readback completed; avoid a tight retry loop for hidden HWNDs.
  return {};
}
} // namespace cgpui
