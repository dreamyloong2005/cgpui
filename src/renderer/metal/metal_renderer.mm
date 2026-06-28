#include "cgpui/renderer/renderer.hpp"

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#include <expected>
#include <memory>
#include <utility>

namespace cgpui {
namespace {

class MetalFrame final : public RenderFrame {
 public:
  void clear(Color color) override { clear_color_ = color; }
  void draw_rect(const SolidRect& rect) override { rect_ = rect; }
  Result<void> present() override { return {}; }

 private:
  Color clear_color_{};
  SolidRect rect_{};
};

class MetalRenderer final : public Renderer {
 public:
  MetalRenderer(RenderSurfaceDescriptor descriptor, id<MTLDevice> device)
      : descriptor_(std::move(descriptor)), device_([device retain]) {}

  ~MetalRenderer() override { [device_ release]; }

  Result<void> resize(Size framebuffer_size, DpiScale scale) override {
    if (framebuffer_size.width <= 0.0F || framebuffer_size.height <= 0.0F) {
      return std::unexpected(Error{
          .code = ErrorCode::renderer_initialization_failed,
          .message = "Metal renderer requires a non-empty framebuffer",
      });
    }

    descriptor_.framebuffer_size = framebuffer_size;
    descriptor_.scale = scale;
    if (auto* surface = std::get_if<MetalSurfaceHandle>(&descriptor_.native_surface);
        surface != nullptr && surface->layer != nullptr) {
      auto* layer = static_cast<CAMetalLayer*>(surface->layer);
      [layer setDevice:device_];
      [layer setDrawableSize:CGSizeMake(framebuffer_size.width,
                                        framebuffer_size.height)];
    }
    return {};
  }

  Result<std::unique_ptr<RenderFrame>> begin_frame() override {
    if (device_ == nil) {
      return std::unexpected(Error{
          .code = ErrorCode::renderer_initialization_failed,
          .message = "Metal renderer requires an MTLDevice",
      });
    }

    return std::make_unique<MetalFrame>();
  }

 private:
  RenderSurfaceDescriptor descriptor_;
  id<MTLDevice> device_ = nil;
};

} // namespace

Result<std::unique_ptr<Renderer>> create_renderer(
    const RenderSurfaceDescriptor& descriptor) {
  if (descriptor.framebuffer_size.width <= 0.0F ||
      descriptor.framebuffer_size.height <= 0.0F) {
    return std::unexpected(Error{
        .code = ErrorCode::renderer_initialization_failed,
        .message = "Metal renderer requires a non-empty framebuffer",
    });
  }

  id<MTLDevice> device = MTLCreateSystemDefaultDevice();
  if (device == nil) {
    return std::unexpected(Error{
        .code = ErrorCode::renderer_initialization_failed,
        .message = "MTLCreateSystemDefaultDevice failed",
    });
  }

  auto renderer = std::make_unique<MetalRenderer>(descriptor, device);
  [device release];
  return renderer;
}

} // namespace cgpui
