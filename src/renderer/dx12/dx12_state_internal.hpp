#pragma once

#include "dx12_native_internal.hpp"
#include "dx12_dred_site_internal.hpp"
#include "dx12_surface_policy_internal.hpp"
#include "dx12_state_operations_internal.hpp"
#include "../../core/native_window_work_internal.hpp"
#include "cgpui/renderer/renderer_frame_diagnostic_snapshot.hpp"
#include <array>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>

namespace cgpui {
class Dx12ImageCache;
struct Dx12TextCache;
struct Dx12TextureRenderer;
struct Dx12BackdropBlurRenderer;
struct Dx12CompositionHost;
struct Dx12WindowAlphaLease;
class Dx12SynchronizationApi;
class Dx12FrameDiagnostics;
class Dx12GeometryUploads;
class Dx12TextureUploads;
class Dx12SrvDescriptorPool;
class Dx12FrameCommandReuse;
class Dx12CommandObservation;
class Dx12FramePacing;
class Dx12FramePacingApi;
struct Dx12RecoveryLease;
struct Dx12RecoveryTicket;
struct Dx12State : std::enable_shared_from_this<Dx12State> {
  explicit Dx12State(Dx12FramePacingApi* pacing_api = nullptr,
                     Dx12SynchronizationApi* synchronization_api = nullptr);
  ~Dx12State();
  mutable std::recursive_mutex operation_mutex;
  std::size_t operation_depth = 0;
  DWORD window_thread_id = 0;
  std::shared_ptr<Dx12RecoveryLease> recovery_lease;
  std::shared_ptr<Dx12RecoveryTicket> recovery_ticket;
  std::shared_ptr<NativeWindowWork> recovery_work;
  NativeWindowWorkToken recovery_work_token;
  Dx12Ptr<IDXGIFactory4> factory;
  Dx12Ptr<IDXGIAdapter1> adapter;
  Dx12Ptr<ID3D12Device> device;
  Dx12Ptr<ID3D12CommandQueue> queue;
  Dx12Ptr<ID3D12Fence> fence;
  Dx12Event fence_event;
  std::uint64_t fence_value = 0;
  std::uint64_t pending_fence_value = 0;
  HRESULT removal_reason = S_OK;
  Dx12DredSite dred_status;
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
  std::unique_ptr<Dx12SrvDescriptorPool> srv_pool;
  std::unique_ptr<Dx12TextureRenderer> texture_renderer;
  std::unique_ptr<Dx12BackdropBlurRenderer> backdrop_blur;
  std::unique_ptr<Dx12GeometryUploads> geometry_uploads;
  std::unique_ptr<Dx12TextureUploads> texture_uploads;
  std::unique_ptr<Dx12FramePacing> frame_pacing;
  std::unique_ptr<Dx12FrameCommandReuse> command_reuse;
  std::unique_ptr<Dx12CommandObservation> command_observation; // Optional state-owned native test attachment.
  Dx12Ptr<ID3D12Resource> readback;
  D3D12_PLACED_SUBRESOURCE_FOOTPRINT capture_layout{};
  std::optional<RendererFramePixels> pixels;
  // Last successful frame, independent of the current surface/device generation.
  std::optional<RendererFrameDiagnosticSnapshot> diagnostics;
  Dx12FrameDiagnostics* active_diagnostics = nullptr;
  RendererBackendInfo info;
  HWND hwnd = nullptr;
  UINT width = 0, height = 0;
  DpiScale scale{1};
  bool owner_alive = true;
  bool frame_active = false;
  bool submitted = false;
  bool failed = false;
  bool recovery_pending = false;
  Dx12SurfacePolicy surface_policy;
  bool surface_changing = false;
  bool composition_surface = false; // Actual PREMULT alpha, independent of the native route.
  bool transparent_background = false;
  // Optional private test boundary; its owner must outlive this state.
  Dx12SynchronizationApi* synchronization_api = nullptr;
};

} // namespace cgpui
