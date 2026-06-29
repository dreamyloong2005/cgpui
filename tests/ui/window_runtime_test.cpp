#include "cgpui/platform/platform.hpp"
#include "cgpui/ui/ui.hpp"

#include <expected>
#include <memory>
#include <string_view>
#include <utility>
#include <variant>

namespace {

bool equal(cgpui::Size lhs, cgpui::Size rhs) {
  return lhs.width == rhs.width && lhs.height == rhs.height;
}

bool equal(cgpui::DpiScale lhs, cgpui::DpiScale rhs) {
  return lhs.value == rhs.value;
}

bool equal(cgpui::Point lhs, cgpui::Point rhs) {
  return lhs.x == rhs.x && lhs.y == rhs.y;
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

  void handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::WindowRuntimeContext& context) override {
    event_count += 1;
    last_event_viewport_size = context.viewport_size;
    last_event_frame_index = context.frame_index;
    last_input_focused = context.input.focused;
    last_input_pointer_position = context.input.pointer_position;
    last_pointer_captured = context.input.pointer_captured;

    if (std::holds_alternative<cgpui::WindowFocused>(event)) {
      focus_count += 1;
      focus_event_saw_focused = context.input.focused;
    } else if (std::holds_alternative<cgpui::PointerMoved>(event)) {
      pointer_move_count += 1;
      pointer_move_event_position = context.input.pointer_position;
      if (capture_on_first_pointer_move && pointer_move_count == 1) {
        context.runtime.capture_pointer();
      }
      if (release_on_third_pointer_move && pointer_move_count == 3) {
        context.runtime.release_pointer();
      }
      if (pointer_move_count == 2) {
        second_pointer_move_saw_capture = context.input.pointer_captured;
      } else if (pointer_move_count == 4) {
        fourth_pointer_move_saw_capture = context.input.pointer_captured;
      }
    } else if (std::holds_alternative<cgpui::PointerButton>(event)) {
      pointer_button_count += 1;
      pointer_button_event_position = context.input.pointer_position;
    } else if (std::holds_alternative<cgpui::PointerScrolled>(event)) {
      pointer_scroll_count += 1;
      pointer_scroll_event_position = context.input.pointer_position;
    } else if (std::holds_alternative<cgpui::KeyboardKey>(event)) {
      keyboard_key_count += 1;
    } else if (std::holds_alternative<cgpui::TextInput>(event)) {
      text_input_count += 1;
    }

    if (request_redraw_on_event && event_redraw_requests == 0) {
      event_redraw_requests += 1;
      context.window.request_redraw();
    }
  }

  int paint_count = 0;
  int event_count = 0;
  int focus_count = 0;
  int pointer_move_count = 0;
  int pointer_button_count = 0;
  int pointer_scroll_count = 0;
  int keyboard_key_count = 0;
  int text_input_count = 0;
  int event_redraw_requests = 0;
  bool request_redraw_on_event = false;
  bool capture_on_first_pointer_move = false;
  bool release_on_third_pointer_move = false;
  bool last_input_focused = false;
  bool focus_event_saw_focused = false;
  bool last_pointer_captured = false;
  bool second_pointer_move_saw_capture = false;
  bool fourth_pointer_move_saw_capture = true;
  cgpui::Size last_viewport_size{};
  cgpui::Size last_event_viewport_size{};
  cgpui::Point last_input_pointer_position{};
  cgpui::Point pointer_move_event_position{};
  cgpui::Point pointer_button_event_position{};
  cgpui::Point pointer_scroll_event_position{};
  int last_event_frame_index = -1;
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

RuntimeFixture* event_dispatch_fixture = nullptr;

void dispatch_view_events() {
  auto& callback = event_dispatch_fixture->window.callback;
  callback(cgpui::WindowFocused{.focused = true});
  callback(cgpui::PointerMoved{.position = {12.0F, 24.0F}});
  callback(cgpui::PointerButton{
      .button = cgpui::MouseButton::left,
      .pressed = true,
      .position = {12.0F, 24.0F}});
  callback(cgpui::PointerScrolled{
      .delta = {0.0F, -4.0F},
      .position = {12.0F, 24.0F}});
  callback(cgpui::KeyboardKey{
      .key_code = 65,
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.shift = true}});
  callback(cgpui::TextInput{
      .text = "A",
      .modifiers = {.shift = true}});
}

