#include "ui_internal.hpp"

#include <string>

namespace cgpui {

Result<Renderer*> WindowRuntime::try_create_renderer(
    const RenderSurfaceDescriptor& descriptor) {
  return try_create_renderer(
      descriptor,
      "Renderer factory returned an empty renderer");
}

Result<Renderer*> WindowRuntime::try_create_renderer(
    const RenderSurfaceDescriptor& descriptor,
    std::string_view empty_renderer_message) {
  if (!state_->core.renderer_factory_ || state_->core.shutting_down_) {
    return std::unexpected(Error{
        .code = ErrorCode::renderer_initialization_failed,
        .message = "Window runtime requires a renderer factory"});
  }

  auto renderer_result = state_->core.renderer_factory_(descriptor);
  if (!renderer_result) {
    return std::unexpected(renderer_result.error());
  }
  if (*renderer_result == nullptr) {
    return std::unexpected(Error{
        .code = ErrorCode::renderer_initialization_failed,
        .message = std::string(empty_renderer_message)});
  }
  return *renderer_result;
}

} // namespace cgpui
