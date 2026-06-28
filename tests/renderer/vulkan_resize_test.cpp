#include "cgpui/renderer/renderer.hpp"

int main() {
  auto renderer = cgpui::create_renderer(cgpui::RenderSurfaceDescriptor{
      .native_surface = cgpui::Win32SurfaceHandle{},
      .framebuffer_size = cgpui::Size{64.0F, 64.0F},
      .scale = cgpui::DpiScale{1.0F}});
  if (!renderer) {
    return 1;
  }

  const auto resized =
      (*renderer)->resize(cgpui::Size{0.0F, 64.0F}, cgpui::DpiScale{1.0F});
  if (resized) {
    return 2;
  }

  return resized.error().code == cgpui::ErrorCode::renderer_initialization_failed
      ? 0
      : 3;
}
