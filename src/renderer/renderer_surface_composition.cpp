#include "cgpui/renderer/renderer.hpp"

namespace cgpui {

// Renderers without a presentation surface (test and headless renderers)
// report no composition and cannot switch to a transparent surface.
RendererSurfaceComposition Renderer::surface_composition() const {
  return RendererSurfaceComposition{};
}

Result<RendererSurfaceComposition> Renderer::set_transparent_surface(
    bool transparent) {
  (void)transparent;
  return std::unexpected(Error{
      .code = ErrorCode::renderer_initialization_failed,
      .message = "renderer has no presentation surface to make transparent",
  });
}

} // namespace cgpui
