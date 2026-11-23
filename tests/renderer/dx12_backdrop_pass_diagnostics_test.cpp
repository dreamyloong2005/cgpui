#include "renderer_backdrop_pass_test_support.hpp"
#include "renderer_pixel_test_support.hpp"

int main() {
  cgpui_test::RendererPixelTestWindow window(
      L"CGPUIdx12BackdropPassDiagnostics", L"dx12 exact backdrop passes");
  const auto size = window.resize_client(128, 128);
  const unsigned expected[] = {0, 3, 6};
  for (unsigned effects = 0; effects != 3; ++effects) {
    // This backend-specific test deliberately ignores CGPUI_RENDERER.
    auto renderer = cgpui::create_dx12_renderer({
        .native_surface = window.surface(), .framebuffer_size = size, .scale = {1}});
    if (!renderer) {
      cgpui_test::report_pixel_test_error("dx12 diagnostic creation", renderer.error());
      return 1;
    }
    if (!cgpui_test::pixel_test_ok(cgpui_test::validate_pixel_backend_info(
            cgpui::RendererBackend::dx12, (*renderer)->backend_info()), "dx12 diagnostic identity")) return 2;
    if (const int result = cgpui_test::check_backdrop_pass_counts(
            **renderer, effects, expected[effects], effects == 1 ? 2 : 1)) return 10 + result;
  }
  return 0;
}
