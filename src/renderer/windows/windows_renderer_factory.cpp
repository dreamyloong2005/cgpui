#include "cgpui/renderer/renderer_backend.hpp"
#include "../renderer_backend_selection_internal.hpp"

#include <cstdlib>
#include <memory>
#include <string_view>

namespace cgpui {

Result<std::unique_ptr<Renderer>> create_renderer(
    const RenderSurfaceDescriptor& descriptor) {
  // Own the environment snapshot: factories may create windows and invoke
  // callbacks before selection returns.
  char* value = nullptr;
  std::size_t length = 0;
  if (_dupenv_s(&value, &length, "CGPUI_RENDERER") != 0) {
    return std::unexpected(Error{
        .code = ErrorCode::renderer_initialization_failed,
        .message = "could not read CGPUI_RENDERER",
    });
  }
  const std::unique_ptr<char, decltype(&std::free)> setting(value, &std::free);
  return detail::select_renderer_backend(
      descriptor, value ? std::string_view(value) : std::string_view{},
      create_vulkan_renderer, create_dx12_renderer);
}

} // namespace cgpui
