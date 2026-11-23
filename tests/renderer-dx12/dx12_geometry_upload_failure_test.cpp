#include "dx12_geometry_upload_test_support.hpp"
#include "dx12_sync_retirement_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12ControlledRetirement script;
  script.fail_signal_until_completion = true;
  std::atomic_bool released = false;
  auto state = geometry_state();
  if (!state || !dx12_begin_geometry_upload_frame(*state))
    return dx12_fail(1, "geometry failure fixture unavailable");
  const std::array<unsigned char, 32> bytes{0xf1, 0xe2, 0xd3, 0xc4};
  auto slice = dx12_upload_geometry(*state, bytes.data(), bytes.size());
  if (!slice || !geometry_sentinel(*slice->resource, released))
    return dx12_fail(2, "arena native lifetime sentinel unavailable");
  const auto before = geometry_stats(*state->geometry_uploads);
  for (unsigned fault = 0; fault != 4; ++fault) {
    state->submitted = fault == 0; // Synthetic pending boundary; no native GPU work.
    state->failed = fault == 1;
    state->recovery_pending = fault == 2;
    state->removal_reason = fault == 3 ? DXGI_ERROR_DEVICE_REMOVED : S_OK;
    const auto begin = dx12_begin_geometry_upload_frame(*state);
    const auto upload = dx12_upload_geometry(*state, bytes.data(), bytes.size());
    state->submitted = state->failed = state->recovery_pending = false;
    state->removal_reason = S_OK;
    if (begin || upload || begin.error().code != ErrorCode::invalid_argument ||
        upload.error().code != ErrorCode::invalid_argument || released.load() ||
        geometry_stats(*state->geometry_uploads) != before || !geometry_bytes(*slice, bytes))
      return dx12_fail(3, "unsafe frame/upload boundary rewound or overwrote retained arena data");
  }
  Dx12State missing_device;
  if (dx12_begin_geometry_upload_frame(missing_device) ||
      dx12_upload_geometry(missing_device, bytes.data(), bytes.size()))
    return dx12_fail(4, "geometry frame wrapper accepted a missing native device");
  state->synchronization_api = &script;
  state->submitted = true;
  if (dx12_wait_idle(*state)) {
    state->submitted = false;
    return dx12_fail(5, "scripted failed completion was hidden");
  }
  const bool rejected = !dx12_begin_geometry_upload_frame(*state) &&
      geometry_stats(*state->geometry_uploads) == before && !released.load() && geometry_bytes(*slice, bytes);
  if (!rejected) {
    state->submitted = false; // No ExecuteCommandLists in this boundary test.
    return dx12_fail(6, "failed idle wait allowed arena rewind or release");
  }
  if (!dx12_retirement_retains_until_complete(std::move(state), script, released))
    return dx12_fail(7, "arena native page retired before completion proof or leaked afterward");
  return script.signal_calls >= 3 ? 0 : dx12_fail(8, "arena retirement did not retry completion");
}
