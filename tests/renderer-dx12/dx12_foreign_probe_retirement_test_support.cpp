#include "dx12_foreign_probe_native_test_support.hpp"
#include "dx12_recovery_test_support.hpp"
#include "../../src/renderer/dx12/dx12_operation_internal.hpp"
#include "../../src/renderer/dx12/dx12_native_retirement_internal.hpp"
#include "../../src/renderer/dx12/dx12_geometry_upload_internal.hpp"
#include "../../src/renderer/dx12/dx12_texture_upload_internal.hpp"
#include "../../src/renderer/dx12/dx12_texture_renderer_internal.hpp"
#include <mutex>

namespace cgpui_test {
bool foreign_probe_preserved(const cgpui::Dx12State& peer, const ForeignProbeSnapshot& before) {
  return peer.pixels && &*peer.pixels == before.pixels && peer.pixels->rgba8.data() == before.data &&
      peer.pixels->rgba8 == before.bytes && peer.info.adapter_luid == before.info.adapter_luid &&
      peer.info.device_generation == before.info.device_generation && peer.hwnd == before.hwnd &&
      peer.width == before.width && peer.height == before.height && peer.scale.value == before.scale &&
      peer.image_cache.get() == before.image_cache && peer.text_cache.get() == before.text_cache &&
      !peer.frame_active && peer.owner_alive && !peer.window_alpha && !peer.composition_surface;
}
int foreign_probe_retire(cgpui::Dx12State& peer, Dx12PacingForwardApi& pacing,
    const ForeignProbeSnapshot& before, ForeignProbeMode mode) {
  using namespace cgpui;
  std::unique_lock gate(peer.operation_mutex, std::try_to_lock);
  if (!gate.owns_lock() || peer.operation_depth != 0 || !peer.device ||
      peer.window_thread_id == GetCurrentThreadId() || peer.submitted || peer.frame_active) return 41;
  Dx12OperationGuard operation(peer);
  const HRESULT reason = peer.device->GetDeviceRemovedReason();
  if (SUCCEEDED(reason) || !foreign_probe_preserved(peer, before)) return 42;
  {
    Dx12Ptr<ID3D12InfoQueue> messages;
    if (FAILED(peer.device.As(&messages)) || !messages ||
        !dx12_recovery_debug_messages_safe(messages.Get(), true)) return 43;
  } // No old InfoQueue/native device reference escapes into purge/reconstruction.
  peer.removal_reason = reason;
  peer.recovery_pending = peer.failed = true;
  foreign_probe_log(foreign_probe_mode_name(mode), "native-purge-enter", peer.window_thread_id,
      *before.info.adapter_luid, before.info.device_generation, static_cast<int>(reason));
  dx12_release_native_objects(peer); // Experiment bypasses enumeration, not this native owner.
  foreign_probe_log(foreign_probe_mode_name(mode), "native-purge-return", peer.window_thread_id,
      *before.info.adapter_luid, before.info.device_generation);
  if (peer.device || peer.adapter || peer.factory || peer.queue || peer.fence || peer.swapchain ||
      peer.commands || peer.allocator || peer.rtv_heap || peer.readback || peer.fence_event.value ||
      peer.targets[0] || peer.targets[1] || peer.submitted || !peer.recovery_pending || !peer.failed ||
      peer.geometry_uploads->stats().page_count || peer.texture_uploads->stats().page_count ||
      !peer.texture_renderer->native_textures.empty() || pacing.closes != before.closes + 1 ||
      !pacing.close_ok || !foreign_probe_preserved(peer, before)) return 44;
  return 0;
}
} // namespace cgpui_test
