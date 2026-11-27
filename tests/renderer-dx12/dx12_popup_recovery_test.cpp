#include "../support/frame_wait_test_support.hpp"
#include "dx12_popup_test_support.hpp"
#include "dx12_pacing_begin_test_support.hpp"
#include "../../src/renderer/dx12/dx12_renderer_creation_internal.hpp"

using namespace cgpui;
using namespace cgpui_test;

namespace {
void paint(RenderFrame& frame, const ImageAsset& image, bool blue) {
  frame.clear({0, 0, 0, 0});
  frame.draw_image({.bounds = {{0, 0}, {20, 64}}, .asset = describe_image_asset(image)});
  frame.draw_rect({.rect = {{40, 0}, {24, 64}},
      .color = blue ? Color{0, 0, 1, 1} : Color{1, 0, 0, 1}});
  // Exercise alpha-PSO/snapshot retirement too; this filter's padded source
  // stays inside the transparent band, so the independent pixel oracle is zero.
  frame.draw_backdrop_blur({.rect = {{26, 20}, {8, 24}}, .blur_radius = 1});
}
bool finish(Renderer& renderer, RenderFrame& frame, bool blue) {
  return frame.request_pixel_capture() && frame.present() &&
      dx12_pixel(renderer.last_frame_pixels(), 10, 32, {128, 0, 0, 128}) &&
      dx12_pixel(renderer.last_frame_pixels(), 30, 32, {0, 0, 0, 0}) &&
      dx12_pixel(renderer.last_frame_pixels(), 52, 32,
          blue ? std::array<unsigned char, 4>{0, 0, 255, 255}
               : std::array<unsigned char, 4>{255, 0, 0, 255});
}
bool desktop(const CompositionTestWindows& windows, bool blue) {
  return windows.expect({0, 255, 0}, windows.backdrop_point()) &&
      windows.expect({128, 127, 0}, windows.popup_point(10, 32)) &&
      windows.expect({0, 255, 0}, windows.popup_point(30, 32)) &&
      windows.expect(blue ? ScreenColor{0, 0, 255} : ScreenColor{255, 0, 0},
          windows.popup_point(52, 32));
}
bool paint_parent(Renderer& renderer) {
  auto frame = cgpui_test::wait_for_test_frame(renderer);
  if (!frame) return false;
  (*frame)->clear({0, 1, 0, 1});
  return (*frame)->request_pixel_capture() && (*frame)->present() &&
      dx12_pixel(renderer.last_frame_pixels(), 8, 8, {0, 255, 0, 255});
}
} // namespace

