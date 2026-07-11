#include "window_runtime_test_support.hpp"

namespace {

class PointerCaptureWindow final : public cgpui::PlatformWindow {
 public:
  cgpui::NativeSurfaceHandle native_surface() const override {
    return cgpui::Win32SurfaceHandle{};
  }
  cgpui::WindowState state() const override {
    return {.framebuffer_size = {640.0F, 480.0F}};
  }
  cgpui::PlatformPointerCaptureState pointer_capture_state() const override {
    return {.supported = true, .captured = captured};
  }
  void set_pointer_capture(bool value) override {
    set_capture_count += 1;
    captured = value;
  }
  void request_redraw() override {
    if (callback) {
      callback(cgpui::WindowRedrawRequested{});
    }
  }
  void request_close() override {}
  void set_title(std::string_view) override {}
  void set_cursor(cgpui::CursorShape) override {}
  void set_ime_text_input_placement(
      std::optional<cgpui::ImeTextInputPlacement>) override {}
  void dispatch_capture_lost() {
    captured = false;
    callback(cgpui::PointerCaptureChanged{.captured = false});
  }

  cgpui::PlatformEventCallback callback;
  int set_capture_count = 0;
  bool captured = false;
};

class PointerCaptureApplication final : public cgpui::PlatformApplication {
 public:
  explicit PointerCaptureApplication(PointerCaptureWindow& window)
      : window_(window) {}

  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_window(
      const cgpui::WindowDescriptor&,
      cgpui::PlatformEventCallback callback) override {
    window_.callback = std::move(callback);
    return std::unique_ptr<cgpui::PlatformWindow>(new BorrowedWindow(window_));
  }
  int run() override {
    on_run();
    return 0;
  }
  void quit() override {}

  std::function<void()> on_run;

 private:
  class BorrowedWindow final : public cgpui::PlatformWindow {
   public:
    explicit BorrowedWindow(PointerCaptureWindow& window) : window_(window) {}
    cgpui::NativeSurfaceHandle native_surface() const override {
      return window_.native_surface();
    }
    cgpui::WindowState state() const override { return window_.state(); }
    cgpui::PlatformPointerCaptureState pointer_capture_state() const override {
      return window_.pointer_capture_state();
    }
    void set_pointer_capture(bool captured) override {
      window_.set_pointer_capture(captured);
    }
    void request_redraw() override { window_.request_redraw(); }
    void request_close() override { window_.request_close(); }
    void set_title(std::string_view title) override { window_.set_title(title); }
    void set_cursor(cgpui::CursorShape shape) override {
      window_.set_cursor(shape);
    }
    void set_ime_text_input_placement(
        std::optional<cgpui::ImeTextInputPlacement> placement) override {
      window_.set_ime_text_input_placement(placement);
    }

   private:
    PointerCaptureWindow& window_;
  };

  PointerCaptureWindow& window_;
};

} // namespace

int main() {
  PointerCaptureWindow window;
  PointerCaptureApplication app(window);
  RecordingFrame frame;
  RecordingRenderer renderer(frame);
  RecordingView view;
  view.capture_on_first_pointer_move = true;

  bool loss_saw_capture = true;
  bool loss_saw_pointer_down = true;
  bool loss_saw_dragging = true;
  app.on_run = [&] {
    window.callback(cgpui::PointerMoved{.position = {5.0F, 5.0F}});
    window.callback(cgpui::PointerButton{
        .button = cgpui::MouseButton::left,
        .pressed = true,
        .position = {5.0F, 5.0F}});
    window.callback(cgpui::PointerMoved{.position = {6.0F, 6.0F}});
    window.dispatch_capture_lost();
  };

  cgpui::FixedSizeElement element(cgpui::Size{40.0F, 40.0F});
  element.assign_id(cgpui::ElementId{11});
  (void)element.layout(cgpui::LayoutInput{});
  cgpui::WindowRuntime runtime(
      app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&renderer};
      });
  runtime.set_element_root(&element);
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord& record) {
        if (record.event_kind == cgpui::EventKind::pointer_capture_changed) {
          loss_saw_capture = context.input.pointer_captured;
          loss_saw_pointer_down = context.input.pointer_down_element_id.has_value();
          loss_saw_dragging = context.input.dragging ||
              context.input.dragging_element_id.has_value();
        }
      });

  if (runtime.run(cgpui::WindowDescriptor{}) != 0) {
    return 1;
  }
  if (window.set_capture_count != 1 || window.captured || loss_saw_capture ||
      loss_saw_pointer_down || loss_saw_dragging) {
    return 2;
  }
  return 0;
}
