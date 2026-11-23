#include "cgpui/renderer/renderer_backend.hpp"

#if defined(__linux__)
namespace cgpui {

Result<std::unique_ptr<Renderer>> create_renderer(
    const RenderSurfaceDescriptor& descriptor) {
  return create_vulkan_renderer(descriptor);
}

} // namespace cgpui
#endif
