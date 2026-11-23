#include "dx12_texture_upload_test_support.hpp"
#include "../../src/renderer/dx12/dx12_texture_renderer_internal.hpp"
#include "../../src/renderer/dx12/dx12_image_cache_internal.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12Window window;
  auto state = texture_upload_state(&window);
  if (!state) return dx12_fail(1, "unsubmitted texture fixture unavailable");
  const auto image = diagnostic_image();
  const auto asset = describe_image_asset(image);
  if (!state->image_cache->upload(*state, image)) return dx12_fail(2, "unsubmitted texture CPU snapshot failed");
  const auto snapshot = state->image_cache->find(asset);
  if (!snapshot || !dx12_wait_idle(*state) || !dx12_begin_texture_upload_frame(*state))
    return dx12_fail(3, "unsubmitted texture frame preparation failed");
  dx12_begin_texture_frame(*state);
  if (FAILED(state->allocator->Reset()) || FAILED(state->commands->Reset(state->allocator.Get(), nullptr)))
    return dx12_fail(4, "unsubmitted texture native list reset failed");
  const auto prepared = dx12_prepare_texture(*state, *snapshot);
  if (!prepared || (*prepared)->uploaded || state->texture_renderer->pending_uploads.size() != 1)
    return dx12_fail(5, "recorded texture was prematurely committed");
  const auto recorded = texture_upload_stats(*state->texture_uploads);
  const auto duplicate = dx12_prepare_texture(*state, *snapshot);
  if (!duplicate || *duplicate != *prepared || texture_upload_stats(*state->texture_uploads) != recorded ||
      state->texture_renderer->pending_uploads.size() != 1 || FAILED(state->commands->Close()))
    return dx12_fail(6, "duplicate recording uploaded the same snapshot twice");
  // Intentionally discard this real native CopyTextureRegion/barrier list.
  // No Execute means the destination is still COPY_DEST, not shader-readable.
  if (state->submitted || state->diagnostics) return dx12_fail(7, "discarded list published frame work");
  for (unsigned pass = 0; pass != 2; ++pass) {
    state->frame_active = true;
    Dx12Frame frame(state);
    frame.clear({0, 0, 0, 1});
    frame.draw_image({.bounds = {{0, 0}, {16, 16}}, .asset = asset, .sampling = ImageSamplingMode::nearest});
    if (!frame.request_pixel_capture() || !frame.present() || !state->pixels ||
        !dx12_pixel(&*state->pixels, 4, 4, {255, 0, 0, 255}) ||
        !dx12_pixel(&*state->pixels, 12, 12, {255, 255, 255, 255}))
      return dx12_fail(8, "discarded native upload poisoned the next real GPU frame");
    if (!diagnostic_work(&*state->diagnostics, 1, 1, 1) ||
        state->diagnostics->upload_bytes.image_byte_count != (pass == 0 ? 16U : 0U) ||
        !(*prepared)->uploaded || !state->texture_renderer->pending_uploads.empty() ||
        state->texture_uploads->stats().allocations_this_frame ||
        (pass == 0 ? !state->texture_uploads->stats().used_bytes : state->texture_uploads->stats().used_bytes != 0))
      return dx12_fail(9, "unsubmitted upload was skipped, duplicated, or reset after completed submission");
  }
  return dx12_check_debug_errors(*state) ? 0 : dx12_fail(10, "unsubmitted texture resource-state validation failed");
}
