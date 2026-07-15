#include "metal_renderer_internal.hpp"

#include <cmath>

namespace cgpui {
namespace {

bool valid_surface_metrics(Size size, DpiScale scale) {
  return std::isfinite(size.width) && std::isfinite(size.height) &&
         std::isfinite(scale.value) && size.width > 0.0F &&
         size.height > 0.0F && scale.value > 0.0F;
}

}  // namespace

Result<void> configure_metal_surface(
    MetalRendererState& state,
    Size framebuffer_size,
    DpiScale scale,
    bool transparent_background) {
  if (!valid_surface_metrics(framebuffer_size, scale)) {
    return std::unexpected(Error{
        .code = ErrorCode::renderer_initialization_failed,
        .message = "Metal renderer requires finite non-empty surface metrics"});
  }
  if (state.layer == nil || state.device == nil) {
    return std::unexpected(Error{
        .code = ErrorCode::renderer_initialization_failed,
        .message = "Metal renderer requires a CAMetalLayer and MTLDevice"});
  }

  state.framebuffer_size = framebuffer_size;
  state.scale = scale;
  state.layer.device = state.device;
  state.layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
  state.layer.framebufferOnly = YES;
  state.layer.opaque = transparent_background ? NO : YES;
  state.layer.maximumDrawableCount = 3;
  state.layer.allowsNextDrawableTimeout = YES;
  state.layer.displaySyncEnabled = YES;
  state.layer.contentsScale = scale.value;
  state.layer.drawableSize = CGSizeMake(
      framebuffer_size.width, framebuffer_size.height);
  return {};
}

Result<void> MetalRendererState::resize(
    Size framebuffer_size,
    DpiScale scale) {
  return configure_metal_surface(
      *this, framebuffer_size, scale, !layer.opaque);
}

}  // namespace cgpui
