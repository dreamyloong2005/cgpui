#include "cgpui/platform/platform.hpp"
#include "cgpui/ui/ui.hpp"

#include <expected>
#include <memory>
#include <string_view>
#include <utility>

namespace {

bool equal(cgpui::Size lhs, cgpui::Size rhs) {
  return lhs.width == rhs.width && lhs.height == rhs.height;
}

bool equal(cgpui::DpiScale lhs, cgpui::DpiScale rhs) {
  return lhs.value == rhs.value;
}

class RecordingFrame final : public cgpui::RenderFrame {
 public:
  void clear(cgpui::Color color) override {
    clear_count += 1;
    last_clear = color;
  }

  void draw_rect(const cgpui::SolidRect& rect) override {
    draw_count += 1;
    last_rect = rect;
  }

  cgpui::Result<void> present() override {
    present_count += 1;
    return {};
  }

  int clear_count = 0;
  int draw_count = 0;
  int present_count = 0;
  cgpui::Color last_clear{};
  cgpui::SolidRect last_rect{};
};

class RecordingRenderer final : public cgpui::Renderer {
 public:
  explicit RecordingRenderer(RecordingFrame& frame) : frame_(frame) {}

  cgpui::Result<void> resize(cgpui::Size size, cgpui::DpiScale scale) override {
    resize_count += 1;
    last_resize_size = size;
    last_resize_scale = scale;
    return {};
  }

  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    begin_frame_count += 1;
    if (fail_begin_frame) {
      return std::unexpected(cgpui::Error{
          .code = cgpui::ErrorCode::frame_acquisition_failed,
          .message = "test frame failure"});
    }
    return std::unique_ptr<cgpui::RenderFrame>(
        new BorrowedFrame(frame_));
  }

  int resize_count = 0;
  int begin_frame_count = 0;
  bool fail_begin_frame = false;
  cgpui::Size last_resize_size{};
  cgpui::DpiScale last_resize_scale{};

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
};

class RecordingView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList& paint_list, cgpui::Size viewport_size) override {
    paint_count += 1;
    last_viewport_size = viewport_size;
    paint_list.fill_rect(
        cgpui::Rect{
            .origin = {1.0F, 2.0F},
            .size = {3.0F, 4.0F}},
        cgpui::Color{.r = 0.2F, .g = 0.4F, .b = 0.6F, .a = 1.0F});
  }

  int paint_count = 0;
  cgpui::Size last_viewport_size{};
};

class FakeWindow final : public cgpui::PlatformWindow {
 public:
  explicit FakeWindow(cgpui::WindowState state) : state_(state) {}

  cgpui::NativeSurfaceHandle native_surface() const override {
    return cgpui::Win32SurfaceHandle{};
  }

  cgpui::WindowState state() const override { return state_; }

  void request_redraw() override {
    request_redraw_count += 1;
    if (callback) {
      callback(cgpui::WindowRedrawRequested{});
    }
  }

  void request_close() override {
    request_close_count += 1;
    if (callback) {
      callback(cgpui::WindowCloseRequested{});
    }
  }

  void set_title(std::string_view title) override { last_title = title; }

  void dispatch_resize(cgpui::Size size, cgpui::DpiScale scale) {
    state_.framebuffer_size = size;
    state_.scale = scale;
    if (callback) {
      callback(cgpui::WindowResized{.size = size, .scale = scale});
    }
  }

  cgpui::PlatformEventCallback callback;
  int request_redraw_count = 0;
  int request_close_count = 0;
  std::string_view last_title;

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
    return std::unique_ptr<cgpui::PlatformWindow>(
        new BorrowedWindow(window_));
  }

  int run() override {
    run_count += 1;
    if (on_run) {
      on_run();
    }
    return run_result;
  }

  void quit() override { quit_count += 1; }

  FakeWindow& window_;
  int create_window_count = 0;
  int run_count = 0;
  int quit_count = 0;
  int run_result = 0;
  cgpui::WindowDescriptor last_descriptor{};
  void (*on_run)() = nullptr;

 private:
  class BorrowedWindow final : public cgpui::PlatformWindow {
   public:
    explicit BorrowedWindow(FakeWindow& window) : window_(window) {}

    cgpui::NativeSurfaceHandle native_surface() const override {
      return window_.native_surface();
    }
    cgpui::WindowState state() const override { return window_.state(); }
    void request_redraw() override { window_.request_redraw(); }
    void request_close() override { window_.request_close(); }
    void set_title(std::string_view title) override {
      window_.set_title(title);
    }

   private:
    FakeWindow& window_;
  };
};

