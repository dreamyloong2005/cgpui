#include "dx12_geometry_upload_test_support.hpp"
#include "dx12_recovery_test_support.hpp"
#include "dx12_diagnostics_support.hpp"
#include "../../src/renderer/dx12/dx12_frame_internal.hpp"
#include "../../src/renderer/dx12/dx12_frame_recovery_internal.hpp"
#include "../../src/renderer/dx12/dx12_recovery_internal.hpp"
#include "../../src/renderer/dx12/dx12_surface_internal.hpp"

using namespace cgpui;
using namespace cgpui_test;

namespace {
bool render(const std::shared_ptr<Dx12State>& state) {
  if (!dx12_prepare_frame(*state)) return false;
  state->frame_active = true;
  Dx12Frame frame(state);
  frame.clear({0, 0, 1, 1});
  frame.draw_rect({.rect = {{4, 4}, {24, 24}}, .color = {1, 0, 0, 1}});
  return frame.request_pixel_capture() && frame.present() && state->pixels &&
      dx12_pixel(&*state->pixels, 8, 8, {255, 0, 0, 255}) &&
      dx12_pixel(&*state->pixels, 40, 40, {0, 0, 255, 255}) &&
      diagnostic_work(state->diagnostics ? &*state->diagnostics : nullptr, 1, 1, 1) &&
      !state->diagnostics->upload_bytes.total_byte_count;
}
} // namespace

int main() {
  Dx12RecoveryDeadline deadline;
  Dx12Window window;
  std::atomic_bool released = false;
  auto state = std::make_shared<Dx12State>();
  if (!deadline.done || !window.hwnd || !dx12_configure_surface(*state, window.descriptor()) ||
      !dx12_create_device(*state) || !dx12_create_pipeline(*state) || !dx12_create_swapchain(*state) || !render(state))
    return dx12_fail(1, "arena recovery native rendering fixture failed");
  const std::array<unsigned char, 16> marker{0x8f, 0x7e, 0x6d, 0x5c};
  auto old = dx12_upload_geometry(*state, marker.data(), marker.size());
  if (!old || !geometry_sentinel(*old->resource, released))
    return dx12_fail(2, "arena recovery native page sentinel unavailable");
  const auto before = state->info;
  const auto allocation_count = state->geometry_uploads->stats().total_allocations;
  for (const float width : {96.0F, 64.0F}) {
    if (!dx12_resize_recovering(*state, {width, 64}, {1}) || !render(state) || released.load() ||
        state->geometry_uploads->stats().allocations_this_frame ||
        state->geometry_uploads->stats().total_allocations != allocation_count)
      return dx12_fail(6, "same-device resize released or reallocated the reusable geometry page");
  }
  const auto saved = *state->diagnostics;
  if (!dx12_remove_native_device(before)) return dx12_fail(3, "arena recovery real device removal failed");
  const auto recovered = dx12_recover_removed_device(*state);
  if (!recovered || !*recovered || !released.load() ||
      state->geometry_uploads->stats().page_count || state->geometry_uploads->stats().used_bytes ||
      state->geometry_uploads->stats().total_allocations != allocation_count ||
      state->info.device_generation != before.device_generation + 1 ||
      !diagnostic_same(&*state->diagnostics, saved))
    return dx12_fail(4, "recovery retained removed-device arena pages or lost diagnostic history");
  if (!render(state) || state->geometry_uploads->stats().allocations_this_frame != 1 ||
      state->geometry_uploads->stats().total_allocations != allocation_count + 1 ||
      !render(state) || state->geometry_uploads->stats().allocations_this_frame ||
      !dx12_recovered_native_device_clean(state->info))
    return dx12_fail(5, "recovered geometry did not allocate fresh storage then reuse it with correct pixels");
  return 0;
}
