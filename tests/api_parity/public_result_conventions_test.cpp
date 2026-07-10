#include "cgpui/cgpui.hpp"

#include <cstddef>
#include <expected>
#include <functional>
#include <memory>
#include <string_view>
#include <type_traits>
#include <utility>

namespace {

static_assert(std::is_same_v<
              decltype(std::declval<cgpui::WindowRuntime&>().try_open_window(
                  std::declval<cgpui::WindowOptions>())),
              cgpui::Result<cgpui::AppOpenedWindow>>);
static_assert(std::is_same_v<
              decltype(std::declval<cgpui::App&>().try_open_window(
                  std::declval<cgpui::WindowOptions>())),
              cgpui::Result<cgpui::AppOpenedWindow>>);
static_assert(std::is_same_v<
              decltype(std::declval<cgpui::AppContext&>().try_open_window(
                  std::declval<cgpui::WindowOptions>())),
              cgpui::Result<cgpui::AppOpenedWindow>>);

class ResultConventionFrame final : public cgpui::RenderFrame {
 public:
  void clear(cgpui::Color) override {}
  void draw_rect(const cgpui::SolidRect&) override {}
  cgpui::Result<void> present() override { return {}; }
};

class ResultConventionRenderer final : public cgpui::Renderer {
 public:
  explicit ResultConventionRenderer(int& begin_frame_count)
      : begin_frame_count_(begin_frame_count) {}

  cgpui::Result<void> resize(cgpui::Size, cgpui::DpiScale) override {
    return {};
  }

  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    begin_frame_count_ += 1;
    return std::make_unique<ResultConventionFrame>();
  }

 private:
  int& begin_frame_count_;
};

class ResultConventionView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}
};

class ResultConventionWindow final : public cgpui::PlatformWindow {
 public:
  cgpui::NativeSurfaceHandle native_surface() const override { return {}; }

  cgpui::WindowState state() const override {
    return cgpui::WindowState{
        .framebuffer_size = {.width = 320.0F, .height = 240.0F},
        .scale = cgpui::DpiScale{1.0F},
        .close_requested = false};
  }

  void request_redraw() override {}
  void request_close() override {}
  void set_title(std::string_view title) override { last_title = title; }
  void set_cursor(cgpui::CursorShape cursor_shape) override {
    last_cursor_shape = cursor_shape;
  }
  void set_ime_text_input_placement(
      std::optional<cgpui::ImeTextInputPlacement>) override {}

  std::string_view last_title;
  cgpui::CursorShape last_cursor_shape = cgpui::CursorShape::default_arrow;
};

class ResultConventionApplication final : public cgpui::PlatformApplication {
 public:
  explicit ResultConventionApplication(ResultConventionWindow& window)
      : window_(window) {}

  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_window(
      const cgpui::WindowDescriptor& descriptor,
      cgpui::PlatformEventCallback callback) override {
    create_window_count += 1;
    last_descriptor = descriptor;
    if (fail_create_window) {
      return std::unexpected(cgpui::Error{
          .code = cgpui::ErrorCode::window_creation_failed,
          .message = "test window creation failure"});
    }
    window_callback = std::move(callback);
    return std::unique_ptr<cgpui::PlatformWindow>(
        new BorrowedWindow(window_));
  }

  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_child_window(
      const cgpui::WindowDescriptor& descriptor,
      cgpui::PlatformWindow&,
      cgpui::PlatformEventCallback callback) override {
    create_child_window_count += 1;
    if (fail_create_child_window) {
      return std::unexpected(cgpui::Error{
          .code = cgpui::ErrorCode::window_creation_failed,
          .message = "test window creation failure"});
    }
    return create_window(descriptor, std::move(callback));
  }

  int run() override {
    if (on_run) {
      on_run();
    }
    return 0;
  }
  void quit() override {}

  bool fail_create_window = false;
  bool fail_create_child_window = false;
  int create_window_count = 0;
  int create_child_window_count = 0;
  cgpui::WindowDescriptor last_descriptor{};
  cgpui::PlatformEventCallback window_callback;
  std::function<void()> on_run;

 private:
  class BorrowedWindow final : public cgpui::PlatformWindow {
   public:
    explicit BorrowedWindow(ResultConventionWindow& window)
        : window_(window) {}

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
    ResultConventionWindow& window_;
  };

  ResultConventionWindow& window_;
};

