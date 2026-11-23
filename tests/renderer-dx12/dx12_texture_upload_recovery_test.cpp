#include "dx12_texture_upload_test_support.hpp"
#include "dx12_recovery_test_support.hpp"
#include "../../src/renderer/dx12/dx12_frame_recovery_internal.hpp"
#include "../../src/renderer/dx12/dx12_recovery_internal.hpp"

using namespace cgpui;
using namespace cgpui_test;

namespace {
bool render(const std::shared_ptr<Dx12State>& state, bool upload) {
  if (!dx12_prepare_frame(*state)) return false;
  state->frame_active = true;
  Dx12Frame frame(state);
  frame.clear({0, 0, 0, 1});
  const auto image = diagnostic_image();
  if (upload) frame.upload_image(image);
  frame.draw_image({.bounds = {{0, 0}, {16, 16}}, .asset = describe_image_asset(image), .sampling = ImageSamplingMode::nearest});
  frame.draw_text(diagnostic_text("M", 16, {32, 4}));
  return frame.request_pixel_capture() && frame.present() && state->pixels &&
      dx12_pixel(&*state->pixels, 4, 4, {255, 0, 0, 255}) &&
      dx12_pixel(&*state->pixels, 12, 12, {255, 255, 255, 255}) &&
      diagnostic_ink(&*state->pixels, 32, 0, 64, 32) &&
      diagnostic_work(state->diagnostics ? &*state->diagnostics : nullptr, 2, 2, 2);
}
} // namespace

int main() {
  Dx12RecoveryDeadline deadline;
  Dx12Window window;
  std::atomic_bool released = false;
  auto state = texture_upload_state(&window);
  if (!deadline.done || !state || !render(state, true)) return dx12_fail(1, "texture recovery mixed GPU fixture failed");
  const auto original_bytes = state->diagnostics->upload_bytes;
  if (original_bytes.image_byte_count != 16 || !original_bytes.glyph_atlas_byte_count)
    return dx12_fail(2, "mixed texture recovery fixture did not upload image and glyph payloads");
  const auto marker = staging_texture(3, 2, DXGI_FORMAT_R8G8B8A8_UNORM, 4);
  const auto slice = dx12_upload_texture(*state, marker);
  if (!slice || !dx12_attach_resource_sentinel(*slice->resource, released))
    return dx12_fail(3, "texture recovery native staging sentinel unavailable");
  const auto before = state->info;
  const auto allocations = state->texture_uploads->stats().total_allocations;
  for (const float width : {96.0F, 64.0F}) {
    if (!dx12_resize_recovering(*state, {width, 64}, {1}) || !render(state, false) || released.load() ||
        state->texture_uploads->stats().allocations_this_frame || state->texture_uploads->stats().used_bytes ||
        state->diagnostics->upload_bytes.total_byte_count)
      return dx12_fail(4, "same-device resize discarded staging storage or reuploaded cached textures");
  }
  const auto saved = *state->diagnostics;
  if (!dx12_remove_native_device(before)) return dx12_fail(5, "texture recovery real removal failed");
  const auto recovered = dx12_recover_removed_device(*state);
  if (!recovered || !*recovered || !released.load() || state->texture_uploads->stats().page_count ||
      state->texture_uploads->stats().total_allocations != allocations ||
      state->info.device_generation != before.device_generation + 1 || !diagnostic_same(&*state->diagnostics, saved))
    return dx12_fail(6, "native recovery retained old staging storage or lost diagnostic history");
  if (!render(state, false) || diagnostic_uploads(state->diagnostics->upload_bytes) != diagnostic_uploads(original_bytes) ||
      state->texture_uploads->stats().allocations_this_frame != 1 ||
      state->texture_uploads->stats().total_allocations != allocations + 1 || !render(state, false) ||
      state->texture_uploads->stats().used_bytes || state->texture_uploads->stats().allocations_this_frame ||
      state->diagnostics->upload_bytes.total_byte_count || !dx12_recovered_native_device_clean(state->info))
    return dx12_fail(7, "recovery did not reupload fresh image/glyph storage then reuse it correctly");
  return 0;
}
