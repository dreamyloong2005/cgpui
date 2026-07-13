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
  Size framebuffer_size_;
  DpiScale scale_{1.0F};
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
  [[nodiscard]] std::uint64_t monotonic_time_ms() const override;
  int run() override;
  void quit() override;
};

class TestRenderFrame final : public RenderFrame {
 public:
  explicit TestRenderFrame(TestWindowRenderSnapshot& snapshot);
  void clear(Color color) override;
  void draw_rect(const SolidRect& rect) override;
  Result<void> present() override;

 private:
  TestWindowRenderSnapshot* snapshot_ = nullptr;
};

class TestRenderer final : public Renderer {
 public:
  Result<void> resize(Size framebuffer_size, DpiScale scale) override;
  Result<std::unique_ptr<RenderFrame>> begin_frame() override;
  [[nodiscard]] TestWindowRenderSnapshot snapshot() const;

 private:
  TestWindowRenderSnapshot snapshot_;
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
  void advance_time(std::uint64_t delta_ms);
  void run_until_parked();
  [[nodiscard]] bool cancel_timer(TimerId id);
  [[nodiscard]] bool complete_task(TaskId id);
  void drain_task_completions();
  void request_redraw(WindowRuntimeId runtime_id);
  [[nodiscard]] Result<void> try_draw_frame(WindowRuntimeId runtime_id);
  [[nodiscard]] TestWindowRenderSnapshot render_snapshot(
      WindowRuntimeId runtime_id) const;

  TestPlatformApplication application;
  TestRootView root_view;
  TestPlatformWindow parent_window;
  TestRenderer parent_renderer;
  std::vector<std::unique_ptr<Renderer>> renderers;
  WindowRuntime runtime;
};

} // namespace cgpui::detail
