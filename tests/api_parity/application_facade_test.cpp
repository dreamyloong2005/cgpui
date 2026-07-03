#include "cgpui/cgpui.hpp"

#include <expected>
#include <memory>
#include <optional>
#include <string_view>

namespace {

class RecordingFrame final : public cgpui::RenderFrame {
 public:
  void clear(cgpui::Color) override { clear_count += 1; }
  void draw_rect(const cgpui::SolidRect&) override { draw_count += 1; }
  cgpui::Result<void> present() override {
    present_count += 1;
    return {};
  }

  int clear_count = 0;
  int draw_count = 0;
  int present_count = 0;
};

class RecordingRenderer final : public cgpui::Renderer {
 public:
  RecordingRenderer(RecordingFrame& frame, int& begin_frame_count)
      : frame_(frame), begin_frame_count_(begin_frame_count) {}

  cgpui::Result<void> resize(cgpui::Size, cgpui::DpiScale) override {
    resize_count += 1;
    return {};
  }

  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    begin_frame_count_ += 1;
    return std::unique_ptr<cgpui::RenderFrame>(new BorrowedFrame(frame_));
  }

  int resize_count = 0;

 private:
  class BorrowedFrame final : public cgpui::RenderFrame {
   public:
    explicit BorrowedFrame(RecordingFrame& frame) : frame_(frame) {}

    void clear(cgpui::Color color) override { frame_.clear(color); }
    void draw_rect(const cgpui::SolidRect& rect) override {
      frame_.draw_rect(rect);
    }
    cgpui::Result<void> present() override { return frame_.present(); }

   private:
    RecordingFrame& frame_;
  };

  RecordingFrame& frame_;
  int& begin_frame_count_;
};

class ApplicationView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList& paint_list, cgpui::Size) override {
    paint_count += 1;
    paint_list.fill_rect(
        cgpui::Rect{.origin = {}, .size = {.width = 8.0F, .height = 6.0F}},
        cgpui::rgb(20, 40, 60));
  }

  int paint_count = 0;
};

class FakeWindow final : public cgpui::PlatformWindow {
 public:
  [[nodiscard]] cgpui::NativeSurfaceHandle native_surface() const override {
    return {};
  }

  [[nodiscard]] cgpui::WindowState state() const override {
    return cgpui::WindowState{
        .framebuffer_size = {.width = 320.0F, .height = 240.0F},
        .scale = cgpui::DpiScale{1.0F},
        .close_requested = false,
    };
  }

  void request_redraw() override {
    request_redraw_count += 1;
    if (callback) {
      callback(cgpui::WindowRedrawRequested{});
    }
  }

  void request_close() override {}
  void set_title(std::string_view title) override { last_title = title; }
  void set_cursor(cgpui::CursorShape cursor_shape) override {
    last_cursor_shape = cursor_shape;
  }
  void set_ime_text_input_placement(
      std::optional<cgpui::ImeTextInputPlacement>) override {}

  cgpui::PlatformEventCallback callback;
  int request_redraw_count = 0;
  std::string_view last_title;
  cgpui::CursorShape last_cursor_shape = cgpui::CursorShape::default_arrow;
};

class FakePlatformApplication final : public cgpui::PlatformApplication {
 public:
  explicit FakePlatformApplication(FakeWindow& window) : window_(window) {}

  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_window(
      const cgpui::WindowDescriptor& descriptor,
      cgpui::PlatformEventCallback callback) override {
    create_window_count += 1;
    last_descriptor = descriptor;
    window_.callback = std::move(callback);
    return std::unique_ptr<cgpui::PlatformWindow>(new BorrowedWindow(window_));
  }

  int run() override {
    run_count += 1;
    if (window_.callback) {
      window_.callback(cgpui::WindowRedrawRequested{});
    }
    return run_result;
  }

  void quit() override { quit_count += 1; }

  int create_window_count = 0;
  int run_count = 0;
  int quit_count = 0;
  int run_result = 0;
  cgpui::WindowDescriptor last_descriptor;

 private:
  class BorrowedWindow final : public cgpui::PlatformWindow {
   public:
    explicit BorrowedWindow(FakeWindow& window) : window_(window) {}

    [[nodiscard]] cgpui::NativeSurfaceHandle native_surface()
        const override {
      return window_.native_surface();
    }
    [[nodiscard]] cgpui::WindowState state() const override {
      return window_.state();
    }
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
    FakeWindow& window_;
  };

  FakeWindow& window_;
};

int test_application_facade_runs_platform_application() {
  FakeWindow window;
  auto platform = std::make_unique<FakePlatformApplication>(window);
  FakePlatformApplication* platform_ptr = platform.get();
  cgpui::Application application(std::move(platform));

  ApplicationView view;
  RecordingFrame frame;
  int renderer_begin_frame_count = 0;
  int renderer_factory_count = 0;
  bool context_setup_called = false;
  bool frame_callback_called = false;

  const int result = application.run(
      view,
      [&](const cgpui::RenderSurfaceDescriptor& descriptor)
          -> cgpui::Result<std::unique_ptr<cgpui::Renderer>> {
        renderer_factory_count += 1;
        if (descriptor.framebuffer_size.width != 320.0F ||
            descriptor.framebuffer_size.height != 240.0F ||
            descriptor.scale.value != 1.0F) {
          return std::unexpected(cgpui::Error{
              .code = cgpui::ErrorCode::renderer_initialization_failed,
              .message = "unexpected Application renderer descriptor"});
        }
        return std::make_unique<RecordingRenderer>(
            frame,
            renderer_begin_frame_count);
      },
      cgpui::AppRunnerOptions{
          .window =
              {
                  .title = "Application Facade",
                  .size = {.width = 500.0F, .height = 400.0F},
              },
          .runtime = {.request_initial_redraw = false},
          .setup_context =
              [&](cgpui::AppContext& context) {
                context_setup_called = true;
                context.runtime.set_after_frame_callback(
                    [&](const cgpui::ViewContext&) {
                      frame_callback_called = true;
                    });
              },
      });

  if (result != 0) {
    return 1;
  }
  if (&application.platform_application() != platform_ptr) {
    return 2;
  }
  if (platform_ptr->create_window_count != 1 || platform_ptr->run_count != 1) {
    return 3;
  }
  if (platform_ptr->last_descriptor.title != "Application Facade" ||
      platform_ptr->last_descriptor.size.width != 500.0F ||
      platform_ptr->last_descriptor.size.height != 400.0F) {
    return 4;
  }
  if (renderer_factory_count != 1 || renderer_begin_frame_count != 1 ||
      frame.present_count != 1 ||
      view.paint_count != 1) {
    return 5;
  }
  if (!context_setup_called || !frame_callback_called) {
    return 6;
  }
  return 0;
}

int test_application_create_api_is_available_without_running_native_app() {
  auto create = &cgpui::Application::create;
  (void)create;
  return 0;
}

} // namespace

int main() {
  if (const int result = test_application_facade_runs_platform_application();
      result != 0) {
    return result;
  }
  if (const int result =
          test_application_create_api_is_available_without_running_native_app();
      result != 0) {
    return result;
  }
  return 0;
}
