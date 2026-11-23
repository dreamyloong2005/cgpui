#include "dx12_frame_internal.hpp"
#include "dx12_image_cache_internal.hpp"
#include "renderer_device_pixels_internal.hpp"
#include "cgpui/renderer/planning/image_geometry.hpp"
#include "cgpui/renderer/planning/clip_scissor.hpp"

namespace cgpui {
void Dx12Frame::upload_image(const ImageAsset& image) {
  if (released_ || error_) return;
  if (auto uploaded = state_->image_cache->upload(*state_, image); !uploaded) error_ = uploaded.error();
}
void Dx12Frame::invalidate_image(ImageAssetId id) {
  if (released_ || error_) return;
  state_->image_cache->invalidate(id);
}
void Dx12Frame::draw_image(const ImageDraw& image) {
  if (released_ || error_) return;
  auto texture = state_->image_cache->find(image.asset);
  if (!texture) { error_ = texture.error(); return; }
  const auto device = renderer_to_device_pixels(image, state_->scale);
  Dx12TexturedDraw draw{.texture = std::move(*texture),
      .clip_rect = resolve_effective_clip_rect(device.clip_rect, device.clip_stack),
      .sampling = device.sampling};
  for (const auto& vertex : build_image_vertices(std::span{&device, 1})) {
    draw.vertices.push_back({vertex.position, vertex.image_uv, vertex.color});
  }
  append(std::move(draw));
}
} // namespace cgpui
