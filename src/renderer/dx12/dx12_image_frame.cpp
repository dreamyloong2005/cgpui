#include "dx12_operation_internal.hpp"
#include "dx12_frame_internal.hpp"
#include "renderer_device_pixels_internal.hpp"

namespace cgpui {
void Dx12Frame::upload_image(const ImageAsset& image) {
  Dx12OperationGuard operation(*state_);
  if (released_ || error_) return;
  images_.upload(image);
}
void Dx12Frame::invalidate_image(ImageAssetId id) {
  Dx12OperationGuard operation(*state_);
  if (released_ || error_) return;
  images_.invalidate(id);
}
void Dx12Frame::draw_image(const ImageDraw& image) {
  Dx12OperationGuard operation(*state_);
  if (released_ || error_) return;
  draws_.push_back({Dx12PendingImageDraw{renderer_to_device_pixels(image, state_->scale)},
      RendererPrimitiveKind::image, command_count_++});
}
} // namespace cgpui
