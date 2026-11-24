#include "dx12_glyph_command_api_internal.hpp"
namespace cgpui {
namespace {
class Native final : public Dx12GlyphCommandApi {
 public:
  HRESULT reset_allocator(ID3D12CommandAllocator& allocator) noexcept override { return allocator.Reset(); }
  HRESULT reset_list(ID3D12GraphicsCommandList& list, ID3D12CommandAllocator& allocator,
                     ID3D12PipelineState* pipeline) noexcept override { return list.Reset(&allocator, pipeline); }
  HRESULT close(ID3D12GraphicsCommandList& list) noexcept override { return list.Close(); }
};
}
Dx12GlyphCommandApi& dx12_native_glyph_command_api() noexcept { static Native api; return api; }
} // namespace cgpui
