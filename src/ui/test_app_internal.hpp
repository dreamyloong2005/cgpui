#pragma once

#include "cgpui/platform/clipboard.hpp"
#include "cgpui/platform/platform_application.hpp"
#include "cgpui/renderer/renderer.hpp"
#include "cgpui/ui/test_app.hpp"
#include "cgpui/ui/window_runtime.hpp"

#include <deque>
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
  PlatformMenuInstallationResult install_native_menu(
      NativeMenuModel menu) override;
  NativeFileDialogResult show_native_file_dialog(
      NativeFileDialogOptions options) override;
  NativeMessageDialogResult show_native_message_dialog(
      NativeMessageDialogOptions options) override;
  PlatformOpenUrlResult open_url(std::string url) override;
  PlatformReopenResult request_reopen() override;
  void simulate_path_prompt_response(
      std::optional<std::vector<std::string>> paths);
  void simulate_prompt_answer(NativeMessageDialogResponse response);
  [[nodiscard]] std::optional<std::string> opened_url() const;
  [[nodiscard]] TestPlatformServiceSnapshot service_snapshot() const;
  int run() override;
  void quit() override;

 private:
  std::deque<std::optional<std::vector<std::string>>> path_responses_;
  std::deque<NativeMessageDialogResponse> prompt_responses_;
  TestPlatformServiceSnapshot service_snapshot_;
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
  [[nodiscard]] bool write_to_clipboard(std::string_view text);
  [[nodiscard]] std::optional<std::string> read_from_clipboard();
  [[nodiscard]] std::optional<std::string> opened_url() const;
  void simulate_path_prompt_response(
      std::optional<std::vector<std::string>> paths);
  void simulate_prompt_answer(NativeMessageDialogResponse response);
  [[nodiscard]] TestPlatformServiceSnapshot service_snapshot() const;

  TestPlatformApplication application;
  TestRootView root_view;
  TestPlatformWindow parent_window;
  TestRenderer parent_renderer;
  std::vector<std::unique_ptr<Renderer>> renderers;
  MemoryClipboard clipboard;
  WindowRuntime runtime;
};

} // namespace cgpui::detail
