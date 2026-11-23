#include "renderer_backdrop_pass_test_support.hpp"
#include "renderer_pixel_test_support.hpp"

int main() {
  cgpui_test::RendererPixelTestWindow window(
      L"CGPUIvulkanBackdropPassDiagnostics", L"vulkan exact backdrop passes");
  const auto size = window.resize_client(128, 128);
  const unsigned expected[] = {0, 5, 9};
  for (unsigned effects = 0; effects != 3; ++effects) {
    // This backend-specific test deliberately ignores CGPUI_RENDERER.
    auto renderer = cgpui::create_vulkan_renderer({
        .native_surface = window.surface(), .framebuffer_size = size, .scale = {1}});
    if (!renderer) {
      cgpui_test::report_pixel_test_error("vulkan diagnostic creation", renderer.error());
      return 1;
    }
    if (!cgpui_test::pixel_test_ok(cgpui_test::validate_pixel_backend_info(
            cgpui::RendererBackend::vulkan, (*renderer)->backend_info()), "vulkan diagnostic identity")) return 2;
    if (const int result = cgpui_test::check_backdrop_pass_counts(
            **renderer, effects, expected[effects], effects == 1 ? 2 : 1)) return 10 + result;
  }
  return 0;
}
