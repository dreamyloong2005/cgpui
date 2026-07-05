#include "cgpui/cgpui.hpp"

#include <expected>
#include <memory>
#include <optional>
#include <string_view>
#include <type_traits>
#include <utility>

namespace {

static_assert(std::is_same_v<
              decltype(std::declval<cgpui::WindowRuntime&>().try_draw_frame()),
              cgpui::Result<void>>);
static_assert(std::is_same_v<
              decltype(std::declval<cgpui::TestContextCapability&>()
                           .try_draw_frame()),
              cgpui::Result<void>>);

class FrameResultFrame final : public cgpui::RenderFrame {
 public:
  FrameResultFrame(
      int& draw_count,
      int& present_count,
      bool& fail_present)
      : draw_count_(draw_count),
        present_count_(present_count),
        fail_present_(fail_present) {}

  void clear(cgpui::Color) override {}
  void draw_rect(const cgpui::SolidRect&) override { draw_count_ += 1; }

  cgpui::Result<void> present() override {
    present_count_ += 1;
    if (fail_present_) {
      return std::unexpected(cgpui::Error{
          .code = cgpui::ErrorCode::frame_acquisition_failed,
          .message = "present failed"});
    }
    return {};
  }

 private:
  int& draw_count_;
  int& present_count_;
  bool& fail_present_;
};

class FrameResultRenderer final : public cgpui::Renderer {
 public:
  cgpui::Result<void> resize(cgpui::Size, cgpui::DpiScale) override {
    return {};
  }

  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    begin_frame_count += 1;
    if (fail_begin_frame) {
      return std::unexpected(cgpui::Error{
          .code = cgpui::ErrorCode::frame_acquisition_failed,
          .message = "begin frame failed"});
    }
    return std::make_unique<FrameResultFrame>(
        draw_count,
        present_count,
        fail_present);
  }

  int begin_frame_count = 0;
  int draw_count = 0;
  int present_count = 0;
  bool fail_begin_frame = false;
  bool fail_present = false;
};

class FrameResultView final : public cgpui::View {
 public:
  cgpui::AnyElement render(cgpui::ViewContext&) override {
    render_count += 1;
    return cgpui::into_element(cgpui::div().size(20.0F, 12.0F));
  }

  void paint(cgpui::PaintList& paint_list, cgpui::Size) override {
    paint_count += 1;
    paint_list.fill_rect(
        cgpui::Rect{.origin = {}, .size = {4.0F, 5.0F}},
        cgpui::Color{.r = 0.3F, .g = 0.4F, .b = 0.5F, .a = 1.0F});
  }

  int render_count = 0;
  int paint_count = 0;
};

class FrameResultWindow final : public cgpui::PlatformWindow {
 public:
  cgpui::NativeSurfaceHandle native_surface() const override { return {}; }

  cgpui::WindowState state() const override {
    return cgpui::WindowState{
        .framebuffer_size = {.width = 640.0F, .height = 480.0F},
        .scale = cgpui::DpiScale{1.0F},
        .close_requested = false};
  }

  void request_redraw() override {
    request_redraw_count += 1;
    if (callback) {
      callback(cgpui::WindowRedrawRequested{});
    }
  }

  void request_close() override {}
  void set_title(std::string_view) override {}
  void set_cursor(cgpui::CursorShape) override {}
  void set_ime_text_input_placement(
      std::optional<cgpui::ImeTextInputPlacement>) override {}

  cgpui::PlatformEventCallback callback;
  int request_redraw_count = 0;
};

class FrameResultApplication final : public cgpui::PlatformApplication {
 public:
  explicit FrameResultApplication(FrameResultWindow& window)
      : window_(window) {}

  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_window(
      const cgpui::WindowDescriptor&,
      cgpui::PlatformEventCallback callback) override {
    window_.callback = std::move(callback);
    return std::unique_ptr<cgpui::PlatformWindow>(
        new BorrowedWindow(window_));
  }

  int run() override {
    run_count += 1;
    if (on_run) {
      on_run();
    }
    return 0;
  }

