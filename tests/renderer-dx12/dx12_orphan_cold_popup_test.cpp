#include "dx12_popup_test_support.hpp"
#include "dx12_pacing_begin_test_support.hpp"
#include "../../src/renderer/dx12/dx12_renderer_creation_internal.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12RecoveryDeadline deadline;
  if (!deadline.done || !dx12_popup_desktop_ready() || !pacing_begin_debug_enable())
    return dx12_fail(1, "cold orphan popup desktop/debug fixture unavailable");
  CompositionTestWindows windows(48, 48);
  Dx12PacingForwardApi api, replacement_api;
  if (!windows.valid() || !windows.expect({0, 255, 0}, windows.backdrop_point()) ||
      !windows.expect({0, 255, 0}, windows.popup_point(32, 32)))
    return dx12_fail(2, "cold popup must initially expose only the controlled green backing");
  auto created = dx12_create_renderer(dx12_popup_descriptor(windows.popup()), api);
  if (!created) return dx12_fail(3, created.error().message.c_str());
  auto renderer = std::move(*created);
  const auto info = renderer->backend_info();
  if (renderer->last_frame_pixels() || renderer->last_frame_diagnostic_snapshot() ||
      renderer->surface_composition().composite_alpha != RendererCompositeAlpha::pre_multiplied ||
      api.sets != 1 || api.gets != 1 || api.polls || api.closes || !pacing_begin_debug_clean(info))
    return dx12_fail(4, "cold orphan must have no prior Present/publication or warmed native admission");
  auto orphan = pacing_begin_bounded(*renderer, api); // Its first acquired frame; no warming Present/Commit.
  if (!orphan || api.ready != 1 || api.polls == 0) return dx12_fail(5, "cold orphan first native grant absent");
  const auto polls = api.polls;
  renderer.reset();
  if (api.closes || !(*orphan)->request_pixel_capture()) return dx12_fail(6, "cold orphan lost state/capture permission");
  (*orphan)->clear({0, 0, 0, 0});
  (*orphan)->draw_rect({.rect = {{0, 0}, {16, 64}}, .color = {1, 0, 0, 0.5F}});
  (*orphan)->draw_rect({.rect = {{48, 0}, {16, 64}}, .color = {0, 0, 1, 1}});
  const auto presented = (*orphan)->present(); // Real first Commit must happen inside production submission.
  if (!presented || !windows.expect({128, 127, 0}, windows.popup_point(8, 32)) ||
      !windows.expect({0, 255, 0}, windows.popup_point(32, 32)) ||
      !windows.expect({0, 0, 255}, windows.popup_point(56, 32)) ||
      !windows.expect({0, 255, 0}, windows.backdrop_point()) || !pacing_begin_debug_clean(info))
    return dx12_fail(7, "cold orphan first-Commit desktop alpha pixels differ");
  if ((*orphan)->present() || (*orphan)->request_pixel_capture() || api.closes ||
      api.sets != 1 || api.gets != 1 || api.polls != polls || api.ready != 1)
    return dx12_fail(8, "cold orphan completion reacquired or prematurely released native ownership");
  orphan->reset();
  if (api.closes != 1 || !api.close_ok || !windows.expect({0, 255, 0}, windows.popup_point(8, 32)) ||
      !windows.expect({0, 255, 0}, windows.popup_point(56, 32)))
    return dx12_fail(9, "last cold orphan release left its visual or pacing handle behind");
  auto replacement = dx12_create_renderer(dx12_popup_descriptor(windows.popup()), replacement_api);
  if (!replacement) return dx12_fail(10, "orphan cleanup did not release the exact HWND's composition target");
  auto next = pacing_begin_bounded(**replacement, replacement_api);
  if (!next) return dx12_fail(11, "replacement popup first frame unavailable");
  (*next)->clear({0, 0, 1, 0.5F});
  if (!(*next)->request_pixel_capture() || !(*next)->present() ||
      !dx12_pixel((*replacement)->last_frame_pixels(), 32, 32, {0, 0, 128, 128}) ||
      !windows.expect({0, 127, 128}, windows.popup_point(32, 32)))
    return dx12_fail(12, "same-HWND replacement popup capture/desktop output failed");
  next->reset(); replacement->reset();
  return api.closes == 1 && replacement_api.closes == 1 && replacement_api.close_ok
      ? 0 : dx12_fail(13, "cold/replacement cleanup leaked or double-closed native pacing");
}