int test_try_open_window_returns_platform_error() {
  ResultConventionWindow window;
  ResultConventionApplication app(window);
  app.fail_create_child_window = true;
  ResultConventionView view;
  int begin_frame_count = 0;
  int renderer_factory_count = 0;
  ResultConventionRenderer renderer(begin_frame_count);
  cgpui::WindowRuntime runtime(
      app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<cgpui::Renderer*> {
        renderer_factory_count += 1;
        return &renderer;
      });
  const std::size_t record_count_before = runtime.window_runtime_records().size();

  std::optional<cgpui::Result<cgpui::AppOpenedWindow>> opened;
  app.on_run = [&] {
    opened = runtime.try_open_window(
        cgpui::WindowOptions{}.title("Fails after root"));
  };
  const int run_result = runtime.run(
      cgpui::WindowDescriptor{},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});

  if (run_result != 0 || !opened.has_value() || opened->has_value()) {
    return 1;
  }
  if (opened->error().code != cgpui::ErrorCode::window_creation_failed ||
      opened->error().message != "test window creation failure") {
    return 2;
  }
  if (app.create_window_count != 1 || app.create_child_window_count != 1 ||
      renderer_factory_count != 1 ||
      begin_frame_count != 0) {
    return 3;
  }
  if (!runtime.app_opened_windows().empty() ||
      runtime.window_runtime_records().size() != record_count_before) {
    return 11;
  }
  return 0;
}

int test_try_open_window_returns_renderer_error() {
  ResultConventionWindow window;
  ResultConventionApplication app(window);
  ResultConventionView view;
  int begin_frame_count = 0;
  int renderer_factory_count = 0;
  ResultConventionRenderer root_renderer(begin_frame_count);
  cgpui::WindowRuntime runtime(
      app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<cgpui::Renderer*> {
        renderer_factory_count += 1;
        if (renderer_factory_count == 1) {
          return &root_renderer;
        }
        return std::unexpected(cgpui::Error{
            .code = cgpui::ErrorCode::renderer_initialization_failed,
            .message = "test renderer failure"});
      });
  const std::size_t record_count_before = runtime.window_runtime_records().size();

  std::optional<cgpui::Result<cgpui::AppOpenedWindow>> opened;
  app.on_run = [&] {
    opened = runtime.try_open_window(
        cgpui::WindowOptions{}.title("Fails renderer"));
  };
  const int run_result = runtime.run(
      cgpui::WindowDescriptor{},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});

  if (run_result != 0 || !opened.has_value() || opened->has_value()) {
    return 4;
  }
  if (opened->error().code !=
          cgpui::ErrorCode::renderer_initialization_failed ||
      opened->error().message != "test renderer failure") {
    return 5;
  }
  if (app.create_window_count != 2 || app.create_child_window_count != 1 ||
      renderer_factory_count != 2) {
    return 6;
  }
  if (!runtime.app_opened_windows().empty() ||
      runtime.window_runtime_records().size() != record_count_before) {
    return 12;
  }
  return 0;
}

int test_app_facades_forward_try_open_window_results() {
  ResultConventionWindow window;
  ResultConventionApplication app(window);
  ResultConventionView view;
  int begin_frame_count = 0;
  int renderer_factory_count = 0;
  ResultConventionRenderer renderer(begin_frame_count);
  cgpui::WindowRuntime runtime(
      app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<cgpui::Renderer*> {
        renderer_factory_count += 1;
        return &renderer;
      });

  cgpui::App public_app(runtime);
  cgpui::AppContext app_context{.runtime = runtime};

  cgpui::Result<cgpui::AppOpenedWindow> from_app =
      public_app.try_open_window(
          cgpui::WindowOptions{}.title("Result App").size(200.0F, 100.0F));
  cgpui::Result<cgpui::AppOpenedWindow> from_context =
      app_context.try_open_window(
          cgpui::WindowOptions{}.title("Result Context"),
          std::make_unique<ResultConventionView>());

  if (!from_app.has_value() || !from_context.has_value()) {
    return 7;
  }
  if (from_app->runtime_id.value == 0 ||
      from_context->runtime_id.value == 0 ||
      from_app->runtime_id == from_context->runtime_id ||
      from_context->root_view_id.value == 0) {
    return 8;
  }
  if (from_app->descriptor.title != "Result App" ||
      from_app->descriptor.size.width != 200.0F ||
      from_app->descriptor.size.height != 100.0F ||
      from_context->descriptor.title != "Result Context") {
    return 9;
  }
  if (app.create_window_count != 0 || app.create_child_window_count != 0 ||
      renderer_factory_count != 0 ||
      begin_frame_count != 0) {
    return 10;
  }
  return 0;
}

} // namespace

int main() {
  if (const int result = test_try_open_window_returns_platform_error();
      result != 0) {
    return result;
  }
  if (const int result = test_try_open_window_returns_renderer_error();
      result != 0) {
    return result;
  }
  if (const int result = test_app_facades_forward_try_open_window_results();
      result != 0) {
    return result;
  }
  return 0;
}
