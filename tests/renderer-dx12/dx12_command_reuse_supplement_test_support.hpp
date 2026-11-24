#pragma once
#include "dx12_command_reuse_native_test_support.hpp"
#include "../../src/renderer/dx12/dx12_glyph_command_api_internal.hpp"
#include "../../src/renderer/dx12/dx12_texture_renderer_internal.hpp"

namespace cgpui_test {
std::size_t reuse_event_count(const cgpui::Dx12State&, cgpui::Dx12CommandEventKind);
ID3D12GraphicsCommandList* reuse_last_bundle(const cgpui::Dx12State&);
bool reuse_parent_before_bundle_release(const cgpui::Dx12State&);
bool reuse_native_sentinel(ID3D12Object&, std::atomic_bool&);
cgpui::Result<std::vector<cgpui::Dx12DrawCommand>> reuse_raw_scene(cgpui::Dx12State&);
cgpui::Result<void> reuse_raw_submit(cgpui::Dx12State&, std::span<const cgpui::Dx12DrawCommand>);
class ReuseFailListReset final : public cgpui::Dx12GlyphCommandApi {
 public:
  unsigned allocator_calls = 0, list_calls = 0, close_calls = 0;
  HRESULT allocator_result = E_UNEXPECTED, list_result = E_UNEXPECTED, close_result = E_UNEXPECTED;
  HRESULT reset_allocator(ID3D12CommandAllocator&) noexcept override;
  HRESULT reset_list(ID3D12GraphicsCommandList&, ID3D12CommandAllocator&, ID3D12PipelineState*) noexcept override;
  HRESULT close(ID3D12GraphicsCommandList&) noexcept override;
};
} // namespace cgpui_test
