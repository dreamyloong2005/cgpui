#include "dx12_popup_test_support.hpp"
#include "dx12_pacing_begin_test_support.hpp"
#include "../../src/renderer/dx12/dx12_renderer_creation_internal.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12RecoveryDeadline deadline;
  if (!deadline.done || !dx12_popup_desktop_ready() || !pacing_begin_debug_enable())
    return dx12_fail(1, "orphan image batch desktop/debug fixture unavailable");
  CompositionTestWindows windows(48, 48);
  Dx12PacingForwardApi api;
  if (!windows.valid() || !windows.expect({0, 255, 0}, windows.popup_point(8, 32)))
    return dx12_fail(2, "orphan image batch backing unavailable");
  auto created = dx12_create_renderer(dx12_popup_descriptor(windows.popup()), api);
  if (!created) return dx12_fail(3, "orphan image batch renderer unavailable");
  auto renderer = std::move(*created);
  const auto info = renderer->backend_info();
  auto orphan = pacing_begin_bounded(*renderer, api);
  if (!orphan) return dx12_fail(4, "orphan image batch frame unavailable");
  ImageAsset red{.id = {9883}, .logical_size = {1, 1}, .bitmap = {.width = 1, .height = 1, .stride = 4,
      .pixels = {255, 0, 0, 255}}};
  auto blue = red; blue.bitmap.pixels = {0, 0, 255, 255};
  const auto asset = describe_image_asset(red);
  (*orphan)->upload_image(red);
  (*orphan)->draw_image({.bounds = {{0, 0}, {16, 64}}, .asset = asset});
  renderer.reset();
  (*orphan)->clear({0, 0, 0, 0});
  (*orphan)->invalidate_image(asset.id);
  (*orphan)->upload_image(blue); blue.bitmap.pixels.assign(4, 0);
  (*orphan)->draw_image({.bounds = {{48, 0}, {16, 64}}, .asset = asset});
  if (!(*orphan)->request_pixel_capture() || !(*orphan)->present() ||
      !windows.expect({0, 0, 255}, windows.popup_point(8, 32)) ||
      !windows.expect({0, 255, 0}, windows.popup_point(32, 32)) ||
      !windows.expect({0, 0, 255}, windows.popup_point(56, 32)) ||
      !pacing_begin_debug_clean(info) || api.closes || api.sets != 1 || api.gets != 1)
    return dx12_fail(5, "healthy orphan lost copied final image batch or first-Commit pixels");
  orphan->reset();
  if (api.closes != 1 || !api.close_ok || !windows.expect({0, 255, 0}, windows.popup_point(8, 32)))
    return dx12_fail(6, "final orphan image owner leaked native state/visual");
  return 0;
}
