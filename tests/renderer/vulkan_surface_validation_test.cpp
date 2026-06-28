#include "cgpui/renderer/renderer.hpp"

namespace {

int expect_renderer_initialization_failure(
    const cgpui::RenderSurfaceDescriptor& descriptor) {
  const auto renderer = cgpui::create_renderer(descriptor);
  if (renderer) {
    return 1;
  }

  return renderer.error().code ==
          cgpui::ErrorCode::renderer_initialization_failed
      ? 0
      : 2;
}

} // namespace

int main() {
  const auto framebuffer = cgpui::Size{64.0F, 64.0F};
  const auto scale = cgpui::DpiScale{1.0F};

#if defined(_WIN32)
  const int non_win32_result = expect_renderer_initialization_failure(
      cgpui::RenderSurfaceDescriptor{
          .native_surface = cgpui::WaylandSurfaceHandle{},
          .framebuffer_size = framebuffer,
          .scale = scale});
  if (non_win32_result != 0) {
    return non_win32_result;
  }

  const int null_win32_result = expect_renderer_initialization_failure(
      cgpui::RenderSurfaceDescriptor{
          .native_surface = cgpui::Win32SurfaceHandle{},
          .framebuffer_size = framebuffer,
          .scale = scale});
  if (null_win32_result != 0) {
    return 10 + null_win32_result;
  }
#else
  const int null_wayland_result = expect_renderer_initialization_failure(
      cgpui::RenderSurfaceDescriptor{
          .native_surface = cgpui::WaylandSurfaceHandle{},
          .framebuffer_size = framebuffer,
          .scale = scale});
  if (null_wayland_result != 0) {
    return null_wayland_result;
  }
#endif

  return 0;
}
