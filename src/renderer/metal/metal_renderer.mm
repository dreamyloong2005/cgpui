#include "metal_renderer_internal.hpp"

namespace cgpui {

Result<std::unique_ptr<Renderer>> create_renderer(
    const RenderSurfaceDescriptor& descriptor) {
  auto state = create_metal_renderer_state(descriptor);
  if (!state) return std::unexpected(state.error());
  return make_metal_renderer(std::move(*state));
}

}  // namespace cgpui
