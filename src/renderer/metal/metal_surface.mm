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

// CAMetalLayer composites premultiplied alpha when it is not opaque; the
// pipelines blend to premultiplied output (metal_pipeline_library.mm).
// supported_composite_alpha is a Vulkan mask and stays 0 here.
RendererSurfaceComposition metal_surface_composition(
    const MetalRendererState& state) {
  const bool transparent = state.layer != nil && !state.layer.opaque;
  return RendererSurfaceComposition{
      .supported = state.layer != nil,
      .transparent_requested = transparent,
      .composite_alpha = transparent ? RendererCompositeAlpha::pre_multiplied
                                     : RendererCompositeAlpha::opaque};
}

Result<RendererSurfaceComposition> metal_set_transparent_surface(
    MetalRendererState& state,
    bool transparent) {
  if (state.layer == nil) {
    return std::unexpected(Error{
        .code = ErrorCode::renderer_initialization_failed,
        .message = "Metal renderer has no CAMetalLayer"});
  }
  state.layer.opaque = transparent ? NO : YES;
  return metal_surface_composition(state);
}

Result<void> MetalRendererState::resize(
    Size framebuffer_size,
    DpiScale scale) {
  return configure_metal_surface(
      *this, framebuffer_size, scale, !layer.opaque);
}

}  // namespace cgpui
