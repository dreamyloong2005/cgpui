#pragma once

#include "dx12_native_internal.hpp"
#include <array>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>

namespace cgpui {
class Dx12ImageCache;
struct Dx12TextCache;
struct Dx12TextureRenderer;
struct Dx12BackdropBlurRenderer;
struct Dx12CompositionHost;
struct Dx12WindowAlphaLease;
class Dx12SynchronizationApi;
struct Dx12State {
  Dx12State();
  ~Dx12State();
  mutable std::recursive_mutex operation_mutex;
  std::size_t operation_depth = 0;
  DWORD window_thread_id = 0;
  Dx12Ptr<IDXGIFactory4> factory;
  Dx12Ptr<IDXGIAdapter1> adapter;
  Dx12Ptr<ID3D12Device> device;
  Dx12Ptr<ID3D12CommandQueue> queue;
  Dx12Ptr<ID3D12Fence> fence;
  Dx12Event fence_event;
  std::uint64_t fence_value = 0;
  std::uint64_t pending_fence_value = 0;
  HRESULT removal_reason = S_OK;
  Dx12Ptr<IDXGISwapChain3> swapchain;
  std::unique_ptr<Dx12CompositionHost> composition_host;
  std::unique_ptr<Dx12WindowAlphaLease> window_alpha;
  Dx12Ptr<ID3D12DescriptorHeap> rtv_heap;
  std::array<Dx12Ptr<ID3D12Resource>, 2> targets;
  UINT rtv_stride = 0;
  Dx12Ptr<ID3D12CommandAllocator> allocator;
  Dx12Ptr<ID3D12GraphicsCommandList> commands;
  Dx12Ptr<ID3D12RootSignature> root_signature;
  Dx12Ptr<ID3D12PipelineState> pipeline;
  Dx12Ptr<ID3D12PipelineState> shadow_pipeline;
  std::unique_ptr<Dx12ImageCache> image_cache;
  std::unique_ptr<Dx12TextCache> text_cache;
  std::unique_ptr<Dx12TextureRenderer> texture_renderer;
  std::unique_ptr<Dx12BackdropBlurRenderer> backdrop_blur;
  std::vector<Dx12Ptr<ID3D12Resource>> uploads;
  Dx12Ptr<ID3D12Resource> readback;
  D3D12_PLACED_SUBRESOURCE_FOOTPRINT capture_layout{};
  std::optional<RendererFramePixels> pixels;
  RendererBackendInfo info;
  HWND hwnd = nullptr;
  UINT width = 0, height = 0;
  DpiScale scale{1};
  bool owner_alive = true;
  bool frame_active = false;
  bool submitted = false;
  bool failed = false;
  bool recovery_pending = false;
  bool composition_surface = false;
  bool transparent_background = false;
  // Optional private test boundary; its owner must outlive this state.
  Dx12SynchronizationApi* synchronization_api = nullptr;
};

Result<void> dx12_create_device(Dx12State& state);
Result<void> dx12_create_swapchain(Dx12State& state);
Result<void> dx12_create_targets(Dx12State& state);
Result<void> dx12_resize(Dx12State& state, Size size, DpiScale scale);
Result<void> dx12_create_pipeline(Dx12State& state);
Result<void> dx12_wait_idle(Dx12State& state);
Result<void> dx12_check_debug_errors(Dx12State& state);
Result<void> dx12_prepare_capture(Dx12State& state);
void dx12_record_capture(Dx12State& state, ID3D12Resource& target);
Result<void> dx12_read_capture(Dx12State& state);
void dx12_transition(ID3D12GraphicsCommandList& commands, ID3D12Resource& resource,
                     D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);
} // namespace cgpui
