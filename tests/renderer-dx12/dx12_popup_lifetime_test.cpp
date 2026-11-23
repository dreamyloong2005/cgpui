#include "../support/frame_wait_test_support.hpp"
#include "dx12_popup_test_support.hpp"
#include "dx12_pacing_begin_test_support.hpp"
#include "../../src/renderer/dx12/dx12_renderer_creation_internal.hpp"

#include <string>

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12RecoveryDeadline deadline;
  if (!deadline.done || !dx12_popup_desktop_ready())
    return dx12_fail(1, "popup lifetime desktop/DPI/watchdog unavailable");
  CompositionTestWindows windows(48, 48);
  if (!windows.valid() || !windows.expect({0, 255, 0}, windows.backdrop_point()))
    return dx12_fail(2, "popup lifetime visible backdrop unavailable");
  const RenderSurfaceDescriptor descriptor{
      .native_surface = Win32SurfaceHandle{.hinstance = GetModuleHandleW(nullptr),
          .hwnd = GetWindow(windows.popup(), GW_OWNER)},
      .framebuffer_size = {96, 96}, .scale = {1}};
  auto live = create_dx12_renderer(descriptor);
  if (!live) return dx12_fail(3, live.error().message.c_str());
  const ImageAsset image{.id = {8602}, .logical_size = {1, 1},
      .bitmap = {.width = 1, .height = 1, .stride = 4, .pixels = {255, 0, 0, 128}}};
  for (const bool destroy_hwnd : {false, true}) {
    auto background = cgpui_test::wait_for_test_frame(**live);
    if (!background) return dx12_fail(18, "live opaque owner warmup begin failed");
    (*background)->clear({0, 1, 0, 1});
    if (!(*background)->present()) return dx12_fail(19, "live opaque owner warmup present failed");
    // The second pass also proves the first host released this HWND's target.
    Dx12PacingForwardApi api; // Outlives this pass's facade and both orphan frames.
    auto popup = dx12_create_renderer(dx12_popup_descriptor(windows.popup()), api);
    if (!popup) return dx12_fail(4, popup.error().message.c_str());
    if (api.sets != 1 || api.gets != 1 || !api.observed_handle || api.closes)
      return dx12_fail(23, "popup lifetime lacks one configured native pacing epoch");
    const auto before = (*live)->backend_info();
    if (before.adapter_luid != (*popup)->backend_info().adapter_luid)
      return dx12_fail(5, "orphan popup and live owner selected different adapters");
    auto finished = cgpui_test::wait_for_test_frame(**popup);
    if (!finished) return dx12_fail(6, "orphan popup warmup begin failed");
    (*finished)->clear({0, 0, 0, 0});
    (*finished)->upload_image(image);
    (*finished)->draw_image({.bounds = {{0, 0}, {64, 64}}, .asset = describe_image_asset(image)});
    if (!(*finished)->request_pixel_capture() || !(*finished)->present() ||
        !dx12_pixel((*popup)->last_frame_pixels(), 32, 32, {128, 0, 0, 128}) ||
        !windows.expect({128, 127, 0}, windows.popup_point(32, 32)))
      return dx12_fail(7, "orphan popup texture/desktop warmup failed");
    if (!destroy_hwnd) {
      // DComp allows one target per HWND/layer. Our host always uses topmost;
      // failed partial construction must leave the existing target untouched.
      Dx12PacingForwardApi duplicate_api;
      const auto duplicate = dx12_create_renderer(dx12_popup_descriptor(windows.popup()), duplicate_api);
      if (duplicate || duplicate.error().message.find("composition HWND target") == std::string::npos)
        return dx12_fail(20, "duplicate popup factory did not reject the occupied DComp layer");
      if (duplicate_api.sets != 1 || duplicate_api.gets != 1 || !duplicate_api.observed_handle ||
          duplicate_api.polls || duplicate_api.closes != 1 || !duplicate_api.close_ok || api.closes)
        return dx12_fail(24, "post-handle failed construction did not close only its own handle once");
      auto updated = cgpui_test::wait_for_test_frame(**popup);
      if (!updated) return dx12_fail(21, "original popup unusable after duplicate factory failure");
      (*updated)->clear({0, 0, 1, 0.5F});
      if (!(*updated)->request_pixel_capture() || !(*updated)->present() ||
          !dx12_pixel((*popup)->last_frame_pixels(), 32, 32, {0, 0, 128, 128}) ||
          !windows.expect({0, 127, 128}, windows.popup_point(32, 32)))
        return dx12_fail(22, "failed duplicate factory corrupted original popup presentation");
    }
    auto orphan = cgpui_test::wait_for_test_frame(**popup);
    if (!orphan) return dx12_fail(8, "orphan popup CPU frame begin failed");
    (*orphan)->draw_image({.bounds = {{0, 0}, {64, 64}}, .asset = describe_image_asset(image)});
    popup->reset(); // A finished and an active CPU frame both outlive their owner.
    const auto polls = api.polls;
    if (api.closes || api.ready < 2) return dx12_fail(25, "facade closed orphan's native pacing");
    if (destroy_hwnd && !DestroyWindow(windows.popup()))
      return dx12_fail(9, "orphan popup HWND destruction failed");
    if (!dx12_remove_native_device(before)) return dx12_fail(10, "orphan popup native removal failed");
    const auto retry = (*live)->begin_frame();
    if (retry || retry.error().code != ErrorCode::frame_retry_required)
      return dx12_fail(11, "orphan DComp host blocked live owner device recovery");
    if (api.closes != 1 || !api.close_ok || api.sets != 1 || api.gets != 1 || api.polls != polls)
      return dx12_fail(26, "orphan peer purge did not retire its handle exactly once without rebuilding");
    const auto after = (*live)->backend_info();
    if (after.device_generation != before.device_generation + 1 || after.adapter_luid != before.adapter_luid)
      return dx12_fail(12, "orphan purge changed live owner generation/adapter");
    (*orphan)->clear({0, 0, 1, 1});
    const auto rejected = (*orphan)->present();
    if (rejected || rejected.error().code != ErrorCode::invalid_argument || (*finished)->present())
      return dx12_fail(13, "ownerless popup frame rebuilt or presented");
    auto frame = cgpui_test::wait_for_test_frame(**live);
    if (!frame) return dx12_fail(14, "live owner unavailable after orphan rejection");
    finished->reset();
    orphan->reset(); // Final state destruction must not hang, even without the HWND.
    if (api.closes != 1 || !api.close_ok || api.polls != polls)
      return dx12_fail(27, "destroyed-window/orphan cleanup double-closed or polled a retired handle");
    const auto overlap = (*live)->begin_frame();
    if (overlap || overlap.error().code != ErrorCode::invalid_argument)
      return dx12_fail(15, "orphan frame destruction unlocked the live owner frame");
    (*frame)->clear({0, 1, 0, 1});
    if (!(*frame)->request_pixel_capture() || !(*frame)->present() ||
        !dx12_pixel((*live)->last_frame_pixels(), 32, 32, {0, 255, 0, 255}) ||
        !windows.expect({0, 255, 0}, windows.popup_point(32, 32)))
      return dx12_fail(16, "stale orphan visual remains after native retirement");
    if (!dx12_recovered_native_device_clean(after)) return dx12_fail(17, "orphan popup recovery debug errors");
  }
  return 0;
}