int main() {
  Dx12RecoveryDeadline deadline{DesktopTestAccess::exclusive};
  if (!deadline.done || !dx12_popup_desktop_ready())
    return dx12_fail(1, "popup recovery desktop/DPI/watchdog unavailable");
  CompositionTestWindows windows(48, 48);
  if (!windows.valid() || !windows.expect({0, 255, 0}, windows.backdrop_point()))
    return dx12_fail(2, "popup recovery visible backdrop unavailable");
  Dx12PacingForwardApi api; // Outlives facade and every retained old-generation CPU frame.
  auto popup = dx12_create_renderer(dx12_popup_descriptor(windows.popup()), api);
  if (!popup) return dx12_fail(3, popup.error().message.c_str());
  if (api.sets != 1 || api.gets != 1 || !api.observed_handle || api.closes)
    return dx12_fail(23, "popup recovery lacks a configured native pacing epoch");
  const ImageAsset image{.id = {8601}, .logical_size = {1, 1},
      .bitmap = {.width = 1, .height = 1, .stride = 4, .pixels = {255, 0, 0, 128}}};
  std::vector<std::unique_ptr<RenderFrame>> retained;
  auto warmup = cgpui_test::wait_for_test_frame(**popup);
  if (!warmup) return dx12_fail(4, "popup recovery warmup begin failed");
  (*warmup)->upload_image(image); // The only upload, across both loss epochs.
  paint(**warmup, image, false);
  if (!finish(**popup, **warmup, false) || !desktop(windows, false))
    return dx12_fail(5, "popup recovery warmup texture/desktop pixels differ");
  retained.push_back(std::move(*warmup));
  std::unique_ptr<Renderer> parent;
  for (unsigned epoch = 0; epoch != 2; ++epoch) {
    if (epoch == 1) {
      // Use the popup's real ordinary HWND owner, not an unrelated test window.
      const RenderSurfaceDescriptor descriptor{
          .native_surface = Win32SurfaceHandle{.hinstance = GetModuleHandleW(nullptr),
              .hwnd = GetWindow(windows.popup(), GW_OWNER)},
          .framebuffer_size = {96, 96}, .scale = {1}};
      auto created = create_dx12_renderer(descriptor);
      if (!created) return dx12_fail(6, created.error().message.c_str());
      parent = std::move(*created);
      if (!paint_parent(*parent)) return dx12_fail(7, "opaque owner warmup failed");
    }
    const auto before = (*popup)->backend_info();
    const auto parent_before = parent ? parent->backend_info() : RendererBackendInfo{};
    if (parent && parent_before.adapter_luid != before.adapter_luid)
      return dx12_fail(8, "popup and opaque owner did not use the same adapter");
    auto pending = cgpui_test::wait_for_test_frame(**popup);
    if (!pending) return dx12_fail(9, "pre-loss popup CPU frame begin failed");
    paint(**pending, image, false);
    if (!(*pending)->request_pixel_capture()) return dx12_fail(10, "pre-loss capture request failed");
    const auto* borrowed = (*popup)->last_frame_pixels();
    if (!borrowed) return dx12_fail(10, "pre-loss popup capture unavailable");
    const auto saved = borrowed->rgba8;
    const auto polls = api.polls, ready = api.ready;
    // The helper drops both ID3D12Device5 and InfoQueue before any recovery.
    if (!dx12_remove_native_device(before)) return dx12_fail(11, "popup native removal failed");
    if (parent) {
      const auto retry = parent->begin_frame();
      if (retry || retry.error().code != ErrorCode::frame_retry_required)
        return dx12_fail(12, "popup DComp references blocked opaque owner recovery");
      const auto after = parent->backend_info();
      if (after.device_generation != parent_before.device_generation + 1 ||
          after.adapter_luid != parent_before.adapter_luid || parent->last_frame_pixels() ||
          (*popup)->backend_info().device_generation != before.device_generation ||
          (*popup)->last_frame_pixels())
        return dx12_fail(13, "popup cohort generations/capture visibility differ");
      // Peer purge hides but must not free the borrowed CPU capture. Do not
      // dereference it after the pending popup operation starts its rebuild.
      if (borrowed->rgba8 != saved || !dx12_pixel(borrowed, 10, 32, {128, 0, 0, 128}))
        return dx12_fail(14, "popup cohort purge invalidated a borrowed CPU capture");
      if (api.closes != epoch + 1 || api.sets != epoch + 1 || api.gets != epoch + 1 ||
          api.polls != polls || api.ready != ready || !api.close_ok)
        return dx12_fail(24, "live peer purge polled/rebuilt instead of retiring only its old handle");
    }
    const auto retry = (*pending)->present();
    if (retry || retry.error().code != ErrorCode::frame_retry_required)
      return dx12_fail(15, "old popup CPU frame did not rebuild and request retry");
    if (api.closes != epoch + 1 || api.sets != epoch + 2 || api.gets != epoch + 2 ||
        api.polls != polls || api.ready != ready || !api.close_ok)
      return dx12_fail(25, "lazy peer rebuild did not close old/configure new epoch before admission");
    const auto after = (*popup)->backend_info();
    if (after.device_generation != before.device_generation + 1 ||
        after.adapter_luid != before.adapter_luid || after.software_adapter != before.software_adapter ||
        (*popup)->last_frame_pixels())
      return dx12_fail(16, "popup rebuild adapter/generation/capture differs");
    const auto repeated = (*pending)->present();
    if (repeated || repeated.error().code != ErrorCode::invalid_argument)
      return dx12_fail(17, "pre-loss popup CPU frame was presentable twice");
    retained.push_back(std::move(*pending));
    if (parent && !paint_parent(*parent)) return dx12_fail(18, "recovered opaque owner pixels differ");
    auto frame = cgpui_test::wait_for_test_frame(**popup);
    if (!frame) return dx12_fail(19, "recovered popup is not immediately usable");
    if (api.polls <= polls || api.ready != ready + 1)
      return dx12_fail(26, "rebuilt composition peer reused an old grant rather than native readiness");
    if (epoch == 1) {
      retained.clear();
      const auto overlap = (*popup)->begin_frame();
      if (overlap || overlap.error().code != ErrorCode::invalid_argument)
        return dx12_fail(20, "old popup frame destruction unlocked the new frame");
    }
    const bool blue = epoch == 0; // Red -> blue -> red rejects an old visual's stale frame.
    paint(**frame, image, blue);
    if (!finish(**popup, **frame, blue) || !desktop(windows, blue))
      return dx12_fail(21, "recovered popup cache/real desktop alpha differs");
    retained.push_back(std::move(*frame));
    if (!dx12_recovered_native_device_clean(after))
      return dx12_fail(22, "recovered popup device reports debug errors");
  }
  retained.clear();
  popup->reset();
  return api.sets == 3 && api.gets == 3 && api.closes == 3 && api.close_ok
      ? 0 : dx12_fail(27, "composition peer epochs did not each close once");
}
