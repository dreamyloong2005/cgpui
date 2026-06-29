#pragma once

#include "cgpui/core/geometry.hpp"
#include "cgpui/platform/platform.hpp"
#include "cgpui/renderer/renderer.hpp"

#include <functional>
#include <span>
#include <vector>

namespace cgpui {

struct WindowRuntimeContext;

struct PaintCommand {
  SolidRect solid_rect;
};

class PaintList {
 public:
  void clear();
  void fill_rect(Rect rect, Color color);
  [[nodiscard]] std::span<const PaintCommand> commands() const;

 private:
  std::vector<PaintCommand> commands_;
};

class View {
 public:
  virtual ~View() = default;
  virtual void paint(PaintList& paint_list, Size viewport_size) = 0;
  virtual void handle_event(
      const PlatformEvent& event,
      const WindowRuntimeContext& context);
};

struct WindowRuntimeOptions {
  bool request_initial_redraw = true;
};

class WindowRuntime;

struct ViewInputState {
  bool focused = false;
  bool pointer_captured = false;
  Point pointer_position{};
};

struct WindowRuntimeContext {
  WindowRuntime& runtime;
  PlatformApplication& application;
  PlatformWindow& window;
  Renderer& renderer;
  Size viewport_size;
  ViewInputState input;
  int frame_index = 0;
};

using RendererFactory =
    std::function<Result<Renderer*>(const RenderSurfaceDescriptor&)>;
using WindowRuntimeFrameCallback =
    std::function<void(const WindowRuntimeContext&)>;
using WindowRuntimeErrorCallback =
    std::function<void(const Error&)>;

class WindowRuntime {
 public:
  WindowRuntime(
      PlatformApplication& application,
      View& view,
      RendererFactory renderer_factory);

  [[nodiscard]] int run(
      const WindowDescriptor& descriptor,
      WindowRuntimeOptions options = {});

  void set_after_frame_callback(WindowRuntimeFrameCallback callback);
  void set_close_requested_callback(WindowRuntimeFrameCallback callback);
  void set_error_callback(WindowRuntimeErrorCallback callback);
  void capture_pointer();
  void release_pointer();
  Result<void> resize_surface(Size size, DpiScale scale);

 private:
  void handle_event(const PlatformEvent& event);
  void handle_resize(const WindowResized& event);
  void handle_redraw();
  void fail_and_quit(Error error);
  [[nodiscard]] WindowRuntimeContext context();

  PlatformApplication& application_;
  View& view_;
  RendererFactory renderer_factory_;
  WindowRuntimeFrameCallback after_frame_callback_;
  WindowRuntimeFrameCallback close_requested_callback_;
  WindowRuntimeErrorCallback error_callback_;
  PlatformWindow* window_ = nullptr;
  Renderer* renderer_ = nullptr;
  Size viewport_size_{};
  ViewInputState input_{};
  int frame_index_ = 0;
  bool should_quit_ = false;
  bool failed_ = false;
};

Result<void> render_view(Renderer& renderer, View& view, Size viewport_size);

} // namespace cgpui