struct RuntimeFixture {
  FakeWindow window{cgpui::WindowState{
      .framebuffer_size = {640.0F, 480.0F},
      .scale = cgpui::DpiScale{1.0F},
      .close_requested = false}};
  FakeApplication app{window};
  RecordingFrame frame;
  RecordingRenderer renderer{frame};
  RecordingView view;
};

int test_redraw_paints_initial_viewport() {
  RuntimeFixture fixture;
  fixture.app.on_run = +[] {};

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor& descriptor) {
        if (!equal(descriptor.framebuffer_size, cgpui::Size{640.0F, 480.0F})) {
          return cgpui::Result<cgpui::Renderer*>{std::unexpected(cgpui::Error{
              .code = cgpui::ErrorCode::renderer_initialization_failed,
              .message = "wrong descriptor size"})};
        }
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{
      .title = "Runtime Test",
      .size = {640.0F, 480.0F}});

  if (result != 0) {
    return 1;
  }
  if (fixture.app.create_window_count != 1 || fixture.app.run_count != 1) {
    return 2;
  }
  if (fixture.window.request_redraw_count != 1) {
    return 3;
  }
  if (fixture.renderer.begin_frame_count != 1 ||
      fixture.view.paint_count != 1 ||
      fixture.frame.present_count != 1) {
    return 4;
  }
  if (fixture.frame.draw_count != 1) {
    return 6;
  }
  if (!equal(fixture.view.last_viewport_size, cgpui::Size{640.0F, 480.0F})) {
    return 5;
  }

  return 0;
}

RuntimeFixture* resize_fixture = nullptr;

void dispatch_runtime_resize() {
  resize_fixture->window.dispatch_resize(
      cgpui::Size{320.0F, 240.0F},
      cgpui::DpiScale{2.0F});
  resize_fixture->window.request_redraw();
}

int test_resize_updates_renderer_and_viewport() {
  RuntimeFixture fixture;
  resize_fixture = &fixture;
  fixture.app.on_run = &dispatch_runtime_resize;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{
      .title = "Runtime Test",
      .size = {640.0F, 480.0F}});
  resize_fixture = nullptr;

  if (result != 0) {
    return 10;
  }
  if (fixture.renderer.resize_count != 1) {
    return 11;
  }
  if (!equal(fixture.renderer.last_resize_size, cgpui::Size{320.0F, 240.0F}) ||
      !equal(fixture.renderer.last_resize_scale, cgpui::DpiScale{2.0F})) {
    return 12;
  }
  if (!equal(fixture.view.last_viewport_size, cgpui::Size{320.0F, 240.0F})) {
    return 13;
  }
  if (fixture.renderer.begin_frame_count != 2 || fixture.view.paint_count != 2) {
    return 14;
  }

  return 0;
}

RuntimeFixture* close_fixture = nullptr;

void dispatch_runtime_close() {
  close_fixture->window.request_close();
}

int test_close_request_quits_application() {
  RuntimeFixture fixture;
  close_fixture = &fixture;
  fixture.app.on_run = &dispatch_runtime_close;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  close_fixture = nullptr;

  if (result != 0) {
    return 20;
  }
  if (fixture.app.quit_count != 1) {
    return 21;
  }

  return 0;
}

RuntimeFixture* render_failure_fixture = nullptr;

void dispatch_render_failure() {
  render_failure_fixture->renderer.fail_begin_frame = true;
  render_failure_fixture->window.request_redraw();
}

int test_render_failure_quits_and_returns_failure() {
  RuntimeFixture fixture;
  render_failure_fixture = &fixture;
  fixture.app.on_run = &dispatch_render_failure;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  render_failure_fixture = nullptr;

  if (result == 0) {
    return 30;
  }
  if (fixture.app.quit_count != 1) {
    return 31;
  }

  return 0;
}

} // namespace

int main() {
  if (const int result = test_redraw_paints_initial_viewport(); result != 0) {
    return result;
  }
  if (const int result = test_resize_updates_renderer_and_viewport();
      result != 0) {
    return result;
  }
  if (const int result = test_close_request_quits_application(); result != 0) {
    return result;
  }
  if (const int result = test_render_failure_quits_and_returns_failure();
      result != 0) {
    return result;
  }
  return 0;
}
