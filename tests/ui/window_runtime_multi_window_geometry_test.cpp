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
  int redraw_request_count = 0;

  void resize(cgpui::Size size, cgpui::DpiScale scale) {
    state.framebuffer_size = size;
    state.scale = scale;
    callback(cgpui::WindowResized{.size = size, .scale = scale});
  }
  void redraw() { callback(cgpui::WindowRedrawRequested{}); }
};

class GeometryWindow final : public cgpui::PlatformWindow {
 public:
  explicit GeometryWindow(NativeWindow& window) : window_(window) {}
  cgpui::NativeSurfaceHandle native_surface() const override { return {}; }
  cgpui::WindowState state() const override { return window_.state; }
  void request_redraw() override { ++window_.redraw_request_count; }
  void request_close() override {}
  void set_title(std::string_view) override {}
  void set_cursor(cgpui::CursorShape) override {}
  void set_ime_text_input_placement(
      std::optional<cgpui::ImeTextInputPlacement>) override {}

 private:
  NativeWindow& window_;
};

class GeometryApplication final : public cgpui::PlatformApplication {
 public:
  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_window(
      const cgpui::WindowDescriptor& descriptor,
      cgpui::PlatformEventCallback callback) override {
    root.state = {.framebuffer_size = descriptor.size,
                  .scale = cgpui::DpiScale{1.0F}};
    root.callback = std::move(callback);
    return std::make_unique<GeometryWindow>(root);
  }
  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_child_window(
      const cgpui::WindowDescriptor& descriptor,
      cgpui::PlatformWindow&,
      cgpui::PlatformEventCallback callback) override {
    child.state = {.framebuffer_size = descriptor.size,
                   .scale = cgpui::DpiScale{1.0F}};
    child.callback = std::move(callback);
    return std::make_unique<GeometryWindow>(child);
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

class GeometryFrame final : public cgpui::RenderFrame {
 public:
  void clear(cgpui::Color) override {}
  void draw_rect(const cgpui::SolidRect&) override {}
  cgpui::Result<void> present() override { return {}; }
};

class GeometryRenderer final : public cgpui::Renderer {
 public:
  cgpui::Result<void> resize(
      cgpui::Size size,
      cgpui::DpiScale scale) override {
    ++resize_count;
    last_resize = size;
    last_scale = scale;
    return {};
  }
  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    ++begin_frame_count;
    return std::make_unique<GeometryFrame>();
  }

  int resize_count = 0;
  int begin_frame_count = 0;
  cgpui::Size last_resize{};
  cgpui::DpiScale last_scale{};
};

class GeometryView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}
};

} // namespace

int main() {
  GeometryApplication application;
  GeometryView root_view;
  std::vector<std::unique_ptr<GeometryRenderer>> renderers;
  cgpui::WindowRuntime runtime(
      application,
      root_view,
      [&](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<cgpui::Renderer*> {
        auto renderer = std::make_unique<GeometryRenderer>();
        cgpui::Renderer* raw = renderer.get();
        renderers.push_back(std::move(renderer));
        return raw;
      });
  const cgpui::AppOpenedWindow opened = runtime.open_window(
      cgpui::WindowOptions{}.title("Geometry Child").size(320.0F, 200.0F),
      std::make_unique<GeometryView>());

  bool root_pending_before = false;
  bool root_pending_after = false;
  bool child_record_resized = false;
  bool child_context_resized = false;
  bool child_scheduled_during_render = false;
  runtime.set_after_render_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::RenderRecord&) {
        const cgpui::WindowRuntimeRecord* record =
            runtime.window_runtime_record(opened.runtime_id);
        child_scheduled_during_render =
            context.window_runtime_id == opened.runtime_id &&
            record != nullptr && record->redraw_scheduled;
      });
  runtime.set_after_frame_callback(
      [&](const cgpui::WindowRuntimeContext& context) {
        if (context.window_runtime_id == opened.runtime_id) {
          child_context_resized =
              context.viewport_size.width == 400.0F &&
              context.viewport_size.height == 300.0F &&
              context.scale.value == 2.0F;
        }
      });
  application.on_run = [&] {
    runtime.request_render();
    root_pending_before = runtime.invalidation_state().render &&
        runtime.invalidation_state().layout &&
        runtime.invalidation_state().paint &&
        application.root.redraw_request_count == 1;

    application.child.resize(
        {.width = 800.0F, .height = 600.0F},
        cgpui::DpiScale{2.0F});
    const cgpui::WindowRuntimeRecord* record =
        runtime.window_runtime_record(opened.runtime_id);
    child_record_resized = record != nullptr &&
        record->framebuffer_size.width == 800.0F &&
        record->viewport_size.width == 400.0F &&
        record->scale.value == 2.0F && !record->redraw_scheduled;
    application.child.redraw();
    record = runtime.window_runtime_record(opened.runtime_id);
    child_record_resized = child_record_resized && record != nullptr &&
        !record->redraw_scheduled;
    root_pending_after = runtime.invalidation_state().render &&
        runtime.invalidation_state().layout &&
        runtime.invalidation_state().paint &&
        application.root.redraw_request_count == 1;
  };

  const int result = runtime.run(
      cgpui::WindowDescriptor{
          .title = "Root",
          .size = {.width = 640.0F, .height = 480.0F}},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  return result == 0 && root_pending_before && root_pending_after &&
          child_record_resized && child_context_resized &&
          child_scheduled_during_render &&
          renderers.size() == 2 && renderers[0]->resize_count == 0 &&
          renderers[0]->begin_frame_count == 0 &&
          renderers[1]->resize_count == 1 &&
          renderers[1]->begin_frame_count == 1 &&
          renderers[1]->last_resize.width == 800.0F &&
          renderers[1]->last_scale.value == 2.0F
      ? 0
      : 1;
}
