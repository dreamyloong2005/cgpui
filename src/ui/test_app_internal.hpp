#pragma once

#include "cgpui/platform/platform_application.hpp"
#include "cgpui/renderer/renderer.hpp"
#include "cgpui/ui/test_app.hpp"
#include "cgpui/ui/window_runtime.hpp"

#include <memory>
#include <vector>

namespace cgpui::detail {

class TestPlatformWindow final : public PlatformWindow {
 public:
  TestPlatformWindow(
      WindowDescriptor descriptor,
      PlatformEventCallback callback);

  [[nodiscard]] NativeSurfaceHandle native_surface() const override;
  [[nodiscard]] WindowState state() const override;
  void request_redraw() override;
  void request_close() override;
  void set_title(std::string_view title) override;
  void set_cursor(CursorShape cursor_shape) override;
  void set_ime_text_input_placement(
      std::optional<ImeTextInputPlacement> placement) override;
  void dispatch_event(const PlatformEvent& event);

 private:
  WindowDescriptor descriptor_;
  PlatformEventCallback callback_;
};

class TestPlatformApplication final : public PlatformApplication {
 public:
  Result<std::unique_ptr<PlatformWindow>> create_window(
      const WindowDescriptor& descriptor,
      PlatformEventCallback callback) override;
  Result<std::unique_ptr<PlatformWindow>> create_child_window(
      const WindowDescriptor& descriptor,
      PlatformWindow& parent,
      PlatformEventCallback callback) override;
  int run() override;
  void quit() override;
};

class TestRenderFrame final : public RenderFrame {
 public:
  void clear(Color color) override;
  void draw_rect(const SolidRect& rect) override;
  Result<void> present() override;
};

class TestRenderer final : public Renderer {
 public:
  Result<void> resize(Size framebuffer_size, DpiScale scale) override;
  Result<std::unique_ptr<RenderFrame>> begin_frame() override;
};

class TestRootView final : public View {
 public:
  void paint(PaintList& paint_list, Size viewport_size) override;
};

struct TestAppState {
  TestAppState();
  [[nodiscard]] bool dispatch_event(
      WindowRuntimeId runtime_id,
      const PlatformEvent& event);
  void focus(WindowRuntimeId runtime_id, ElementId element_id);
  void release_focus(WindowRuntimeId runtime_id, ElementId element_id);

  TestPlatformApplication application;
  TestRootView root_view;
  TestPlatformWindow parent_window;
  TestRenderer parent_renderer;
  std::vector<std::unique_ptr<Renderer>> renderers;
  WindowRuntime runtime;
};

} // namespace cgpui::detail
