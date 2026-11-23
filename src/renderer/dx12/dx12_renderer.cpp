#include "dx12_frame_internal.hpp"
#include "dx12_frame_recovery_internal.hpp"
#include "dx12_backdrop_blur_internal.hpp"
#include "cgpui/renderer/renderer_backend.hpp"

namespace cgpui {
namespace {
class Dx12Renderer final : public Renderer {
 public:
  explicit Dx12Renderer(std::shared_ptr<Dx12State> state) : state_(std::move(state)) {}
  ~Dx12Renderer() override { state_->owner_alive = false; }
  Result<void> resize(Size size, DpiScale scale) override {
    return dx12_resize_recovering(*state_, size, scale);
  }
  Result<std::unique_ptr<RenderFrame>> begin_frame() override {
    if (auto ready = dx12_prepare_frame(*state_); !ready) return std::unexpected(ready.error());
    auto frame = std::make_unique<Dx12Frame>(state_);
    state_->frame_active = true;
    return frame;
  }
  const RendererFramePixels* last_frame_pixels() const override {
    return state_->pixels ? &*state_->pixels : nullptr;
  }
  RendererBackendInfo backend_info() const override { return state_->info; }
  RendererSurfaceComposition surface_composition() const override {
    return {.supported = true, .transparent_requested = false,
            .composite_alpha = RendererCompositeAlpha::opaque};
  }
  bool popup_windows_supported() const override { return false; }
  RendererEffectSupport backdrop_blur_support() const override {
    return state_->backdrop_blur->support;
  }
  Result<RendererSurfaceComposition> set_transparent_surface(bool transparent) override {
    if (transparent) return std::unexpected(dx12_unsupported("transparent surfaces"));
    return surface_composition();
  }
 private:
  std::shared_ptr<Dx12State> state_;
};
}

Result<std::unique_ptr<Renderer>> create_dx12_renderer(const RenderSurfaceDescriptor& descriptor) {
  const auto* native = std::get_if<Win32SurfaceHandle>(&descriptor.native_surface);
  if (!native || !native->hwnd || !IsWindow(static_cast<HWND>(native->hwnd))) {
    return std::unexpected(Error{.code = ErrorCode::invalid_argument,
        .message = "DX12 requires a live Win32 HWND"});
  }
  if (native->composition || descriptor.transparent_background) {
    return std::unexpected(dx12_unsupported("transparent/composition presentation"));
  }
  if (auto valid = dx12_validate_extent(descriptor.framebuffer_size, descriptor.scale); !valid) {
    return std::unexpected(valid.error());
  }
  auto state = std::make_shared<Dx12State>();
  state->hwnd = static_cast<HWND>(native->hwnd);
  state->width = static_cast<UINT>(descriptor.framebuffer_size.width);
  state->height = static_cast<UINT>(descriptor.framebuffer_size.height);
  state->scale = descriptor.scale;
  if (auto device = dx12_create_device(*state); !device) return std::unexpected(device.error());
  state->backdrop_blur->support = dx12_backdrop_blur_support(*state->device.Get());
  if (auto pipeline = dx12_create_pipeline(*state); !pipeline) return std::unexpected(pipeline.error());
  if (auto surface = dx12_create_swapchain(*state); !surface) return std::unexpected(surface.error());
  if (auto debug = dx12_check_debug_errors(*state); !debug) return std::unexpected(debug.error());
  return std::make_unique<Dx12Renderer>(std::move(state));
}
} // namespace cgpui
