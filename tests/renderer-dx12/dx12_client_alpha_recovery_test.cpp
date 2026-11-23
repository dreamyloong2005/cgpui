#include "dx12_client_alpha_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

namespace {
bool client_recovery_peer(Renderer& renderer) {
  auto frame = renderer.begin_frame();
  if (!frame) return false;
  (*frame)->clear({0, 1, 0, 1});
  return (*frame)->request_pixel_capture() && (*frame)->present() &&
      dx12_pixel(renderer.last_frame_pixels(), 8, 8, {0, 255, 0, 255});
}
} // namespace

int main() {
  Dx12RecoveryDeadline deadline;
  OrdinaryDesktop desktop;
  auto app = create_platform_application();
  if (!deadline.done || !desktop.valid() || !app) return dx12_fail(1, "client recovery fixture unavailable");
  auto window = (*app)->create_window(client_alpha_descriptor(desktop), [](const PlatformEvent&) {});
  if (!window || !desktop.place(**window)) return dx12_fail(2, "client recovery window unavailable");
  if (const auto gated = ordinary_alpha_os_gate(**window)) return *gated;
  client_alpha_regions(**window);
  ClientAlphaTopology initial;
  auto renderer = ordinary_renderer(**window, true);
  Dx12Window peer_window;
  auto peer = create_dx12_renderer(peer_window.descriptor());
  if (!renderer || !peer || !initial.capture(**window) || !client_recovery_peer(**peer))
    return dx12_fail(3, "client recovery cohort unavailable");
  auto finished = (*renderer)->begin_frame();
  if (!finished) return dx12_fail(4, "retained client frame unavailable");
  (*finished)->clear({1, 1, 0, 0.5F});
  if (!(*finished)->present() || !client_alpha_paint(**renderer, **window, false))
    return dx12_fail(5, "client recovery warmup failed");
  std::vector<std::unique_ptr<RenderFrame>> retained;
  retained.push_back(std::move(*finished));
  for (unsigned epoch = 0; epoch != 2; ++epoch) {
    const auto before = (*renderer)->backend_info(), peer_before = (*peer)->backend_info();
    if (!before.adapter_luid || before.adapter_luid != peer_before.adapter_luid)
      return dx12_fail(6, "client and opaque peer chose different adapters");
    auto pending = (*renderer)->begin_frame();
    if (!pending) return dx12_fail(7, "pending pre-removal client frame unavailable");
    (*pending)->clear({1, 0, 1, 0.5F});
    const auto* borrowed = (*renderer)->last_frame_pixels();
    if (!borrowed) return dx12_fail(8, "borrowed client capture unavailable");
    const auto saved = borrowed->rgba8;
    if (!dx12_remove_native_device(before)) return dx12_fail(9, "client device removal failed");
    const auto peer_retry = (*peer)->begin_frame();
    if (peer_retry || peer_retry.error().code != ErrorCode::frame_retry_required ||
        (*peer)->backend_info().device_generation != peer_before.device_generation + 1 ||
        (*renderer)->backend_info().device_generation != before.device_generation ||
        (*renderer)->last_frame_pixels() || (*peer)->last_frame_pixels() || borrowed->rgba8 != saved)
      return dx12_fail(10, "client host blocked peer recovery or invalidated retained CPU capture");
    // Native GPU objects are retired now, but the exact live HWND lease must remain occupied.
    if (ordinary_renderer(**window, true)) return dx12_fail(11, "GPU retirement lost the client HWND lease");
    const auto retry = (*pending)->present();
    if (retry || retry.error().code != ErrorCode::frame_retry_required)
      return dx12_fail(12, "old client frame did not rebuild/request retry");
    const auto after = (*renderer)->backend_info();
    if (after.device_generation != before.device_generation + 1 ||
        after.adapter_luid != before.adapter_luid || after.software_adapter != before.software_adapter ||
        (*renderer)->last_frame_pixels()) return dx12_fail(13, "client recovery epoch/adapter/capture differs");
    const auto repeated = (*pending)->present();
    if (repeated || repeated.error().code != ErrorCode::invalid_argument)
      return dx12_fail(14, "old client frame presented twice");
    retained.push_back(std::move(*pending));
    const bool blue = epoch == 0;
    if (!client_recovery_peer(**peer) || !client_alpha_paint(**renderer, **window, blue) ||
        !initial.same(**window) || !client_alpha_hits(**window) ||
        !ordinary_composition((*renderer)->surface_composition(), true) ||
        !dx12_recovered_native_device_clean(after)) return dx12_fail(15, "recovered client pixels/chrome/debug differ");
  }
  auto active = (*renderer)->begin_frame();
  if (!active) return dx12_fail(16, "new active client frame unavailable");
  retained.clear();
  const auto overlap = (*renderer)->begin_frame();
  if (overlap || overlap.error().code != ErrorCode::invalid_argument)
    return dx12_fail(17, "old finished frames unlocked a new active client frame");
  active->reset();
  return client_alpha_paint(**renderer, **window, true) && initial.same(**window) && client_alpha_hits(**window)
      ? 0 : dx12_fail(18, "client failed after old-generation frame destruction");
}
