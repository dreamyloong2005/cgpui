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

class TestView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList& paint_list, cgpui::Size) override {
    paint_count += 1;
    paint_list.fill_rect(
        cgpui::Rect{.origin = {}, .size = {.width = 8.0F, .height = 6.0F}},
        cgpui::rgb(30, 50, 70));
  }

  int paint_count = 0;
};

struct AppContextCapabilityGlobal {
  int value = 0;
};

class FakeWindow final : public cgpui::PlatformWindow {
 public:
  explicit FakeWindow(cgpui::WindowState state) : state_(state) {}

  [[nodiscard]] cgpui::NativeSurfaceHandle native_surface() const override {
    return {};
  }

  [[nodiscard]] cgpui::WindowState state() const override { return state_; }

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

 private:
  cgpui::WindowState state_;
};

class FakeApplication final : public cgpui::PlatformApplication {
 public:
  explicit FakeApplication(FakeWindow& window) : window_(window) {}

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

int test_app_and_window_context_facades() {
  FakeWindow window(cgpui::WindowState{
      .framebuffer_size = {.width = 320.0F, .height = 240.0F},
      .scale = cgpui::DpiScale{2.0F},
      .close_requested = false});
  FakeApplication application(window);
  TestView view;
  RecordingFrame frame;
  int renderer_begin_frame_count = 0;
  bool setup_called = false;
  bool frame_called = false;
  bool child_window_available = false;
  bool child_window_active = false;
  cgpui::AppOpenedWindow opened;
  cgpui::WindowRuntimeId root_window_id;
  cgpui::WindowDescriptor root_descriptor;
  cgpui::Size root_viewport_size;
  cgpui::DpiScale root_scale;
  cgpui::WindowDescriptor child_descriptor;
  bool app_context_global_visible = false;
  cgpui::WindowRuntimeId app_context_root_window_id;

  const int result = cgpui::run_app(
      application,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<std::unique_ptr<cgpui::Renderer>> {
        return std::make_unique<RecordingRenderer>(
            frame,
            renderer_begin_frame_count);
      },
      cgpui::AppRunnerOptions{
          .window =
              {
                  .title = "Root Facade",
                  .size = {.width = 500.0F, .height = 400.0F},
              },
          .runtime = {.request_initial_redraw = false},
          .setup_context =
              [&](cgpui::AppContext& context) {
                setup_called = true;
                context.set_global(AppContextCapabilityGlobal{.value = 5});
                cgpui::App app = context.app();
                opened = app.open_window(cgpui::WindowOptions{}
                                             .title("Child Facade")
                                             .size(240.0F, 120.0F));
                context.runtime.set_after_frame_callback(
                    [&](const cgpui::ViewContext& frame_context) {
                      frame_called = true;
                      cgpui::App frame_app = frame_context.app();
                      cgpui::AppContext app_context =
                          frame_context.app_context();
                      cgpui::Window root_window =
                          frame_context.current_window();
                      root_window_id = root_window.runtime_id();
                      root_descriptor = root_window.descriptor();
                      root_viewport_size = root_window.viewport_size();
                      root_scale = root_window.scale();
                      app_context_root_window_id =
                          app_context.app().root_window().runtime_id();

                      const AppContextCapabilityGlobal* global =
                          app_context.global<AppContextCapabilityGlobal>();
                      app_context_global_visible =
                          global != nullptr && global->value == 5;

                      const std::optional<cgpui::Window> child_window =
                          frame_app.window(opened.runtime_id);
                      child_window_available = child_window.has_value();
                      if (child_window_available) {
                        child_window_active = child_window->active();
                        child_descriptor = child_window->descriptor();
                      }
                    });
              },
      });

  if (result != 0) {
    return 1;
  }
  if (!setup_called || !frame_called) {
    return 2;
  }
  if (opened.runtime_id.value == 0 || root_window_id.value == 0 ||
      opened.runtime_id == root_window_id) {
    return 3;
  }
  if (app_context_root_window_id != root_window_id ||
      !app_context_global_visible) {
    return 8;
  }
  if (root_descriptor.title != "Root Facade" ||
      root_descriptor.size.width != 500.0F ||
      root_descriptor.size.height != 400.0F) {
    return 4;
  }
  if (root_viewport_size.width != 160.0F ||
      root_viewport_size.height != 120.0F ||
      root_scale.value != 2.0F) {
    return 5;
  }
  if (!child_window_available || !child_window_active ||
      child_descriptor.title != "Child Facade" ||
      child_descriptor.size.width != 240.0F ||
      child_descriptor.size.height != 120.0F) {
    return 6;
  }
  if (application.create_window_count != 2 ||
      application.run_count != 1 ||
      renderer_begin_frame_count != 1 ||
      frame.present_count != 1 ||
      view.paint_count != 1) {
    return 7;
  }
  return 0;
}

} // namespace

int main() {
  if (const int result = test_app_and_window_context_facades(); result != 0) {
    return result;
  }
  return 0;
}
