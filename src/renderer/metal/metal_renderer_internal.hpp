#pragma once

#include "cgpui/renderer/renderer.hpp"

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>

namespace cgpui {

class MetalFramePacingState {
 public:
  [[nodiscard]] bool try_acquire();
  void release();
  [[nodiscard]] std::size_t in_flight_count() const;

 private:
  static constexpr std::size_t max_frames_in_flight_ = 3;
  mutable std::mutex mutex_;
  std::size_t in_flight_count_ = 0;
};

struct MetalAcquiredFrame {
  __strong id<CAMetalDrawable> drawable = nil;
  __strong id<MTLCommandBuffer> command_buffer = nil;
  std::uint64_t frame_id = 0;
  RendererFrameTimings timings{};
  bool pacing_slot = false;
};

struct MetalRendererState {
  __strong CAMetalLayer* layer = nil;
  __strong id<MTLDevice> device = nil;
  __strong id<MTLCommandQueue> command_queue = nil;
  Size framebuffer_size{};
  DpiScale scale{};
  std::atomic<std::uint64_t> next_frame_id{1};
  MetalFramePacingState pacing;
  RendererFrameDiagnosticSnapshot last_snapshot{};
  bool has_snapshot = false;
  mutable std::mutex snapshot_mutex;

  Result<void> resize(Size framebuffer_size, DpiScale scale);
  Result<MetalAcquiredFrame> acquire_frame();
  void record_clear_frame(RendererFrameTimings timings);
  [[nodiscard]] const RendererFrameDiagnosticSnapshot* snapshot() const;
};

Result<void> configure_metal_surface(
    MetalRendererState& state,
    Size framebuffer_size,
    DpiScale scale,
    bool transparent_background);
Result<std::shared_ptr<MetalRendererState>> create_metal_renderer_state(
    const RenderSurfaceDescriptor& descriptor);
std::unique_ptr<Renderer> make_metal_renderer(
    std::shared_ptr<MetalRendererState> state);
Result<std::unique_ptr<RenderFrame>> make_metal_frame(
    std::shared_ptr<MetalRendererState> state);

}  // namespace cgpui
