#include "dx12_dynamic_client_test_support.hpp"
#include "dx12_client_alpha_test_support.hpp"

namespace cgpui_test {
bool dynamic_client_resize(cgpui::Renderer& renderer, cgpui::PlatformWindow& window) {
  const auto geometry = window.state();
  client_alpha_regions(window);
  return bool(renderer.resize(geometry.framebuffer_size, geometry.scale));
}

bool dynamic_client_paint(cgpui::Renderer& renderer, const cgpui::PlatformWindow& window, bool blue, bool alpha) {
  const auto identity = renderer.backend_info();
  if (identity.backend != cgpui::RendererBackend::dx12 || identity.selection != cgpui::RendererBackendSelection::dx12 ||
      !identity.adapter_luid || !*identity.adapter_luid || !identity.device_generation || !identity.fallback_reason.empty()) return false;
  std::cout << "DYNAMIC_CLIENT_NATIVE adapter=" << identity.adapter_name << " luid=" << *identity.adapter_luid
            << " software=" << identity.software_adapter << " generation=" << identity.device_generation
            << " alpha=" << alpha << '\n';
  // Existing helper checks exact GPU body+title-row RGBA. Black/zero clear makes
  // these pixels identical for IGNORE/PREMULT; desktop checks distinguish them.
  if (!ordinary_composition(renderer.surface_composition(), alpha) ||
      !client_alpha_paint(renderer, window, blue, false) || !client_alpha_hits(window) ||
      !ordinary_desktop_bands(window, blue, alpha)) return false;
  const auto* pixels = renderer.last_frame_pixels();
  const LONG y = LONG(std::lround((window.frame_metrics().client_resize_border.top + 8) * window.state().scale.value));
  POINT origin{};
  if (!pixels || !ClientToScreen(ordinary_hwnd(window), &origin)) return false;
  const int green = alpha ? 127 : 0;
  return expect_screen_color(blue ? ScreenColor{0, green, 128} : ScreenColor{128, green, 0},
                            {origin.x + LONG(pixels->width / 6), origin.y + y}) &&
      expect_screen_color(alpha ? ScreenColor{0, 255, 0} : ScreenColor{0, 0, 0},
                          {origin.x + LONG(pixels->width / 2), origin.y + y}) &&
      expect_screen_color(blue ? ScreenColor{0, 0, 255} : ScreenColor{255, 0, 0},
                          {origin.x + LONG(pixels->width * 5 / 6), origin.y + y});
}
} // namespace cgpui_test
