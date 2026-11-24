#pragma once
#include "dx12_native_internal.hpp"

namespace cgpui {
// Private forwarding seam for command-recording lifetime failures, not pacing or execution policy.
class Dx12GlyphCommandApi {
 public:
  virtual ~Dx12GlyphCommandApi() = default;
  virtual HRESULT reset_allocator(ID3D12CommandAllocator&) noexcept = 0;
  virtual HRESULT reset_list(ID3D12GraphicsCommandList&, ID3D12CommandAllocator&, ID3D12PipelineState*) noexcept = 0;
  virtual HRESULT close(ID3D12GraphicsCommandList&) noexcept = 0;
};
Dx12GlyphCommandApi& dx12_native_glyph_command_api() noexcept;
} // namespace cgpui
