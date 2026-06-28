#include "cgpui/renderer/renderer.hpp"

#include <vulkan/vulkan.h>

#include <expected>
#include <memory>
#include <variant>
#include <utility>

namespace cgpui {
namespace {

Result<Win32SurfaceHandle> require_win32_surface(
    const NativeSurfaceHandle& native_surface) {
  const auto* surface = std::get_if<Win32SurfaceHandle>(&native_surface);
  if (surface == nullptr) {
    return std::unexpected(Error{
        .code = ErrorCode::renderer_initialization_failed,
        .message = "Vulkan renderer requires a Win32 native surface",
    });
  }

  if (surface->hinstance == nullptr || surface->hwnd == nullptr) {
    return std::unexpected(Error{
        .code = ErrorCode::renderer_initialization_failed,
        .message = "Vulkan renderer requires non-null Win32 handles",
    });
  }

  return *surface;
}

class VulkanFrame final : public RenderFrame {
 public:
  void clear(Color color) override { clear_color_ = color; }
  void draw_rect(const SolidRect& rect) override { rect_ = rect; }
  Result<void> present() override { return {}; }

 private:
  Color clear_color_{};
  SolidRect rect_{};
};

class VulkanRenderer final : public Renderer {
 public:
  explicit VulkanRenderer(RenderSurfaceDescriptor descriptor)
      : descriptor_(std::move(descriptor)) {}

  Result<void> resize(Size framebuffer_size, DpiScale scale) override {
    if (framebuffer_size.width <= 0.0F || framebuffer_size.height <= 0.0F) {
      return std::unexpected(Error{
          .code = ErrorCode::renderer_initialization_failed,
          .message = "Vulkan renderer requires a non-empty framebuffer",
      });
    }

    descriptor_.framebuffer_size = framebuffer_size;
    descriptor_.scale = scale;
    return {};
  }

  Result<std::unique_ptr<RenderFrame>> begin_frame() override {
    return std::make_unique<VulkanFrame>();
  }

 private:
  RenderSurfaceDescriptor descriptor_;
  VkInstance instance_ = VK_NULL_HANDLE;
};

} // namespace

Result<std::unique_ptr<Renderer>> create_renderer(
    const RenderSurfaceDescriptor& descriptor) {
  if (descriptor.framebuffer_size.width <= 0.0F ||
      descriptor.framebuffer_size.height <= 0.0F) {
    return std::unexpected(Error{
        .code = ErrorCode::renderer_initialization_failed,
        .message = "Vulkan renderer requires a non-empty framebuffer",
    });
  }

  if (auto surface = require_win32_surface(descriptor.native_surface);
      !surface) {
    return std::unexpected(surface.error());
  }

  return std::make_unique<VulkanRenderer>(descriptor);
}

} // namespace cgpui
