#include "metal_renderer_internal.hpp"

namespace cgpui {
namespace {

class MetalRenderer final : public Renderer {
 public:
  explicit MetalRenderer(std::shared_ptr<MetalRendererState> state)
      : state_(std::move(state)) {}

  Result<void> resize(Size framebuffer_size, DpiScale scale) override {
    return state_->resize(framebuffer_size, scale);
  }

  Result<std::unique_ptr<RenderFrame>> begin_frame() override {
    return make_metal_frame(state_);
  }

  const RendererFrameDiagnosticSnapshot*
  last_frame_diagnostic_snapshot() const override {
    return state_->snapshot();
  }

 private:
  std::shared_ptr<MetalRendererState> state_;
};

}  // namespace

Result<std::shared_ptr<MetalRendererState>> create_metal_renderer_state(
    const RenderSurfaceDescriptor& descriptor) {
  const auto* surface =
      std::get_if<MetalSurfaceHandle>(&descriptor.native_surface);
  if (surface == nullptr) {
    return std::unexpected(Error{
        .code = ErrorCode::renderer_initialization_failed,
        .message = "Metal renderer requires a Metal native surface"});
  }
  if (surface->layer == nullptr) {
    return std::unexpected(Error{
        .code = ErrorCode::renderer_initialization_failed,
        .message = "Metal renderer requires a non-null CAMetalLayer"});
  }

  auto state = std::make_shared<MetalRendererState>();
  state->layer = (__bridge CAMetalLayer*)surface->layer;
  state->device = MTLCreateSystemDefaultDevice();
  if (state->device == nil) {
    return std::unexpected(Error{
        .code = ErrorCode::renderer_initialization_failed,
        .message = "MTLCreateSystemDefaultDevice failed"});
  }
  state->command_queue = [state->device newCommandQueue];
  if (state->command_queue == nil) {
    return std::unexpected(Error{
        .code = ErrorCode::renderer_initialization_failed,
        .message = "Metal command queue creation failed"});
  }
  if (auto configured = configure_metal_surface(
          *state,
          descriptor.framebuffer_size,
          descriptor.scale,
          descriptor.transparent_background);
      !configured) {
    return std::unexpected(configured.error());
  }
  return state;
}

std::unique_ptr<Renderer> make_metal_renderer(
    std::shared_ptr<MetalRendererState> state) {
  return std::make_unique<MetalRenderer>(std::move(state));
}

}  // namespace cgpui