int test_runtime_dispatches_input_events_to_view() {
  RuntimeFixture fixture;
  event_dispatch_fixture = &fixture;
  fixture.app.on_run = &dispatch_view_events;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  event_dispatch_fixture = nullptr;

  if (result != 0) {
    return 40;
  }
  if (fixture.view.event_count != 6) {
    return 41;
  }
  if (fixture.view.focus_count != 1 ||
      fixture.view.pointer_move_count != 1 ||
      fixture.view.pointer_button_count != 1 ||
      fixture.view.pointer_scroll_count != 1 ||
      fixture.view.keyboard_key_count != 1 ||
      fixture.view.text_input_count != 1) {
    return 42;
  }
  if (!equal(
          fixture.view.last_event_viewport_size,
          cgpui::Size{640.0F, 480.0F})) {
    return 43;
  }
  if (fixture.view.last_event_frame_index != 1) {
    return 44;
  }
  if (!fixture.view.focus_event_saw_focused ||
      !fixture.view.last_input_focused) {
    return 45;
  }
  if (!equal(
          fixture.view.pointer_move_event_position,
          cgpui::Point{12.0F, 24.0F})) {
    return 46;
  }
  if (!equal(
          fixture.view.pointer_button_event_position,
          cgpui::Point{12.0F, 24.0F})) {
    return 47;
  }
  if (!equal(
          fixture.view.pointer_scroll_event_position,
          cgpui::Point{12.0F, 24.0F})) {
    return 48;
  }
  if (!equal(
          fixture.view.last_input_pointer_position,
          cgpui::Point{12.0F, 24.0F})) {
    return 49;
  }

  return 0;
}

RuntimeFixture* event_redraw_fixture = nullptr;

void dispatch_event_driven_redraw() {
  event_redraw_fixture->window.callback(
      cgpui::PointerMoved{.position = {5.0F, 6.0F}});
}

int test_view_event_can_request_redraw() {
  RuntimeFixture fixture;
  event_redraw_fixture = &fixture;
  fixture.app.on_run = &dispatch_event_driven_redraw;
  fixture.view.request_redraw_on_event = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  event_redraw_fixture = nullptr;

  if (result != 0) {
    return 50;
  }
  if (fixture.view.event_count != 1 ||
      fixture.view.event_redraw_requests != 1) {
    return 51;
  }
  if (fixture.window.request_redraw_count != 2) {
    return 52;
  }
  if (fixture.renderer.begin_frame_count != 2 ||
      fixture.view.paint_count != 2 ||
      fixture.frame.present_count != 2) {
    return 53;
  }

  return 0;
}

RuntimeFixture* pointer_capture_fixture = nullptr;

void dispatch_pointer_capture_sequence() {
  auto& callback = pointer_capture_fixture->window.callback;
  callback(cgpui::PointerMoved{.position = {1.0F, 1.0F}});
  callback(cgpui::PointerMoved{.position = {2.0F, 2.0F}});
  callback(cgpui::PointerMoved{.position = {3.0F, 3.0F}});
  callback(cgpui::PointerMoved{.position = {4.0F, 4.0F}});
}

int test_view_can_capture_and_release_pointer() {
  RuntimeFixture fixture;
  pointer_capture_fixture = &fixture;
  fixture.app.on_run = &dispatch_pointer_capture_sequence;
  fixture.view.capture_on_first_pointer_move = true;
  fixture.view.release_on_third_pointer_move = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  pointer_capture_fixture = nullptr;

  if (result != 0) {
    return 60;
  }
  if (fixture.view.pointer_move_count != 4) {
    return 61;
  }
  if (!fixture.view.second_pointer_move_saw_capture) {
    return 62;
  }
  if (fixture.view.fourth_pointer_move_saw_capture ||
      fixture.view.last_pointer_captured) {
    return 63;
  }
  if (!equal(
          fixture.view.last_input_pointer_position,
          cgpui::Point{4.0F, 4.0F})) {
    return 64;
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
  if (const int result = test_runtime_dispatches_input_events_to_view();
      result != 0) {
    return result;
  }
  if (const int result = test_view_event_can_request_redraw(); result != 0) {
    return result;
  }
  if (const int result = test_view_can_capture_and_release_pointer();
      result != 0) {
    return result;
  }
  return 0;
}
