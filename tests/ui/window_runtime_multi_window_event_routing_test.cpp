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
};

class EventWindow final : public cgpui::PlatformWindow {
 public:
  explicit EventWindow(NativeWindow& window) : window_(window) {}
  cgpui::NativeSurfaceHandle native_surface() const override { return {}; }
  cgpui::WindowState state() const override { return window_.state; }
  void request_redraw() override {}
  void request_close() override {}
  void set_title(std::string_view) override {}
  void set_cursor(cgpui::CursorShape) override {}
  void set_ime_text_input_placement(
      std::optional<cgpui::ImeTextInputPlacement>) override {}

 private:
  NativeWindow& window_;
};

class EventApplication final : public cgpui::PlatformApplication {
 public:
  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_window(
      const cgpui::WindowDescriptor& descriptor,
      cgpui::PlatformEventCallback callback) override {
    root.state = {.framebuffer_size = descriptor.size,
                  .scale = cgpui::DpiScale{1.0F}};
    root.callback = std::move(callback);
    return std::make_unique<EventWindow>(root);
  }
  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_child_window(
      const cgpui::WindowDescriptor& descriptor,
      cgpui::PlatformWindow&,
      cgpui::PlatformEventCallback callback) override {
    child.state = {.framebuffer_size = descriptor.size,
                   .scale = cgpui::DpiScale{1.0F}};
    child.callback = std::move(callback);
    return std::make_unique<EventWindow>(child);
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

class EventFrame final : public cgpui::RenderFrame {
 public:
  void clear(cgpui::Color) override {}
  void draw_rect(const cgpui::SolidRect&) override {}
  cgpui::Result<void> present() override { return {}; }
};

class EventRenderer final : public cgpui::Renderer {
 public:
  cgpui::Result<void> resize(cgpui::Size, cgpui::DpiScale) override {
    return {};
  }
  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    return std::make_unique<EventFrame>();
  }
};

class RootView final : public cgpui::View {
 public:
  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent&,
      const cgpui::WindowRuntimeContext&) override {
    return cgpui::EventResult::consumed_event();
  }
  void paint(cgpui::PaintList&, cgpui::Size) override {}
};

class ChildView final : public cgpui::View {
 public:
  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent&,
      const cgpui::WindowRuntimeContext&) override {
    return cgpui::EventResult::cancelled_event();
  }
  void paint(cgpui::PaintList&, cgpui::Size) override {}
};

bool dispatch_matches(
    const cgpui::WindowRuntimeRecord& record,
    cgpui::ViewId view_id,
    bool consumed,
    bool cancelled) {
  return record.event_route.has_value() &&
      record.event_route->target_view_id == view_id &&
      record.last_event_dispatch.has_value() &&
      record.last_event_dispatch->view_id == view_id &&
      record.last_event_dispatch->route.target_view_id == view_id &&
      record.last_event_dispatch->result.consumed == consumed &&
      record.last_event_dispatch->result.cancelled == cancelled &&
      record.last_event_result.consumed == consumed &&
      record.last_event_result.cancelled == cancelled;
}

} // namespace

int main() {
  EventApplication application;
  RootView root_view;
  std::vector<std::unique_ptr<EventRenderer>> renderers;
  cgpui::WindowRuntime runtime(
      application,
      root_view,
      [&](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<cgpui::Renderer*> {
        auto renderer = std::make_unique<EventRenderer>();
        cgpui::Renderer* raw = renderer.get();
        renderers.push_back(std::move(renderer));
        return raw;
      });
  const cgpui::AppOpenedWindow opened = runtime.open_window(
      cgpui::WindowOptions{}.title("Event Child").size(320.0F, 200.0F),
      std::make_unique<ChildView>());

  bool root_callback_isolated = false;
  bool child_callback_isolated = false;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord& record) {
        const bool route_matches = context.event_route.has_value() &&
            context.event_route->target_view_id == context.view_id &&
            context.last_event_dispatch.has_value() &&
            context.last_event_dispatch->sequence == record.sequence;
        if (context.window_runtime_id == runtime.root_window_runtime_id()) {
          root_callback_isolated = route_matches && record.result.consumed &&
              !record.result.cancelled;
        } else if (context.window_runtime_id == opened.runtime_id) {
          child_callback_isolated = route_matches && record.result.consumed &&
              record.result.cancelled;
        }
      });

  bool root_record_isolated = false;
  bool child_record_isolated = false;
  bool sequences_isolated = false;
  application.on_run = [&] {
    application.root.callback(cgpui::KeyboardKey{
        .key_code = 65,
        .action = cgpui::KeyAction::pressed});
    application.child.callback(cgpui::KeyboardKey{
        .key_code = 66,
        .action = cgpui::KeyAction::pressed});

    const cgpui::WindowRuntimeRecord* root_record =
        runtime.window_runtime_record(runtime.root_window_runtime_id());
    const cgpui::WindowRuntimeRecord* child_record =
        runtime.window_runtime_record(opened.runtime_id);
    root_record_isolated = root_record != nullptr &&
        dispatch_matches(*root_record, cgpui::ViewId{1}, true, false);
    child_record_isolated = child_record != nullptr &&
        dispatch_matches(*child_record, opened.root_view_id, true, true);
    sequences_isolated = root_record != nullptr && child_record != nullptr &&
        root_record->last_event_dispatch.has_value() &&
        child_record->last_event_dispatch.has_value() &&
        root_record->last_event_dispatch->sequence == 1 &&
        child_record->last_event_dispatch->sequence == 2;
  };

  const int result = runtime.run(
      cgpui::WindowDescriptor{
          .title = "Root",
          .size = {.width = 640.0F, .height = 480.0F}},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  if (result != 0) return 2;
  if (!root_record_isolated) return 3;
  if (!child_record_isolated) return 4;
  if (!sequences_isolated) return 5;
  if (!root_callback_isolated) return 6;
  if (!child_callback_isolated) return 7;
  return renderers.size() == 2 ? 0 : 8;
}
