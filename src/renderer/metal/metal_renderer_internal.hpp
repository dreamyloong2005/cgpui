#pragma once

#include "cgpui/renderer/renderer.hpp"

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#include <simd/simd.h>

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <vector>

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

struct MetalFrameDrawOrderEntry {
  RendererPrimitiveKind primitive_kind{};
  std::size_t command_index = 0;

  friend bool operator==(
      const MetalFrameDrawOrderEntry&,
      const MetalFrameDrawOrderEntry&) = default;
};

struct MetalVertex {
  simd_float2 position{};
  simd_float2 texture_coordinate{};
  simd_float4 color{};
};

struct MetalRoundedRectUniforms {
  simd_float4 fill_color{};
  simd_float4 border_color{};
  simd_float4 radii{};
  simd_float2 size{};
  float border_width = 0.0F;
  float fill_enabled = 1.0F;
};

struct MetalPipelineLibrary {
  __strong id<MTLLibrary> library = nil;
  __strong id<MTLRenderPipelineState> solid = nil;
  __strong id<MTLRenderPipelineState> rounded = nil;
  __strong id<MTLRenderPipelineState> glyph = nil;
  __strong id<MTLRenderPipelineState> image = nil;
  __strong id<MTLSamplerState> nearest_sampler = nil;
  __strong id<MTLSamplerState> linear_sampler = nil;
};

struct MetalImageTextureResource {
  __strong id<MTLTexture> texture = nil;
  ImageAssetDescriptor descriptor{};
};

struct MetalGlyphPageResource {
  __strong id<MTLTexture> texture = nil;
  std::size_t page_index = 0;
};

struct MetalFrameCommands {
  std::vector<SolidRect> solid_rects;
  std::vector<RoundedRectDraw> rounded_rects;
  std::vector<TextDraw> text_draws;
  std::vector<TextSelectionDraw> selections;
  std::vector<TextCaretDraw> carets;
  std::vector<ImageAsset> image_uploads;
  std::vector<ImageAssetId> image_invalidations;
  std::vector<ImageDraw> image_draws;
  std::vector<MetalFrameDrawOrderEntry> draw_order;
  bool capture_requested = false;
};

struct MetalClipResolution {
  MTLScissorRect scissor{};
  bool visible = false;
};

struct MetalRendererState {
  __strong CAMetalLayer* layer = nil;
  __strong id<MTLDevice> device = nil;
  __strong id<MTLCommandQueue> command_queue = nil;
  Size framebuffer_size{};
  DpiScale scale{};
  std::atomic<std::uint64_t> next_frame_id{1};
  MetalFramePacingState pacing;
  MetalPipelineLibrary pipelines;
  GlyphCache glyph_cache;
  std::vector<MetalGlyphPageResource> glyph_pages;
  std::unordered_map<std::uint64_t, MetalImageTextureResource> images;
  RendererFrameDiagnosticSnapshot last_snapshot{};
  RendererFramePixels last_pixels{};
  bool has_snapshot = false;
  bool has_pixels = false;
  mutable std::mutex snapshot_mutex;

  Result<void> resize(Size framebuffer_size, DpiScale scale);
  Result<MetalAcquiredFrame> acquire_frame();
  void record_frame(
      RendererFrameTimings timings,
      const MetalFrameCommands& commands,
      std::size_t batch_count,
      std::size_t glyph_upload_byte_count,
      std::size_t image_upload_byte_count);
  void store_pixels(
      id<MTLBuffer> buffer,
      std::uint32_t width,
      std::uint32_t height,
      std::size_t row_byte_count);
  [[nodiscard]] const RendererFrameDiagnosticSnapshot* snapshot() const;
  [[nodiscard]] const RendererFramePixels* pixels() const;
};

Result<void> configure_metal_surface(
    MetalRendererState& state,
    Size framebuffer_size,
    DpiScale scale,
    bool transparent_background);
Result<std::shared_ptr<MetalRendererState>> create_metal_renderer_state(
    const RenderSurfaceDescriptor& descriptor);
Result<void> initialize_metal_pipeline_library(MetalRendererState& state);
std::unique_ptr<Renderer> make_metal_renderer(
    std::shared_ptr<MetalRendererState> state);
Result<std::unique_ptr<RenderFrame>> make_metal_frame(
    std::shared_ptr<MetalRendererState> state);

std::array<MetalVertex, 4> metal_rect_vertices(
    Rect rect,
    Color color,
    PaintMetadata metadata,
    const RendererCompositionStackRecord& composition_stack,
    Rect texture_coordinates = {{}, {1.0F, 1.0F}});
MetalClipResolution metal_resolve_clip(
    Size framebuffer_size,
    std::optional<Rect> clip_rect,
    const RendererClipStackRecord& clip_stack);
std::vector<RendererCommandBatch> metal_build_command_batches(
    const MetalFrameCommands& commands);
Result<void> metal_encode_solid_rect(
    id<MTLRenderCommandEncoder> encoder,
    MetalRendererState& state,
    const SolidRect& rect);
Result<void> metal_encode_rounded_rect(
    id<MTLRenderCommandEncoder> encoder,
    MetalRendererState& state,
    const RoundedRectDraw& rect);
Result<std::size_t> metal_encode_text(
    id<MTLRenderCommandEncoder> encoder,
    MetalRendererState& state,
    const TextDraw& text);
Result<std::size_t> metal_upload_images(
    MetalRendererState& state,
    const MetalFrameCommands& commands);
Result<void> metal_encode_image(
    id<MTLRenderCommandEncoder> encoder,
    MetalRendererState& state,
    const ImageDraw& image);
id<MTLBuffer> metal_encode_pixel_capture(
    MetalRendererState& state,
    id<MTLCommandBuffer> command_buffer,
    id<MTLTexture> texture,
    std::size_t& row_byte_count);

}  // namespace cgpui
