#include "dx12_glyph_native_test_support.hpp"
#include "../../src/renderer/dx12/dx12_buffers_internal.hpp"
#include <cstring>

namespace cgpui_test {
namespace {
struct ReadbackJob {
  cgpui::Dx12Ptr<ID3D12CommandAllocator> allocator;
  cgpui::Dx12Ptr<ID3D12GraphicsCommandList> commands;
  cgpui::Dx12Ptr<ID3D12Fence> fence;
  cgpui::Dx12Ptr<ID3D12Resource> source, buffer;
  HANDLE event = nullptr;
  ~ReadbackJob() { if (event) CloseHandle(event); }
};
}
cgpui::Result<std::vector<std::uint8_t>> glyph_readback(cgpui::Dx12State& state, ID3D12Resource& resource) {
  auto job = std::make_unique<ReadbackJob>();
  job->source = &resource;
  const auto desc = resource.GetDesc();
  if (desc.Format != DXGI_FORMAT_R8_UNORM || !desc.Width || !desc.Height)
    return std::unexpected(cgpui::dx12_error(E_INVALIDARG, "glyph readback format"));
  D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
  UINT rows = 0; UINT64 row_bytes = 0, bytes = 0;
  state.device->GetCopyableFootprints(&desc, 0, 1, 0, &footprint, &rows, &row_bytes, &bytes);
  // Readback intentionally never borrows the upload pool.
  auto buffer = cgpui::dx12_create_buffer(*state.device.Get(), static_cast<std::size_t>(bytes), D3D12_HEAP_TYPE_READBACK);
  if (!buffer) return std::unexpected(buffer.error());
  job->buffer = std::move(*buffer);
  HRESULT hr = state.device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&job->allocator));
  if (SUCCEEDED(hr)) hr = state.device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
      job->allocator.Get(), nullptr, IID_PPV_ARGS(&job->commands));
  if (SUCCEEDED(hr)) hr = state.device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&job->fence));
  if (FAILED(hr)) return std::unexpected(cgpui::dx12_error(hr, "glyph readback commands"));
  job->event = CreateEventW(nullptr, FALSE, FALSE, nullptr);
  if (!job->event) return std::unexpected(cgpui::dx12_error(HRESULT_FROM_WIN32(GetLastError()), "glyph readback event"));
  cgpui::dx12_transition(*job->commands.Get(), resource,
      D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE);
  D3D12_TEXTURE_COPY_LOCATION from{}, to{};
  from.pResource = &resource; from.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
  to.pResource = job->buffer.Get(); to.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
  to.PlacedFootprint = footprint;
  job->commands->CopyTextureRegion(&to, 0, 0, 0, &from, nullptr);
  cgpui::dx12_transition(*job->commands.Get(), resource,
      D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
  hr = job->commands->Close();
  if (FAILED(hr)) return std::unexpected(cgpui::dx12_error(hr, "glyph readback close"));
  ID3D12CommandList* lists[] = {job->commands.Get()};
  state.queue->ExecuteCommandLists(1, lists);
  hr = state.queue->Signal(job->fence.Get(), 1);
  if (SUCCEEDED(hr)) hr = job->fence->SetEventOnCompletion(1, job->event);
  const bool completed = SUCCEEDED(hr) && WaitForSingleObject(job->event, 10000) == WAIT_OBJECT_0 &&
      job->fence->GetCompletedValue() >= 1 && job->fence->GetCompletedValue() != UINT64_MAX;
  if (!completed) {
    (void)job.release(); // Failure-only retention: never free unknown-GPU test objects on timeout.
    return std::unexpected(cgpui::dx12_error(E_FAIL, "glyph readback completion unproven"));
  }
  std::vector<std::uint8_t> result(static_cast<std::size_t>(desc.Width) * desc.Height);
  const D3D12_RANGE read{0, static_cast<SIZE_T>(bytes)};
  void* mapped = nullptr;
  hr = job->buffer->Map(0, &read, &mapped);
  if (FAILED(hr)) return std::unexpected(cgpui::dx12_error(hr, "glyph readback map"));
  for (UINT row = 0; row < desc.Height; ++row)
    std::memcpy(result.data() + std::size_t(row) * desc.Width,
        static_cast<const std::uint8_t*>(mapped) + footprint.Offset + std::size_t(row) * footprint.Footprint.RowPitch,
        static_cast<std::size_t>(desc.Width));
  const D3D12_RANGE no_write{0, 0}; job->buffer->Unmap(0, &no_write);
  if (auto checked = cgpui::dx12_check_debug_errors(state); !checked) return std::unexpected(checked.error());
  return result;
}
} // namespace cgpui_test
