#pragma once
#include "dx12_srv_topology_test_support.hpp"
#include "dx12_recovery_test_support.hpp"
#include "../../src/renderer/dx12/dx12_command_observation_internal.hpp"
#include "../../src/renderer/dx12/dx12_srv_descriptor_pool_internal.hpp"

namespace cgpui_test {
class ReuseQueueTail final : public cgpui::Dx12CommandVerifier {
 public:
  void prepare(cgpui::Dx12State&, ID3D12Resource&, std::uint64_t) noexcept override;
  ID3D12CommandList* prepared_list() const noexcept override;
  void completed(cgpui::Dx12State&, std::uint64_t, std::uint64_t) noexcept override;
  void retired(bool removed) noexcept override;
  bool failed = false;
  std::uint64_t parent = 0, completed_parent = 0, ticket = 0;
  ID3D12Resource* target_identity = nullptr;
  cgpui::RendererFramePixels pixels;
 private:
  cgpui::Result<void> prepare_checked(cgpui::Dx12State&, ID3D12Resource&);
  cgpui::Dx12Ptr<ID3D12CommandAllocator> allocator_;
  cgpui::Dx12Ptr<ID3D12GraphicsCommandList> commands_;
  cgpui::Dx12Ptr<ID3D12Resource> target_, buffer_;
  D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout_{};
  UINT64 bytes_ = 0;
  bool ready_ = false;
};
bool reuse_scene(const std::shared_ptr<cgpui::Dx12State>&, bool upload, bool capture);
bool reuse_trace_pixels_ready(const cgpui::Dx12State&, const ReuseQueueTail&,
                             std::span<const std::uint8_t> expected);
bool reuse_native_history(std::span<const cgpui::Dx12CommandEvent>,
                          std::span<const std::uint64_t> parents);
} // namespace cgpui_test
