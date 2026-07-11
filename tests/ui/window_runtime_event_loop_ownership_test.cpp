#include "window_runtime_test_support.hpp"

namespace {

struct EventWindow {
  cgpui::PlatformEventCallback callback;
  bool callback_active = false;
  bool wrapper_destroyed = false;
  bool wrapper_destroyed_during_callback = false;

  void emit(const cgpui::PlatformEvent& event) {
    callback_active = true;
    callback(event);
    callback_active = false;
  }
};

class EventLoopWindow final : public cgpui::PlatformWindow {
 public:
  EventLoopWindow(EventWindow& event_window, cgpui::Size size)
      : event_window_(event_window), size_(size) {}
  ~EventLoopWindow() override {
    event_window_.wrapper_destroyed = true;
    event_window_.wrapper_destroyed_during_callback =
        event_window_.callback_active;
  }

  cgpui::NativeSurfaceHandle native_surface() const override { return {}; }
  cgpui::WindowState state() const override {
    return cgpui::WindowState{
        .framebuffer_size = size_,
        .scale = cgpui::DpiScale{1.0F},
    };
  }
  void request_redraw() override {}
  void request_close() override {}
  void set_title(std::string_view) override {}
  void set_cursor(cgpui::CursorShape) override {}
  void set_ime_text_input_placement(
      std::optional<cgpui::ImeTextInputPlacement>) override {}

 private:
  EventWindow& event_window_;
  cgpui::Size size_;
};

class EventLoopApplication final : public cgpui::PlatformApplication {
 public:
  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_window(
      const cgpui::WindowDescriptor& descriptor,
      cgpui::PlatformEventCallback callback) override {
    root.callback = std::move(callback);
    return std::make_unique<EventLoopWindow>(root, descriptor.size);
  }

  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_child_window(
      const cgpui::WindowDescriptor& descriptor,
      cgpui::PlatformWindow&,
      cgpui::PlatformEventCallback callback) override {
    child.callback = std::move(callback);
    return std::make_unique<EventLoopWindow>(child, descriptor.size);
  }

  int run() override {
    on_run();
    return 0;
  }
  void quit() override {}
  void request_wakeup() override { ++wakeup_count; }

  EventWindow root;
  EventWindow child;
  std::function<void()> on_run;
  int wakeup_count = 0;
};

} // namespace

int main() {
  EventLoopApplication application;
  RecordingView root_view;
  auto child_view = std::make_unique<RecordingView>();
  RecordingFrame frame;
  std::vector<std::unique_ptr<RecordingRenderer>> renderers;
  cgpui::WindowRuntime runtime(
      application,
      root_view,
      [&](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<cgpui::Renderer*> {
        auto renderer = std::make_unique<RecordingRenderer>(frame);
        cgpui::Renderer* raw = renderer.get();
        renderers.push_back(std::move(renderer));
        return raw;
      });
  const cgpui::AppOpenedWindow opened = runtime.open_window(
      cgpui::WindowOptions{}.title("Event Loop Child"),
      std::move(child_view));

  bool active_before_close = false;
  bool inactive_after_close = false;
  bool deferred_after_close = false;
  bool collected_after_wakeup = false;
  application.on_run = [&] {
    const cgpui::WindowRuntimeRecord* record =
        runtime.window_runtime_record(opened.runtime_id);
    active_before_close = record != nullptr && record->active;

    application.child.emit(cgpui::WindowCloseRequested{});
    record = runtime.window_runtime_record(opened.runtime_id);
    inactive_after_close = record != nullptr && !record->active &&
        record->window == nullptr && record->renderer == nullptr;
    deferred_after_close = !application.child.wrapper_destroyed &&
        application.wakeup_count == 1;

    application.child.emit(cgpui::KeyboardKey{
        .key_code = 65,
        .action = cgpui::KeyAction::pressed,
    });
    application.root.emit(cgpui::WindowWakeupRequested{});
    collected_after_wakeup = application.child.wrapper_destroyed &&
        !application.child.wrapper_destroyed_during_callback;
  };

  const int result = runtime.run(
      cgpui::WindowDescriptor{.title = "Root"},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  return result == 0 && active_before_close && inactive_after_close &&
          deferred_after_close && collected_after_wakeup &&
          renderers.size() == 2
      ? 0
      : 1;
}
