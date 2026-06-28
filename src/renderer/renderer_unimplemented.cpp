#include "cgpui/renderer/renderer.hpp"

#include <memory>
#include <string>
#include <utility>

namespace cgpui {

Result<std::unique_ptr<Renderer>> create_renderer(
    const RenderSurfaceDescriptor& descriptor) {
  static_cast<void>(descriptor);
  return std::unexpected(Error{
      .code = ErrorCode::renderer_initialization_failed,
      .message = "No renderer implementation is enabled for this host",
  });
}

} // namespace cgpui
