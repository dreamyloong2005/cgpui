#include "cgpui/platform/platform.hpp"
#include "cgpui/ui/ui.hpp"

#include <expected>
#include <memory>
#include <optional>
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

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::WindowRuntimeContext& context) override {
    event_count += 1;
    last_event_result_consumed = context.last_event_result.consumed;
    last_event_result_cancelled = context.last_event_result.cancelled;
    saw_event_route = context.event_route.has_value();
    if (context.event_route) {
      last_event_route = *context.event_route;
    }
    saw_last_event_dispatch = context.last_event_dispatch.has_value();
    if (context.last_event_dispatch) {
      last_event_dispatch = *context.last_event_dispatch;
    }
    if (!saw_first_view_id) {
      saw_first_view_id = true;
      first_view_id = context.view_id;
    } else if (context.view_id != first_view_id) {
      view_id_stayed_stable = false;
    }
    last_view_id = context.view_id;
    last_event_viewport_size = context.viewport_size;
    last_event_frame_index = context.frame_index;
    last_input_focused = context.input.focused;
    last_input_pointer_position = context.input.pointer_position;
    last_pointer_captured = context.input.pointer_captured;
    last_pointer_capture_owner_present =
        context.input.pointer_capture_owner.has_value();
    last_pointer_capture_owner_matches_view =
        context.input.pointer_capture_owner.has_value() &&
        *context.input.pointer_capture_owner == context.view_id;
    last_keyboard_focused = context.input.keyboard_focused;
    last_keyboard_focus_owner_present =
        context.input.keyboard_focus_owner.has_value();
    last_keyboard_focus_owner_matches_view =
        context.input.keyboard_focus_owner.has_value() &&
        *context.input.keyboard_focus_owner == context.view_id;

    if (std::holds_alternative<cgpui::WindowFocused>(event)) {
      focus_count += 1;
      focus_event_saw_focused = context.input.focused;
    } else if (std::holds_alternative<cgpui::PointerMoved>(event)) {
      pointer_move_count += 1;
      pointer_move_event_position = context.input.pointer_position;
      if (capture_on_first_pointer_move && pointer_move_count == 1) {
        context.runtime.capture_pointer();
      }
      if (capture_pointer_owner_on_first_pointer_move &&
          pointer_move_count == 1) {
        context.runtime.capture_pointer(context.view_id);
      }
      if (release_pointer_with_wrong_owner_on_second_pointer_move &&
          pointer_move_count == 2) {
        context.runtime.release_pointer(
            cgpui::ViewId{context.view_id.value + 1});
      }
      if (release_on_third_pointer_move && pointer_move_count == 3) {
        context.runtime.release_pointer();
      }
      if (release_pointer_owner_on_third_pointer_move &&
          pointer_move_count == 3) {
        context.runtime.release_pointer(context.view_id);
      }
      if (pointer_move_count == 2) {
        second_pointer_move_saw_capture = context.input.pointer_captured;
        second_pointer_move_saw_capture_owner =
            last_pointer_capture_owner_matches_view;
      } else if (pointer_move_count == 3) {
        third_pointer_move_saw_capture = context.input.pointer_captured;
        third_pointer_move_saw_capture_owner =
            last_pointer_capture_owner_matches_view;
      } else if (pointer_move_count == 4) {
        fourth_pointer_move_saw_capture = context.input.pointer_captured;
        fourth_pointer_move_saw_capture_owner =
            last_pointer_capture_owner_matches_view;
      }
    } else if (std::holds_alternative<cgpui::PointerButton>(event)) {
      pointer_button_count += 1;
      pointer_button_event_position = context.input.pointer_position;
    } else if (std::holds_alternative<cgpui::PointerScrolled>(event)) {
      pointer_scroll_count += 1;
      pointer_scroll_event_position = context.input.pointer_position;
    } else if (std::holds_alternative<cgpui::KeyboardKey>(event)) {
      keyboard_key_count += 1;
      if (request_keyboard_focus_on_first_key && keyboard_key_count == 1) {
        context.runtime.request_keyboard_focus();
      }
      if (request_keyboard_focus_owner_on_first_key &&
          keyboard_key_count == 1) {
        context.runtime.request_keyboard_focus(context.view_id);
      }
      if (release_keyboard_focus_with_wrong_owner_on_second_key &&
          keyboard_key_count == 2) {
        context.runtime.release_keyboard_focus(
            cgpui::ViewId{context.view_id.value + 1});
      }
      if (release_keyboard_focus_on_third_key && keyboard_key_count == 3) {
        context.runtime.release_keyboard_focus();
      }
      if (release_keyboard_focus_owner_on_third_key &&
          keyboard_key_count == 3) {
        context.runtime.release_keyboard_focus(context.view_id);
      }
      if (keyboard_key_count == 2) {
        second_key_saw_keyboard_focus = context.input.keyboard_focused;
        second_key_saw_keyboard_focus_owner =
            last_keyboard_focus_owner_matches_view;
      } else if (keyboard_key_count == 3) {
        third_key_saw_keyboard_focus = context.input.keyboard_focused;
        third_key_saw_keyboard_focus_owner =
            last_keyboard_focus_owner_matches_view;
      } else if (keyboard_key_count == 4) {
        fourth_key_saw_keyboard_focus = context.input.keyboard_focused;
        fourth_key_saw_keyboard_focus_owner =
            last_keyboard_focus_owner_matches_view;
      }
    } else if (std::holds_alternative<cgpui::TextInput>(event)) {
      text_input_count += 1;
      text_input_saw_keyboard_focus = context.input.keyboard_focused;
      text_input_saw_keyboard_focus_owner =
          last_keyboard_focus_owner_matches_view;
    }

    if (request_redraw_on_event && event_redraw_requests == 0) {
      event_redraw_requests += 1;
      context.window.request_redraw();
    }

    if (consume_next_event) {
      consume_next_event = false;
      return cgpui::EventResult::consumed_event();
    }
    if (cancel_next_event) {
      cancel_next_event = false;
      return cgpui::EventResult::cancelled_event();
    }
    return cgpui::EventResult::unhandled();
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
  bool consume_next_event = false;
  bool cancel_next_event = false;
  bool capture_on_first_pointer_move = false;
  bool release_on_third_pointer_move = false;
  bool capture_pointer_owner_on_first_pointer_move = false;
  bool release_pointer_with_wrong_owner_on_second_pointer_move = false;
  bool release_pointer_owner_on_third_pointer_move = false;
  bool request_keyboard_focus_on_first_key = false;
  bool release_keyboard_focus_on_third_key = false;
  bool request_keyboard_focus_owner_on_first_key = false;
  bool release_keyboard_focus_with_wrong_owner_on_second_key = false;
  bool release_keyboard_focus_owner_on_third_key = false;
  bool last_input_focused = false;
  bool focus_event_saw_focused = false;
  bool last_pointer_captured = false;
  bool last_pointer_capture_owner_present = false;
  bool last_pointer_capture_owner_matches_view = false;
  bool second_pointer_move_saw_capture = false;
  bool second_pointer_move_saw_capture_owner = false;
  bool third_pointer_move_saw_capture = false;
  bool third_pointer_move_saw_capture_owner = false;
  bool fourth_pointer_move_saw_capture = true;
  bool fourth_pointer_move_saw_capture_owner = true;
  bool last_keyboard_focused = false;
  bool last_keyboard_focus_owner_present = false;
  bool last_keyboard_focus_owner_matches_view = false;
  bool second_key_saw_keyboard_focus = false;
  bool second_key_saw_keyboard_focus_owner = false;
  bool third_key_saw_keyboard_focus = false;
  bool third_key_saw_keyboard_focus_owner = false;
  bool fourth_key_saw_keyboard_focus = true;
  bool fourth_key_saw_keyboard_focus_owner = true;
  bool text_input_saw_keyboard_focus = false;
  bool text_input_saw_keyboard_focus_owner = false;
  bool last_event_result_consumed = false;
  bool last_event_result_cancelled = false;
  bool saw_event_route = false;
  bool saw_last_event_dispatch = false;
  bool saw_first_view_id = false;
  bool view_id_stayed_stable = true;
  cgpui::EventRoute last_event_route{};
  cgpui::EventDispatchRecord last_event_dispatch{};
  cgpui::ViewId first_view_id{};
  cgpui::ViewId last_view_id{};
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

RuntimeFixture* event_result_fixture = nullptr;

void dispatch_event_result_sequence() {
  auto& callback = event_result_fixture->window.callback;
  event_result_fixture->view.consume_next_event = true;
  callback(cgpui::PointerMoved{.position = {7.0F, 8.0F}});
  callback(cgpui::PointerMoved{.position = {9.0F, 10.0F}});
  event_result_fixture->view.cancel_next_event = true;
  callback(cgpui::KeyboardKey{
      .key_code = 65,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::TextInput{.text = "a"});
}

int test_runtime_exposes_last_view_event_result() {
  RuntimeFixture fixture;
  event_result_fixture = &fixture;
  fixture.app.on_run = &dispatch_event_result_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  event_result_fixture = nullptr;

  if (result != 0) {
    return 55;
  }
  if (fixture.view.pointer_move_count != 2 ||
      fixture.view.keyboard_key_count != 1 ||
      fixture.view.text_input_count != 1) {
    return 56;
  }
  if (!fixture.view.last_event_result_consumed ||
      !fixture.view.last_event_result_cancelled) {
    return 57;
  }

  return 0;
}

RuntimeFixture* event_observability_fixture = nullptr;

void dispatch_event_observability_sequence() {
  auto& callback = event_observability_fixture->window.callback;
  event_observability_fixture->view.consume_next_event = true;
  callback(cgpui::PointerMoved{.position = {11.0F, 12.0F}});
  event_observability_fixture->view.cancel_next_event = true;
  callback(cgpui::KeyboardKey{
      .key_code = 66,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::TextInput{.text = "b"});
}

int test_runtime_reports_each_view_event_dispatch() {
  RuntimeFixture fixture;
  event_observability_fixture = &fixture;
  fixture.app.on_run = &dispatch_event_observability_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  int callback_count = 0;
  cgpui::EventDispatchRecord first_record{};
  cgpui::EventDispatchRecord second_record{};
  cgpui::EventDispatchRecord third_record{};
  bool callback_context_saw_current_record = true;
  bool callback_view_id_matched_context = true;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord& record) {
        callback_count += 1;
        if (callback_count == 1) {
          first_record = record;
        } else if (callback_count == 2) {
          second_record = record;
        } else if (callback_count == 3) {
          third_record = record;
        }
        callback_context_saw_current_record =
            callback_context_saw_current_record &&
            context.last_event_dispatch.has_value() &&
            context.last_event_dispatch->sequence == record.sequence &&
            context.last_event_dispatch->event_kind == record.event_kind &&
            context.last_event_dispatch->view_id == record.view_id &&
            context.last_event_dispatch->result.consumed ==
                record.result.consumed &&
            context.last_event_dispatch->result.cancelled ==
                record.result.cancelled;
        callback_view_id_matched_context =
            callback_view_id_matched_context &&
            record.view_id == context.view_id;
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  event_observability_fixture = nullptr;

  if (result != 0) {
    return 58;
  }
  if (callback_count != 3) {
    return 59;
  }
  if (first_record.sequence != 1 ||
      first_record.event_kind != cgpui::EventKind::pointer_moved ||
      !first_record.result.consumed ||
      first_record.result.cancelled) {
    return 100;
  }
  if (second_record.sequence != 2 ||
      second_record.event_kind != cgpui::EventKind::keyboard_key ||
      !second_record.result.consumed ||
      !second_record.result.cancelled) {
    return 101;
  }
  if (third_record.sequence != 3 ||
      third_record.event_kind != cgpui::EventKind::text_input ||
      third_record.result.consumed ||
      third_record.result.cancelled) {
    return 102;
  }
  if (first_record.view_id.value == 0 ||
      first_record.view_id != second_record.view_id ||
      second_record.view_id != third_record.view_id) {
    return 103;
  }
  if (!callback_context_saw_current_record ||
      !callback_view_id_matched_context) {
    return 104;
  }
  if (!fixture.view.saw_last_event_dispatch ||
      fixture.view.last_event_dispatch.sequence != 2 ||
      fixture.view.last_event_dispatch.event_kind !=
          cgpui::EventKind::keyboard_key ||
      !fixture.view.last_event_dispatch.result.cancelled) {
    return 105;
  }

  return 0;
}

int test_event_router_routes_events_to_root_view() {
  const cgpui::ViewId root_view_id{42};

  const auto pointer_route = cgpui::EventRouter::route_to_root(
      cgpui::PointerMoved{.position = {1.0F, 2.0F}},
      root_view_id);
  const auto key_route = cgpui::EventRouter::route_to_root(
      cgpui::KeyboardKey{
          .key_code = 13,
          .action = cgpui::KeyAction::pressed},
      root_view_id);

  if (pointer_route.target_view_id != root_view_id ||
      pointer_route.event_kind != cgpui::EventKind::pointer_moved) {
    return 106;
  }
  if (key_route.target_view_id != root_view_id ||
      key_route.event_kind != cgpui::EventKind::keyboard_key) {
    return 107;
  }

  return 0;
}

RuntimeFixture* event_route_fixture = nullptr;

void dispatch_event_route_sequence() {
  event_route_fixture->window.callback(cgpui::PointerMoved{
      .position = {21.0F, 22.0F}});
}

int test_runtime_exposes_current_event_route() {
  RuntimeFixture fixture;
  event_route_fixture = &fixture;
  fixture.app.on_run = &dispatch_event_route_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  cgpui::EventDispatchRecord callback_record{};
  cgpui::EventRoute callback_route{};
  bool callback_saw_route = false;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord& record) {
        callback_record = record;
        callback_saw_route = context.event_route.has_value();
        if (context.event_route) {
          callback_route = *context.event_route;
        }
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  event_route_fixture = nullptr;

  if (result != 0) {
    return 108;
  }
  if (!fixture.view.saw_event_route ||
      fixture.view.last_event_route.target_view_id != fixture.view.last_view_id ||
      fixture.view.last_event_route.event_kind !=
          cgpui::EventKind::pointer_moved) {
    return 109;
  }
  if (!callback_saw_route ||
      callback_route.target_view_id != fixture.view.last_view_id ||
      callback_route.event_kind != cgpui::EventKind::pointer_moved) {
    return 110;
  }
  if (callback_record.route.target_view_id != fixture.view.last_view_id ||
      callback_record.route.event_kind != cgpui::EventKind::pointer_moved ||
      callback_record.view_id != callback_record.route.target_view_id ||
      callback_record.event_kind != callback_record.route.event_kind) {
    return 111;
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

int test_pointer_capture_tracks_owner_view_id() {
  RuntimeFixture fixture;
  pointer_capture_fixture = &fixture;
  fixture.app.on_run = &dispatch_pointer_capture_sequence;
  fixture.view.capture_pointer_owner_on_first_pointer_move = true;
  fixture.view.release_pointer_with_wrong_owner_on_second_pointer_move = true;
  fixture.view.release_pointer_owner_on_third_pointer_move = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  pointer_capture_fixture = nullptr;

  if (result != 0) {
    return 90;
  }
  if (!fixture.view.saw_first_view_id ||
      fixture.view.first_view_id.value == 0 ||
      !fixture.view.view_id_stayed_stable) {
    return 91;
  }
  if (!fixture.view.second_pointer_move_saw_capture ||
      !fixture.view.second_pointer_move_saw_capture_owner) {
    return 92;
  }
  if (!fixture.view.third_pointer_move_saw_capture ||
      !fixture.view.third_pointer_move_saw_capture_owner) {
    return 93;
  }
  if (fixture.view.fourth_pointer_move_saw_capture ||
      fixture.view.fourth_pointer_move_saw_capture_owner ||
      fixture.view.last_pointer_capture_owner_present) {
    return 94;
  }
  if (!equal(
          fixture.view.last_input_pointer_position,
          cgpui::Point{4.0F, 4.0F})) {
    return 95;
  }

  return 0;
}

RuntimeFixture* keyboard_focus_fixture = nullptr;

void dispatch_keyboard_focus_sequence() {
  auto& callback = keyboard_focus_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 65,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::KeyboardKey{
      .key_code = 66,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::TextInput{.text = "b"});
  callback(cgpui::KeyboardKey{
      .key_code = 67,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::KeyboardKey{
      .key_code = 68,
      .action = cgpui::KeyAction::pressed});
}

int test_view_can_request_and_release_keyboard_focus() {
  RuntimeFixture fixture;
  keyboard_focus_fixture = &fixture;
  fixture.app.on_run = &dispatch_keyboard_focus_sequence;
  fixture.view.request_keyboard_focus_on_first_key = true;
  fixture.view.release_keyboard_focus_on_third_key = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  keyboard_focus_fixture = nullptr;

  if (result != 0) {
    return 70;
  }
  if (fixture.view.keyboard_key_count != 4 ||
      fixture.view.text_input_count != 1) {
    return 71;
  }
  if (!fixture.view.second_key_saw_keyboard_focus ||
      !fixture.view.text_input_saw_keyboard_focus) {
    return 72;
  }
  if (fixture.view.fourth_key_saw_keyboard_focus ||
      fixture.view.last_keyboard_focused) {
    return 73;
  }

  return 0;
}

int test_keyboard_focus_tracks_owner_view_id() {
  RuntimeFixture fixture;
  keyboard_focus_fixture = &fixture;
  fixture.app.on_run = &dispatch_keyboard_focus_sequence;
  fixture.view.request_keyboard_focus_owner_on_first_key = true;
  fixture.view.release_keyboard_focus_with_wrong_owner_on_second_key = true;
  fixture.view.release_keyboard_focus_owner_on_third_key = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  keyboard_focus_fixture = nullptr;

  if (result != 0) {
    return 80;
  }
  if (!fixture.view.saw_first_view_id ||
      fixture.view.first_view_id.value == 0 ||
      !fixture.view.view_id_stayed_stable) {
    return 81;
  }
  if (!fixture.view.second_key_saw_keyboard_focus ||
      !fixture.view.second_key_saw_keyboard_focus_owner) {
    return 82;
  }
  if (!fixture.view.text_input_saw_keyboard_focus ||
      !fixture.view.text_input_saw_keyboard_focus_owner) {
    return 83;
  }
  if (!fixture.view.third_key_saw_keyboard_focus ||
      !fixture.view.third_key_saw_keyboard_focus_owner) {
    return 84;
  }
  if (fixture.view.fourth_key_saw_keyboard_focus ||
      fixture.view.fourth_key_saw_keyboard_focus_owner ||
      fixture.view.last_keyboard_focus_owner_present) {
    return 85;
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
  if (const int result = test_runtime_exposes_last_view_event_result();
      result != 0) {
    return result;
  }
  if (const int result = test_runtime_reports_each_view_event_dispatch();
      result != 0) {
    return result;
  }
  if (const int result = test_event_router_routes_events_to_root_view();
      result != 0) {
    return result;
  }
  if (const int result = test_runtime_exposes_current_event_route();
      result != 0) {
    return result;
  }
  if (const int result = test_view_can_capture_and_release_pointer();
      result != 0) {
    return result;
  }
  if (const int result = test_pointer_capture_tracks_owner_view_id();
      result != 0) {
    return result;
  }
  if (const int result = test_view_can_request_and_release_keyboard_focus();
      result != 0) {
    return result;
  }
  if (const int result = test_keyboard_focus_tracks_owner_view_id();
      result != 0) {
    return result;
  }
  return 0;
}
