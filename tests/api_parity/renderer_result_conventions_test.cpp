#include "cgpui/cgpui.hpp"

#include <expected>
#include <memory>
#include <string_view>
#include <type_traits>
#include <utility>

namespace {

static_assert(std::is_same_v<
              decltype(std::declval<cgpui::WindowRuntime&>().try_create_renderer(
                  std::declval<cgpui::RenderSurfaceDescriptor>())),
              cgpui::Result<cgpui::Renderer*>>);

class RendererResultFrame final : public cgpui::RenderFrame {
 public:
  void clear(cgpui::Color) override {}
  void draw_rect(const cgpui::SolidRect&) override {}
  cgpui::Result<void> present() override { return {}; }
};

class RendererResultRenderer final : public cgpui::Renderer {
 public:
  cgpui::Result<void> resize(cgpui::Size, cgpui::DpiScale) override {
    return {};
  }

  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    return std::make_unique<RendererResultFrame>();
  }
};

class RendererResultView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}
};

class RendererResultWindow final : public cgpui::PlatformWindow {
 public:
  cgpui::NativeSurfaceHandle native_surface() const override { return {}; }

  cgpui::WindowState state() const override {
    return cgpui::WindowState{
        .framebuffer_size = {.width = 640.0F, .height = 480.0F},
        .scale = cgpui::DpiScale{2.0F},
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

class RendererResultApplication final : public cgpui::PlatformApplication {
 public:
  explicit RendererResultApplication(RendererResultWindow& window)
      : window_(window) {}

  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_window(
      const cgpui::WindowDescriptor& descriptor,
      cgpui::PlatformEventCallback callback) override {
    create_window_count += 1;
    last_descriptor = descriptor;
    window_callback = std::move(callback);
    return std::unique_ptr<cgpui::PlatformWindow>(
        new BorrowedWindow(window_));
  }

  int run() override { return 0; }
  void quit() override {}

  int create_window_count = 0;
  cgpui::WindowDescriptor last_descriptor{};
  cgpui::PlatformEventCallback window_callback;

 private:
  class BorrowedWindow final : public cgpui::PlatformWindow {
   public:
    explicit BorrowedWindow(RendererResultWindow& window)
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
    RendererResultWindow& window_;
  };

  RendererResultWindow& window_;
};

struct RendererResultFixture {
  RendererResultWindow window;
  RendererResultApplication app{window};
  RendererResultView view;
  RendererResultRenderer renderer;
};

cgpui::RenderSurfaceDescriptor descriptor() {
  return cgpui::RenderSurfaceDescriptor{
      .native_surface = {},
      .framebuffer_size = {.width = 123.0F, .height = 456.0F},
      .scale = cgpui::DpiScale{1.5F}};
}

int test_try_create_renderer_returns_renderer_pointer() {
  RendererResultFixture fixture;
  int factory_count = 0;
  cgpui::RenderSurfaceDescriptor captured{};
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor& surface)
          -> cgpui::Result<cgpui::Renderer*> {
        factory_count += 1;
        captured = surface;
        return &fixture.renderer;
      });

  cgpui::Result<cgpui::Renderer*> result =
      runtime.try_create_renderer(descriptor());

  if (!result.has_value() || *result != &fixture.renderer) {
    return 1;
  }
  if (factory_count != 1 ||
      captured.framebuffer_size.width != 123.0F ||
      captured.framebuffer_size.height != 456.0F ||
      captured.scale.value != 1.5F) {
    return 2;
  }
  return 0;
}

int test_try_create_renderer_propagates_factory_error() {
  RendererResultFixture fixture;
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<cgpui::Renderer*> {
        return std::unexpected(cgpui::Error{
            .code = cgpui::ErrorCode::renderer_initialization_failed,
            .message = "factory failed"});
      });

  cgpui::Result<cgpui::Renderer*> result =
      runtime.try_create_renderer(descriptor());

  if (result.has_value()) {
    return 3;
  }
  if (result.error().code !=
          cgpui::ErrorCode::renderer_initialization_failed ||
      result.error().message != "factory failed") {
    return 4;
  }
  return 0;
}

int test_try_create_renderer_rejects_empty_renderer() {
  RendererResultFixture fixture;
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<cgpui::Renderer*> { return nullptr; });

  cgpui::Result<cgpui::Renderer*> result =
      runtime.try_create_renderer(descriptor());

  if (result.has_value()) {
    return 5;
  }
  if (result.error().code !=
          cgpui::ErrorCode::renderer_initialization_failed ||
      result.error().message !=
          "Renderer factory returned an empty renderer") {
    return 6;
  }
  return 0;
}

int test_try_create_renderer_rejects_missing_factory() {
  RendererResultFixture fixture;
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      cgpui::RendererFactory{});

  cgpui::Result<cgpui::Renderer*> result =
      runtime.try_create_renderer(descriptor());

  if (result.has_value()) {
    return 7;
  }
  if (result.error().code !=
          cgpui::ErrorCode::renderer_initialization_failed ||
      result.error().message != "Window runtime requires a renderer factory") {
    return 8;
  }
  return 0;
}

int test_try_open_window_reuses_renderer_result_conventions() {
  RendererResultFixture fixture;
  int factory_count = 0;
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<cgpui::Renderer*> {
        factory_count += 1;
        return nullptr;
      });
  const std::size_t record_count_before =
      runtime.window_runtime_records().size();

  cgpui::Result<cgpui::AppOpenedWindow> opened =
      runtime.try_open_window(cgpui::WindowOptions{}.title("No Renderer"));

  if (opened.has_value()) {
    return 9;
  }
  if (opened.error().code !=
          cgpui::ErrorCode::renderer_initialization_failed ||
      opened.error().message !=
          "Renderer factory returned an empty child renderer") {
    return 10;
  }
  if (fixture.app.create_window_count != 1 || factory_count != 1 ||
      !runtime.app_opened_windows().empty() ||
      runtime.window_runtime_records().size() != record_count_before) {
    return 11;
  }
  return 0;
}

} // namespace

int main() {
  if (const int result = test_try_create_renderer_returns_renderer_pointer();
      result != 0) {
    return result;
  }
  if (const int result = test_try_create_renderer_propagates_factory_error();
      result != 0) {
    return result;
  }
  if (const int result = test_try_create_renderer_rejects_empty_renderer();
      result != 0) {
    return result;
  }
  if (const int result = test_try_create_renderer_rejects_missing_factory();
      result != 0) {
    return result;
  }
  if (const int result =
          test_try_open_window_reuses_renderer_result_conventions();
      result != 0) {
    return result;
  }
  return 0;
}
