#include "cgpui/cgpui.hpp"

#include <expected>
#include <memory>
#include <optional>
#include <string_view>
#include <type_traits>
#include <utility>

namespace {

static_assert(std::is_same_v<
              decltype(std::declval<cgpui::WindowRuntime&>().try_resize_surface(
                  std::declval<cgpui::Size>(),
                  std::declval<cgpui::DpiScale>())),
              cgpui::Result<void>>);

class ResizeResultFrame final : public cgpui::RenderFrame {
 public:
  void clear(cgpui::Color) override {}
  void draw_rect(const cgpui::SolidRect&) override {}
  cgpui::Result<void> present() override { return {}; }
};

class ResizeResultRenderer final : public cgpui::Renderer {
 public:
  cgpui::Result<void> resize(cgpui::Size size, cgpui::DpiScale scale) override {
    resize_count += 1;
    last_size = size;
    last_scale = scale;
    if (fail_resize) {
      return std::unexpected(cgpui::Error{
          .code = cgpui::ErrorCode::renderer_initialization_failed,
          .message = "resize failed"});
    }
    return {};
  }

  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    return std::make_unique<ResizeResultFrame>();
  }

  int resize_count = 0;
  bool fail_resize = false;
  cgpui::Size last_size{};
  cgpui::DpiScale last_scale{};
};

class ResizeResultView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}
};

class ResizeResultWindow final : public cgpui::PlatformWindow {
 public:
  cgpui::NativeSurfaceHandle native_surface() const override { return {}; }

  cgpui::WindowState state() const override {
    return cgpui::WindowState{
        .framebuffer_size = {.width = 640.0F, .height = 480.0F},
        .scale = cgpui::DpiScale{1.0F},
        .close_requested = false};
  }

  void request_redraw() override {}
  void request_close() override {}
  void set_title(std::string_view) override {}
  void set_cursor(cgpui::CursorShape) override {}
  void set_ime_text_input_placement(
      std::optional<cgpui::ImeTextInputPlacement>) override {}
};

class ResizeResultApplication final : public cgpui::PlatformApplication {
 public:
  explicit ResizeResultApplication(ResizeResultWindow& window)
      : window_(window) {}

  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_window(
      const cgpui::WindowDescriptor&,
      cgpui::PlatformEventCallback) override {
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
    explicit BorrowedWindow(ResizeResultWindow& window) : window_(window) {}

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
    ResizeResultWindow& window_;
  };

  ResizeResultWindow& window_;
};

struct ResizeResultFixture {
  ResizeResultWindow window;
  ResizeResultApplication app{window};
  ResizeResultView view;
  ResizeResultRenderer renderer;
};

ResizeResultFixture* active_fixture = nullptr;
cgpui::WindowRuntime* active_runtime = nullptr;
cgpui::Result<void> captured_result{};
int captured_error_count = 0;

void try_resize_success() {
  captured_result = active_runtime->try_resize_surface(
      cgpui::Size{320.0F, 240.0F},
      cgpui::DpiScale{2.0F});
}

void try_resize_failure() {
  active_fixture->renderer.fail_resize = true;
  captured_result = active_runtime->try_resize_surface(
      cgpui::Size{300.0F, 200.0F},
      cgpui::DpiScale{1.5F});
}

void compatibility_resize_failure() {
  active_fixture->renderer.fail_resize = true;
  captured_result = active_runtime->resize_surface(
      cgpui::Size{300.0F, 200.0F},
      cgpui::DpiScale{1.5F});
}

void set_active(ResizeResultFixture& fixture, cgpui::WindowRuntime& runtime) {
  active_fixture = &fixture;
  active_runtime = &runtime;
  captured_result = {};
  captured_error_count = 0;
}

int test_try_resize_surface_returns_renderer_resize_result() {
  ResizeResultFixture fixture;
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<cgpui::Renderer*> { return &fixture.renderer; });
  set_active(fixture, runtime);
  fixture.app.on_run = &try_resize_success;

  const int run_result = runtime.run(cgpui::WindowDescriptor{});
  active_fixture = nullptr;
  active_runtime = nullptr;

  if (run_result != 0 || !captured_result.has_value()) {
    return 1;
  }
  if (fixture.renderer.resize_count != 1 ||
      fixture.renderer.last_size.width != 320.0F ||
      fixture.renderer.last_size.height != 240.0F ||
      fixture.renderer.last_scale.value != 2.0F ||
      fixture.app.quit_count != 0) {
    return 2;
  }
  return 0;
}

int test_try_resize_surface_returns_error_without_quitting() {
  ResizeResultFixture fixture;
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<cgpui::Renderer*> { return &fixture.renderer; });
  runtime.set_error_callback(
      [](const cgpui::Error&) { captured_error_count += 1; });
  set_active(fixture, runtime);
  fixture.app.on_run = &try_resize_failure;

  const int run_result = runtime.run(cgpui::WindowDescriptor{});
  active_fixture = nullptr;
  active_runtime = nullptr;

  if (run_result != 0 || captured_result.has_value()) {
    return 3;
  }
  if (captured_result.error().code !=
          cgpui::ErrorCode::renderer_initialization_failed ||
      captured_result.error().message != "resize failed") {
    return 4;
  }
  if (fixture.renderer.resize_count != 1 || fixture.app.quit_count != 0 ||
      captured_error_count != 0) {
    return 5;
  }
  return 0;
}

int test_resize_surface_preserves_fail_and_quit_compatibility() {
  ResizeResultFixture fixture;
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<cgpui::Renderer*> { return &fixture.renderer; });
  runtime.set_error_callback(
      [](const cgpui::Error&) { captured_error_count += 1; });
  set_active(fixture, runtime);
  fixture.app.on_run = &compatibility_resize_failure;

  const int run_result = runtime.run(cgpui::WindowDescriptor{});
  active_fixture = nullptr;
  active_runtime = nullptr;

  if (run_result == 0 || captured_result.has_value()) {
    return 6;
  }
  if (fixture.renderer.resize_count != 1 || fixture.app.quit_count != 1 ||
      captured_error_count != 1) {
    return 7;
  }
  return 0;
}

} // namespace

int main() {
  if (const int result =
          test_try_resize_surface_returns_renderer_resize_result();
      result != 0) {
    return result;
  }
  if (const int result =
          test_try_resize_surface_returns_error_without_quitting();
      result != 0) {
    return result;
  }
  if (const int result =
          test_resize_surface_preserves_fail_and_quit_compatibility();
      result != 0) {
    return result;
  }
  return 0;
}
