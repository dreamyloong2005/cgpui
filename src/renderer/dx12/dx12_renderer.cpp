#include "dx12_operation_internal.hpp"
#include "dx12_frame_internal.hpp"
#include "dx12_frame_recovery_internal.hpp"
#include "dx12_frame_admission_internal.hpp"
#include "dx12_backdrop_blur_internal.hpp"
#include "dx12_device_cohort_internal.hpp"
#include "dx12_surface_internal.hpp"
#include "dx12_window_alpha_internal.hpp"
#include "dx12_renderer_creation_internal.hpp"
#include "dx12_frame_pacing_native_internal.hpp"
#include "cgpui/renderer/renderer_backend.hpp"

namespace cgpui {
namespace {
class Dx12Renderer final : public Renderer {
 public:
  explicit Dx12Renderer(std::shared_ptr<Dx12State> state) : state_(std::move(state)) {}
  ~Dx12Renderer() override {
    Dx12OperationGuard operation(*state_);
    state_->owner_alive = false;
  }
  Result<void> resize(Size size, DpiScale scale) override {
    Dx12OperationGuard operation(*state_);
    if (auto valid = dx12_validate_window_alpha(*state_); !valid) return valid;
    return dx12_resize_recovering(*state_, size, scale);
  }
  Result<std::unique_ptr<RenderFrame>> begin_frame() override {
    Dx12OperationGuard operation(*state_);
    if (auto ready = dx12_prepare_frame(*state_); !ready) return std::unexpected(ready.error());
    auto admitted = dx12_admit_frame(*state_);
    if (!admitted) return std::unexpected(admitted.error());
    auto frame = std::make_unique<Dx12Frame>(state_, admitted->admission_call_ns);
    state_->frame_active = true;
    return frame;
  }
  const RendererFramePixels* last_frame_pixels() const override {
    Dx12OperationGuard operation(*state_);
    return !state_->recovery_pending && state_->pixels ? &*state_->pixels : nullptr;
  }
  const RendererFrameDiagnosticSnapshot* last_frame_diagnostic_snapshot() const override {
    Dx12OperationGuard operation(*state_);
    return state_->diagnostics ? &*state_->diagnostics : nullptr;
  }
  RendererBackendInfo backend_info() const override {
    Dx12OperationGuard operation(*state_);
    return state_->info;
  }
  RendererSurfaceComposition surface_composition() const override {
    Dx12OperationGuard operation(*state_);
    return dx12_surface_composition(*state_);
  }
  bool popup_windows_supported() const override { return true; }
  RendererEffectSupport backdrop_blur_support() const override {
    Dx12OperationGuard operation(*state_);
    return state_->backdrop_blur->support;
  }
  Result<RendererSurfaceComposition> set_transparent_surface(bool transparent) override {
    Dx12OperationGuard operation(*state_);
    return dx12_set_transparent_surface(*state_, transparent);
  }
 private:
  std::shared_ptr<Dx12State> state_;
};
}

Result<std::unique_ptr<Renderer>> create_dx12_renderer(const RenderSurfaceDescriptor& descriptor) {
  return dx12_create_renderer(descriptor, dx12_native_frame_pacing_api());
}

Result<std::unique_ptr<Renderer>> dx12_create_renderer(
    const RenderSurfaceDescriptor& descriptor, Dx12FramePacingApi& pacing_api,
    Dx12SynchronizationApi* synchronization_api) {
  auto state = std::make_shared<Dx12State>(&pacing_api, synchronization_api);
  Dx12OperationGuard operation(*state);
  if (auto surface = dx12_configure_surface(*state, descriptor); !surface)
    return std::unexpected(surface.error());
  if (auto device = dx12_create_device(*state); !device) return std::unexpected(device.error());
  state->backdrop_blur->support = dx12_backdrop_blur_support(*state->device.Get());
  if (auto pipeline = dx12_create_pipeline(*state); !pipeline) return std::unexpected(pipeline.error());
  if (auto history = dx12_mark_opaque_window(*state); !history) return std::unexpected(history.error());
  if (auto surface = dx12_create_swapchain(*state); !surface) return std::unexpected(surface.error());
  if (auto debug = dx12_check_debug_errors(*state); !debug) return std::unexpected(debug.error());
  dx12_register_device_cohort(state);
  return std::make_unique<Dx12Renderer>(std::move(state));
}
} // namespace cgpui
