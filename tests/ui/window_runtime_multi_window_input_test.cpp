#include "cgpui/cgpui.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace {

struct NativeWindow {
  cgpui::WindowState state;
  cgpui::PlatformEventCallback callback;
  bool pointer_captured = false;
};

class InputWindow final : public cgpui::PlatformWindow {
 public:
  explicit InputWindow(NativeWindow& window) : window_(window) {}
  cgpui::NativeSurfaceHandle native_surface() const override { return {}; }
  cgpui::WindowState state() const override { return window_.state; }
  void request_redraw() override {}
  void request_close() override {}
  void set_title(std::string_view) override {}
  void set_cursor(cgpui::CursorShape) override {}
  void set_pointer_capture(bool captured) override {
    window_.pointer_captured = captured;
  }
  void set_ime_text_input_placement(
      std::optional<cgpui::ImeTextInputPlacement>) override {}

 private:
  NativeWindow& window_;
};

class InputApplication final : public cgpui::PlatformApplication {
 public:
  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_window(
      const cgpui::WindowDescriptor& descriptor,
      cgpui::PlatformEventCallback callback) override {
    root.state = {.framebuffer_size = descriptor.size,
                  .scale = cgpui::DpiScale{1.0F}};
    root.callback = std::move(callback);
    return std::make_unique<InputWindow>(root);
  }
  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_child_window(
      const cgpui::WindowDescriptor& descriptor,
      cgpui::PlatformWindow&,
      cgpui::PlatformEventCallback callback) override {
    child.state = {.framebuffer_size = descriptor.size,
                   .scale = cgpui::DpiScale{1.0F}};
    child.callback = std::move(callback);
    return std::make_unique<InputWindow>(child);
  }
  int run() override {
    on_run();
    return 0;
  }
  void quit() override {}

  NativeWindow root;
  NativeWindow child;
  std::function<void()> on_run;
};

class InputFrame final : public cgpui::RenderFrame {
 public:
  void clear(cgpui::Color) override {}
  void draw_rect(const cgpui::SolidRect&) override {}
  cgpui::Result<void> present() override { return {}; }
};

class InputRenderer final : public cgpui::Renderer {
 public:
  cgpui::Result<void> resize(cgpui::Size, cgpui::DpiScale) override {
    return {};
  }
  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    return std::make_unique<InputFrame>();
  }
};

class RootView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}
};

class ChildView final : public cgpui::View {
 public:
  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::WindowRuntimeContext& context) override {
    if (std::holds_alternative<cgpui::PointerButton>(event)) {
      context.capture_pointer(cgpui::PointerCaptureOwner::view(context.view_id));
      context.request_keyboard_focus();
    }
    return cgpui::EventResult::unhandled();
  }
  void paint(cgpui::PaintList&, cgpui::Size) override {}
};

bool matches_root_input(const cgpui::ViewInputState& input) {
  return input.focused && input.pointer_position.x == 11.0F &&
      input.pointer_position.y == 12.0F && !input.pointer_captured &&
      input.keyboard_focus_owner == cgpui::ViewId{1};
}

} // namespace

int main() {
  InputApplication application;
  RootView root_view;
  std::vector<std::unique_ptr<InputRenderer>> renderers;
  cgpui::WindowRuntime runtime(
      application,
      root_view,
      [&](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<cgpui::Renderer*> {
        auto renderer = std::make_unique<InputRenderer>();
        cgpui::Renderer* raw = renderer.get();
        renderers.push_back(std::move(renderer));
        return raw;
      });
  const cgpui::AppOpenedWindow opened = runtime.open_window(
      cgpui::WindowOptions{}.title("Input Child").size(320.0F, 200.0F),
      std::make_unique<ChildView>());

  bool child_context_isolated = false;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord&) {
        if (context.window_runtime_id == opened.runtime_id &&
            context.input.keyboard_focus_owner == opened.root_view_id) {
          child_context_isolated = context.input.focused &&
              context.input.pointer_position.x == 31.0F &&
              context.input.pointer_position.y == 32.0F &&
              context.input.pointer_captured;
        }
      });

  bool root_accessors_isolated = false;
  bool root_record_isolated = false;
  bool child_record_isolated = false;
  application.on_run = [&] {
    application.root.callback(cgpui::WindowFocused{.focused = true});
    application.root.callback(
        cgpui::PointerMoved{.position = {11.0F, 12.0F}});
    runtime.request_keyboard_focus();

    application.child.callback(cgpui::WindowFocused{.focused = true});
    application.child.callback(
        cgpui::PointerMoved{.position = {31.0F, 32.0F}});
    application.child.callback(cgpui::PointerButton{
        .button = cgpui::MouseButton::left,
        .pressed = true,
        .position = {31.0F, 32.0F}});

    root_accessors_isolated = matches_root_input(runtime.input_state());
    const cgpui::WindowRuntimeRecord* root_record =
        runtime.window_runtime_record(runtime.root_window_runtime_id());
    const cgpui::WindowRuntimeRecord* child_record =
        runtime.window_runtime_record(opened.runtime_id);
    root_record_isolated =
        root_record != nullptr && matches_root_input(root_record->input);
    child_record_isolated = child_record != nullptr &&
        child_record->input.focused && child_record->input.pointer_captured &&
        child_record->input.pointer_position.x == 31.0F &&
        child_record->input.pointer_position.y == 32.0F &&
        child_record->input.keyboard_focus_owner == opened.root_view_id;
  };

  const int result = runtime.run(
      cgpui::WindowDescriptor{
          .title = "Root",
          .size = {.width = 640.0F, .height = 480.0F}},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  return result == 0 && root_accessors_isolated && root_record_isolated &&
          child_record_isolated && child_context_isolated &&
          !application.root.pointer_captured &&
          application.child.pointer_captured && renderers.size() == 2
      ? 0
      : 1;
}
