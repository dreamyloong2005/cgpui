#include "dx12_texture_upload_test_support.hpp"
#include "dx12_sync_retirement_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12ControlledRetirement script;
  script.fail_signal_until_completion = true;
  std::atomic_bool released = false;
  auto state = std::make_unique<Dx12State>();
  if (!dx12_create_device(*state) || !dx12_begin_texture_upload_frame(*state))
    return dx12_fail(1, "texture failure native fixture unavailable");
  const auto texture = staging_texture(3, 2, DXGI_FORMAT_R8G8B8A8_UNORM, 4);
  auto slice = dx12_upload_texture(*state, texture);
  if (!slice || !dx12_attach_resource_sentinel(*slice->resource, released))
    return dx12_fail(2, "texture native page sentinel unavailable");
  const auto before = texture_upload_stats(*state->texture_uploads);
  for (unsigned fault = 0; fault != 4; ++fault) {
    state->submitted = fault == 0; // Synthetic pending: no ExecuteCommandLists here.
    state->failed = fault == 1;
    state->recovery_pending = fault == 2;
    state->removal_reason = fault == 3 ? DXGI_ERROR_DEVICE_REMOVED : S_OK;
    const auto begin = dx12_begin_texture_upload_frame(*state);
    const auto uploaded = dx12_upload_texture(*state, texture);
    state->submitted = state->failed = state->recovery_pending = false;
    state->removal_reason = S_OK;
    if (begin || uploaded || begin.error().code != ErrorCode::invalid_argument ||
        uploaded.error().code != ErrorCode::invalid_argument || released.load() ||
        texture_upload_stats(*state->texture_uploads) != before || !staging_bytes(*slice, texture))
      return dx12_fail(3, "unsafe upload boundary rewound or rewrote the retained texture page");
  }
  Dx12State missing;
  if (dx12_begin_texture_upload_frame(missing) || dx12_upload_texture(missing, texture))
    return dx12_fail(4, "texture wrapper accepted a missing native device");
  state->synchronization_api = &script;
  state->submitted = true;
  if (dx12_wait_idle(*state)) {
    state->submitted = false;
    return dx12_fail(5, "scripted texture completion failure was hidden");
  }
  const bool retained = !dx12_begin_texture_upload_frame(*state) && !released.load() &&
      texture_upload_stats(*state->texture_uploads) == before && staging_bytes(*slice, texture);
  if (!retained) {
    state->submitted = false; // Test never submitted native commands.
    return dx12_fail(6, "failed wait released or rewound an unconfirmed staging page");
  }
  return dx12_retirement_retains_until_complete(std::move(state), script, released)
      ? 0 : dx12_fail(7, "texture staging retirement ignored completion proof or leaked");
}
