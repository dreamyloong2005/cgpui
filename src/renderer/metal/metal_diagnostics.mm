#include "metal_renderer_internal.hpp"

namespace cgpui {

void MetalRendererState::record_frame(
    RendererFrameTimings timings,
    const MetalFrameCommands& commands,
    std::size_t batch_count,
    std::size_t glyph_upload_byte_count,
    std::size_t image_upload_byte_count) {
  const std::size_t upload_byte_count =
      glyph_upload_byte_count + image_upload_byte_count;
  const std::size_t draw_count = commands.draw_order.size();
  const RendererFrameWork work{
      .batch_count = batch_count,
      .command_count = std::max<std::size_t>(1, draw_count),
      .upload_byte_count = upload_byte_count,
      .draw_count = draw_count};
  RendererDrawCounts draws;
  for (const MetalFrameDrawOrderEntry entry : commands.draw_order) {
    renderer_add_draw_count(draws, entry.primitive_kind);
  }
  RendererUploadByteCounts uploads{
      .glyph_atlas_byte_count = glyph_upload_byte_count,
      .image_byte_count = image_upload_byte_count,
      .total_byte_count = upload_byte_count};
  const std::scoped_lock lock(snapshot_mutex);
  last_snapshot = RendererFrameDiagnosticSnapshot{
      .work = compare_renderer_frame_work(work, work),
      .upload_bytes = uploads,
      .planned_draws = draws,
      .submitted_draws = draws,
      .timings = timings};
  has_snapshot = true;
}

id<MTLBuffer> metal_encode_pixel_capture(
    MetalRendererState& state,
    id<MTLCommandBuffer> command_buffer,
    id<MTLTexture> texture,
    std::size_t& row_byte_count) {
  row_byte_count = ((texture.width * 4U + 255U) / 256U) * 256U;
  id<MTLBuffer> buffer = [state.device
      newBufferWithLength:row_byte_count * texture.height
                  options:MTLResourceStorageModeShared];
  if (buffer == nil) return nil;
  id<MTLBlitCommandEncoder> encoder = [command_buffer blitCommandEncoder];
  if (encoder == nil) return nil;
  [encoder copyFromTexture:texture
               sourceSlice:0
               sourceLevel:0
              sourceOrigin:MTLOriginMake(0, 0, 0)
                sourceSize:MTLSizeMake(texture.width, texture.height, 1)
                  toBuffer:buffer
         destinationOffset:0
    destinationBytesPerRow:row_byte_count
  destinationBytesPerImage:row_byte_count * texture.height];
  [encoder endEncoding];
  return buffer;
}

void MetalRendererState::store_pixels(
    id<MTLBuffer> buffer,
    std::uint32_t width,
    std::uint32_t height,
    std::size_t row_byte_count) {
  RendererFramePixels pixels{
      .width = width,
      .height = height,
      .encoding = RendererFramePixelEncoding::linear,
      .rgba8 = std::vector<std::uint8_t>(
          static_cast<std::size_t>(width) * height * 4U)};
  const auto* source = static_cast<const std::uint8_t*>(buffer.contents);
  for (std::uint32_t y = 0; y < height; ++y) {
    for (std::uint32_t x = 0; x < width; ++x) {
      const std::size_t source_index =
          static_cast<std::size_t>(y) * row_byte_count + x * 4U;
      const std::size_t destination_index =
          (static_cast<std::size_t>(y) * width + x) * 4U;
      pixels.rgba8[destination_index] = source[source_index + 2U];
      pixels.rgba8[destination_index + 1U] = source[source_index + 1U];
      pixels.rgba8[destination_index + 2U] = source[source_index];
      pixels.rgba8[destination_index + 3U] = source[source_index + 3U];
    }
  }
  const std::scoped_lock lock(snapshot_mutex);
  last_pixels = std::move(pixels);
  has_pixels = true;
}

const RendererFrameDiagnosticSnapshot* MetalRendererState::snapshot() const {
  const std::scoped_lock lock(snapshot_mutex);
  return has_snapshot ? &last_snapshot : nullptr;
}

const RendererFramePixels* MetalRendererState::pixels() const {
  const std::scoped_lock lock(snapshot_mutex);
  return has_pixels ? &last_pixels : nullptr;
}

}  // namespace cgpui