  void quit() override { quit_count += 1; }

  int run_count = 0;
  int quit_count = 0;
  void (*on_run)() = nullptr;

 private:
  class BorrowedWindow final : public cgpui::PlatformWindow {
   public:
    explicit BorrowedWindow(FrameResultWindow& window) : window_(window) {}

    cgpui::NativeSurfaceHandle native_surface() const override {
      return window_.native_surface();
    }
    cgpui::WindowState state() const override { return window_.state(); }
    void request_redraw() override { window_.request_redraw(); }
    void request_close() override { window_.request_close(); }
    void set_title(std::string_view title) override {
      window_.set_title(title);
    }
    void set_cursor(cgpui::CursorShape cursor_shape) override {
      window_.set_cursor(cursor_shape);
    }
    void set_ime_text_input_placement(
        std::optional<cgpui::ImeTextInputPlacement> placement) override {
      window_.set_ime_text_input_placement(placement);
    }

   private:
    FrameResultWindow& window_;
  };

  FrameResultWindow& window_;
};

struct FrameResultFixture {
  FrameResultWindow window;
  FrameResultApplication app{window};
  FrameResultView view;
  FrameResultRenderer renderer;
};

FrameResultFixture* active_fixture = nullptr;
cgpui::WindowRuntime* active_runtime = nullptr;
cgpui::Result<void> captured_result{};
int captured_error_count = 0;
bool test_context_try_frame_observed = false;

void set_active(FrameResultFixture& fixture, cgpui::WindowRuntime& runtime) {
  active_fixture = &fixture;
  active_runtime = &runtime;
  captured_result = {};
  captured_error_count = 0;
  test_context_try_frame_observed = false;
}

void try_draw_success() {
  captured_result = active_runtime->try_draw_frame();
}

void try_draw_begin_failure() {
  active_fixture->renderer.fail_begin_frame = true;
  captured_result = active_runtime->try_draw_frame();
}

void try_draw_present_failure() {
  active_fixture->renderer.fail_present = true;
  captured_result = active_runtime->try_draw_frame();
}

void compatibility_redraw_begin_failure() {
  active_fixture->renderer.fail_begin_frame = true;
  active_fixture->window.request_redraw();
}

void dispatch_test_context_event() {
  active_fixture->window.callback(cgpui::KeyboardKey{
      .key_code = 'T',
      .action = cgpui::KeyAction::pressed});
}

int test_try_draw_frame_updates_frame_lifecycle() {
  FrameResultFixture fixture;
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<cgpui::Renderer*> { return &fixture.renderer; });
  set_active(fixture, runtime);
  fixture.app.on_run = &try_draw_success;

  const int run_result = runtime.run(
      cgpui::WindowDescriptor{},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  active_fixture = nullptr;
  active_runtime = nullptr;

  if (run_result != 0 || !captured_result.has_value()) {
    return 1;
  }
  const cgpui::RuntimeDiagnosticsSnapshot diagnostics =
      runtime.diagnostics_snapshot();
  if (fixture.window.request_redraw_count != 0 ||
      fixture.renderer.begin_frame_count != 1 ||
      fixture.renderer.draw_count != 1 ||
      fixture.renderer.present_count != 1 ||
      fixture.view.render_count != 1 ||
      fixture.view.paint_count != 1 ||
      diagnostics.frame_index != 1 ||
      fixture.app.quit_count != 0) {
    return 2;
  }
  return 0;
}

