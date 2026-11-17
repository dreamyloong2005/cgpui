#include "cgpui/platform/platform.hpp"
#include "cgpui/renderer/renderer.hpp"

#include <iostream>

namespace {

using cgpui::WindowBackdrop;

constexpr std::uint32_t premultiplied_bit = 0x2U;

int fail(int code, const char* message) {
  std::cerr << "win32 vulkan backdrop: " << message << '\n';
  return code;
}

} // namespace

// A window created with a backdrop asks the renderer for a transparent
// surface; device selection must then use a GPU whose surface supports
// premultiplied alpha when one exists, and the backdrop must cover the
// client area exactly when the chosen surface composites with alpha.
int main() {
  auto app = cgpui::create_platform_application();
  if (!app) return fail(1, "platform application was not created");
  cgpui::WindowDescriptor descriptor;
  descriptor.title = "CGPUI vulkan backdrop test";
  descriptor.size = cgpui::Size{.width = 320.0F, .height = 200.0F};
  descriptor.backdrop = WindowBackdrop::mica;
  auto window = (*app)->create_window(descriptor, [](const auto&) {});
  if (!window) return fail(2, "window was not created");
  const cgpui::WindowState state = (*window)->state();
  auto renderer = cgpui::create_renderer(cgpui::RenderSurfaceDescriptor{
      .native_surface = (*window)->native_surface(),
      .framebuffer_size = state.framebuffer_size,
      .scale = state.scale,
      .transparent_background = true});
  if (!renderer) return fail(3, "renderer was not created");
  const cgpui::RendererSurfaceComposition composition =
      (*renderer)->surface_composition();
  const bool premultiplied =
      (composition.supported_composite_alpha & premultiplied_bit) != 0;
  std::cout << "composite alpha mask 0x" << std::hex
            << composition.supported_composite_alpha << std::dec
            << ", composited " << composition.alpha_composited() << '\n';
  if (!composition.supported || !composition.transparent_requested) {
    return fail(4, "the transparent request was not reported");
  }
  if (premultiplied != composition.alpha_composited()) {
    return fail(5, "premultiplied support must select premultiplied alpha");
  }
  const auto applied = (*window)->apply_window_backdrop(
      WindowBackdrop::mica, composition.alpha_composited());
  std::cout << "applied " << static_cast<int>(applied.applied) << ", reason '"
            << applied.reason << "'\n";
  if (!applied.supported) return 0;  // old build or transparency disabled
  if ((applied.applied == WindowBackdrop::mica) !=
      composition.alpha_composited()) {
    return fail(6, "client-area mica must follow the surface composition");
  }
  return 0;
}
