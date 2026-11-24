#include "dx12_foreign_probe_native_test_support.hpp"
#include "../../src/renderer/dx12/dx12_operation_internal.hpp"
#include "../../src/renderer/dx12/dx12_surface_internal.hpp"
#include "../../src/renderer/dx12/dx12_window_alpha_internal.hpp"
#include "../../src/renderer/dx12/dx12_backdrop_blur_internal.hpp"
#include "../../src/renderer/dx12/dx12_device_cohort_internal.hpp"
#include "../../src/renderer/dx12/dx12_frame_recovery_internal.hpp"

namespace cgpui_test {
ForeignProbeFixture::~ForeignProbeFixture() { state.reset(); } // No external gate; HWND/API still live.
bool ForeignProbeFixture::initialize() {
  using namespace cgpui;
  if (!window.hwnd) return false;
  state = std::make_shared<Dx12State>(&pacing);
  Dx12OperationGuard operation(*state);
  auto descriptor = window.descriptor();
  descriptor.transparent_background = false;
  if (!dx12_configure_surface(*state, descriptor) || !dx12_create_device(*state)) return false;
  state->backdrop_blur->support = dx12_backdrop_blur_support(*state->device.Get());
  if (!dx12_create_pipeline(*state) || !dx12_mark_opaque_window(*state) ||
      !dx12_create_swapchain(*state) || !dx12_check_debug_errors(*state)) return false;
  dx12_register_device_cohort(state);
  return state->window_thread_id == GetCurrentThreadId() && !state->composition_surface &&
      !state->transparent_background && !state->window_alpha && state->info.adapter_luid &&
      state->info.backend == RendererBackend::dx12 && state->info.selection == RendererBackendSelection::dx12 &&
      state->info.fallback_reason.empty() && pacing.sets == 1 && pacing.gets == 1 && !pacing.closes;
}
bool ForeignProbeFixture::remember(ForeignProbeSnapshot& snapshot) {
  cgpui::Dx12OperationGuard operation(*state);
  if (!state->pixels || !state->pixels->valid() || state->submitted || state->frame_active) return false;
  snapshot.pixels = &*state->pixels;
  snapshot.data = state->pixels->rgba8.data();
  snapshot.bytes = state->pixels->rgba8;
  snapshot.info = state->info;
  snapshot.hwnd = state->hwnd;
  snapshot.width = state->width; snapshot.height = state->height; snapshot.scale = state->scale.value;
  snapshot.image_cache = state->image_cache.get(); snapshot.text_cache = state->text_cache.get();
  snapshot.closes = pacing.closes;
  return true;
}
bool ForeignProbeFixture::recover_once(const ForeignProbeSnapshot& before) {
  using namespace cgpui;
  Dx12OperationGuard operation(*state);
  const auto ready = dx12_prepare_frame(*state);
  return !ready && ready.error().code == ErrorCode::frame_retry_required && state->device &&
      state->swapchain && !state->failed && !state->recovery_pending && !state->pixels &&
      state->info.adapter_luid == before.info.adapter_luid &&
      state->info.device_generation == before.info.device_generation + 1;
}
bool foreign_probe_filter_excludes(cgpui::Dx12State& origin,
    const std::shared_ptr<cgpui::Dx12State>& peer) {
  std::vector<std::weak_ptr<cgpui::Dx12State>> candidates;
  {
    cgpui::Dx12OperationGuard operation(origin);
    candidates = cgpui::dx12_device_cohort_peers(origin);
  }
  for (const auto& weak : candidates) {
    auto pin = weak.lock(); // Promotion and final release are outside every state gate.
    if (pin == peer) return false;
  }
  return true;
}
} // namespace cgpui_test