int test_try_draw_frame_returns_begin_error_without_quitting() {
  FrameResultFixture fixture;
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<cgpui::Renderer*> { return &fixture.renderer; });
  runtime.set_error_callback(
      [](const cgpui::Error&) { captured_error_count += 1; });
  set_active(fixture, runtime);
  fixture.app.on_run = &try_draw_begin_failure;

  const int run_result = runtime.run(
      cgpui::WindowDescriptor{},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  active_fixture = nullptr;
  active_runtime = nullptr;

  if (run_result != 0 || captured_result.has_value()) {
    return 3;
  }
  if (captured_result.error().code !=
          cgpui::ErrorCode::frame_acquisition_failed ||
      captured_result.error().message != "begin frame failed") {
    return 4;
  }
  if (fixture.app.quit_count != 0 ||
      captured_error_count != 0 ||
      fixture.renderer.begin_frame_count != 1 ||
      fixture.renderer.present_count != 0 ||
      runtime.diagnostics_snapshot().frame_index != 0) {
    return 5;
  }
  return 0;
}

int test_try_draw_frame_returns_present_error_without_quitting() {
  FrameResultFixture fixture;
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<cgpui::Renderer*> { return &fixture.renderer; });
  runtime.set_error_callback(
      [](const cgpui::Error&) { captured_error_count += 1; });
  set_active(fixture, runtime);
  fixture.app.on_run = &try_draw_present_failure;

  const int run_result = runtime.run(
      cgpui::WindowDescriptor{},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  active_fixture = nullptr;
  active_runtime = nullptr;

  if (run_result != 0 || captured_result.has_value()) {
    return 6;
  }
  if (captured_result.error().code !=
          cgpui::ErrorCode::frame_acquisition_failed ||
      captured_result.error().message != "present failed") {
    return 7;
  }
  if (fixture.app.quit_count != 0 ||
      captured_error_count != 0 ||
      fixture.renderer.begin_frame_count != 1 ||
      fixture.renderer.present_count != 1 ||
      runtime.diagnostics_snapshot().frame_index != 0) {
    return 8;
  }
  return 0;
}

int test_redraw_event_preserves_fail_and_quit_compatibility() {
  FrameResultFixture fixture;
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<cgpui::Renderer*> { return &fixture.renderer; });
  runtime.set_error_callback(
      [](const cgpui::Error&) { captured_error_count += 1; });
  set_active(fixture, runtime);
  fixture.app.on_run = &compatibility_redraw_begin_failure;

  const int run_result = runtime.run(
      cgpui::WindowDescriptor{},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  active_fixture = nullptr;
  active_runtime = nullptr;

  if (run_result == 0 ||
      fixture.app.quit_count != 1 ||
      captured_error_count != 1 ||
      fixture.renderer.begin_frame_count != 1) {
    return 9;
  }
  return 0;
}

int test_test_context_try_draw_frame_returns_renderer_error() {
  FrameResultFixture fixture;
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<cgpui::Renderer*> { return &fixture.renderer; });
  runtime.set_error_callback(
      [](const cgpui::Error&) { captured_error_count += 1; });
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord&) {
        fixture.renderer.fail_begin_frame = true;
        captured_result = context.test_context().try_draw_frame();
        test_context_try_frame_observed =
            !captured_result.has_value() &&
            captured_result.error().code ==
                cgpui::ErrorCode::frame_acquisition_failed;
      });
  set_active(fixture, runtime);
  fixture.app.on_run = &dispatch_test_context_event;

  const int run_result = runtime.run(
      cgpui::WindowDescriptor{},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  active_fixture = nullptr;
  active_runtime = nullptr;

  if (run_result != 0 ||
      !test_context_try_frame_observed ||
      fixture.app.quit_count != 0 ||
      captured_error_count != 0) {
    return 10;
  }
  return 0;
}

} // namespace

int main() {
  if (const int result = test_try_draw_frame_updates_frame_lifecycle();
      result != 0) {
    return result;
  }
  if (const int result =
          test_try_draw_frame_returns_begin_error_without_quitting();
      result != 0) {
    return result;
  }
  if (const int result =
          test_try_draw_frame_returns_present_error_without_quitting();
      result != 0) {
    return result;
  }
  if (const int result =
          test_redraw_event_preserves_fail_and_quit_compatibility();
      result != 0) {
    return result;
  }
  if (const int result =
          test_test_context_try_draw_frame_returns_renderer_error();
      result != 0) {
    return result;
  }
  return 0;
}
