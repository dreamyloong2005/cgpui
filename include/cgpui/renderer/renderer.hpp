#pragma once

#include "cgpui/core/error.hpp"
#include "cgpui/core/geometry.hpp"
#include "cgpui/platform/native_surface.hpp"

#include <memory>

namespace cgpui {

struct RenderSurfaceDescriptor {
  NativeSurfaceHandle native_surface;
  Size framebuffer_size;
  DpiScale scale;
};

struct SolidRect {
  Rect rect;
  Color color;
};

class RenderFrame {
 public:
  virtual ~RenderFrame() = default;
  virtual void clear(Color color) = 0;
  virtual void draw_rect(const SolidRect& rect) = 0;
  virtual Result<void> present() = 0;
};

class Renderer {
 public:
  virtual ~Renderer() = default;

  virtual Result<void> resize(Size framebuffer_size, DpiScale scale) = 0;
  virtual Result<std::unique_ptr<RenderFrame>> begin_frame() = 0;
};

Result<std::unique_ptr<Renderer>> create_renderer(
    const RenderSurfaceDescriptor& descriptor);

} // namespace cgpui
