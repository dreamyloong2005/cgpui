#include "../support/frame_wait_test_support.hpp"
#include "dx12_recovery_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12RecoveryDeadline deadline;
  Dx12Window live_window, orphan_window;
  if (!deadline.done || !live_window.hwnd || !orphan_window.hwnd)
    return dx12_fail(1, "orphan cohort fixture unavailable");
  auto live = create_dx12_renderer(live_window.descriptor());
  auto dead = create_dx12_renderer(orphan_window.descriptor());
  if (!live || !dead) return dx12_fail(2, "orphan cohort renderers unavailable");
  const auto before = (*live)->backend_info();
  if (before.adapter_luid != (*dead)->backend_info().adapter_luid)
    return dx12_fail(3, "orphan cohort adapters differ");
  const ImageAsset image{.id = {8502}, .logical_size = {1, 1},
      .bitmap = {.width = 1, .height = 1, .stride = 4, .pixels = {255, 0, 0, 255}}};
  auto finished = cgpui_test::wait_for_test_frame(**dead);
  if (!finished) return dx12_fail(4, "orphan warmup begin failed");
  (*finished)->upload_image(image);
  (*finished)->draw_image({.bounds = {{4, 4}, {24, 24}}, .asset = describe_image_asset(image)});
  if (!(*finished)->request_pixel_capture() || !(*finished)->present() ||
      !dx12_pixel((*dead)->last_frame_pixels(), 8, 8, {255, 0, 0, 255}))
    return dx12_fail(5, "orphan warmup failed");
  auto orphan = cgpui_test::wait_for_test_frame(**dead);
  if (!orphan) return dx12_fail(6, "orphan CPU frame begin failed");
  (*orphan)->draw_image({.bounds = {{4, 4}, {24, 24}}, .asset = describe_image_asset(image)});
  dead->reset(); // Both retained frames now outlive the public owner.
  if (!DestroyWindow(orphan_window.hwnd)) return dx12_fail(7, "orphan HWND destroy failed");
  orphan_window.hwnd = nullptr; // Any attempted orphan rebuild would lack a live surface.
  if (!dx12_remove_native_device(before)) return dx12_fail(8, "orphan cohort native removal failed");
  const auto retry = (*live)->begin_frame();
  if (retry || retry.error().code != ErrorCode::frame_retry_required)
    return dx12_fail(9, "retained orphan native resources blocked live cohort recovery");
  const auto after = (*live)->backend_info();
  if (after.device_generation != before.device_generation + 1 || after.adapter_luid != before.adapter_luid)
    return dx12_fail(10, "live generation differs after orphan purge");
  (*orphan)->draw_rect({.rect = {{0, 0}, {4, 4}}, .color = {0, 1, 0, 1}});
  const auto presented = (*orphan)->present();
  if (presented || presented.error().code != ErrorCode::invalid_argument)
    return dx12_fail(11, "orphan frame was recovered or presented without its owner");
  auto frame = cgpui_test::wait_for_test_frame(**live);
  if (!frame) return dx12_fail(12, "live cohort cannot begin after orphan rejection");
  finished->reset();
  orphan->reset();
  const auto overlap = (*live)->begin_frame();
  if (overlap || overlap.error().code != ErrorCode::invalid_argument)
    return dx12_fail(13, "orphan destruction unlocked unrelated live frame");
  (*frame)->clear({0, 1, 0, 1});
  if (!(*frame)->request_pixel_capture() || !(*frame)->present() ||
      !dx12_pixel((*live)->last_frame_pixels(), 8, 8, {0, 255, 0, 255}))
    return dx12_fail(14, "live pixels failed after orphan destruction");
  return dx12_recovered_native_device_clean(after) ? 0 : dx12_fail(15, "orphan cohort debug errors");
}
