#include "cgpui/cgpui.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace {

struct NativeWindowState {
  cgpui::WindowState state;
  cgpui::PlatformEventCallback callback;
  bool callback_active = false;
  bool destroyed = false;
  bool destroyed_during_callback = false;

  void emit(const cgpui::PlatformEvent& event) {
    callback_active = true;
    callback(event);
    callback_active = false;
  }
};

class ChurnWindow final : public cgpui::PlatformWindow {
 public:
  explicit ChurnWindow(std::shared_ptr<NativeWindowState> state)
      : state_(std::move(state)) {}
  ~ChurnWindow() override {
    state_->destroyed = true;
    state_->destroyed_during_callback = state_->callback_active;
  }
  cgpui::NativeSurfaceHandle native_surface() const override { return {}; }
  cgpui::WindowState state() const override { return state_->state; }
  void request_redraw() override {}
  void request_close() override {}
  void set_title(std::string_view) override {}
  void set_cursor(cgpui::CursorShape) override {}
  void set_ime_text_input_placement(
      std::optional<cgpui::ImeTextInputPlacement>) override {}

 private:
  std::shared_ptr<NativeWindowState> state_;
};

class ChurnApplication final : public cgpui::PlatformApplication {
 public:
  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_window(
      const cgpui::WindowDescriptor& descriptor,
      cgpui::PlatformEventCallback callback) override {
    root = std::make_shared<NativeWindowState>();
    root->state = {.framebuffer_size = descriptor.size,
                   .scale = cgpui::DpiScale{1.0F}};
    root->callback = std::move(callback);
    return std::make_unique<ChurnWindow>(root);
  }
  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_child_window(
      const cgpui::WindowDescriptor& descriptor,
      cgpui::PlatformWindow&,
      cgpui::PlatformEventCallback callback) override {
    auto child = std::make_shared<NativeWindowState>();
    child->state = {.framebuffer_size = descriptor.size,
                    .scale = cgpui::DpiScale{1.0F}};
    child->callback = std::move(callback);
    children.push_back(child);
    return std::make_unique<ChurnWindow>(std::move(child));
  }
  int run() override {
    on_run();
    return 0;
  }
  void quit() override {}
  void request_wakeup() override { ++wakeup_requests; }

  std::shared_ptr<NativeWindowState> root;
  std::vector<std::shared_ptr<NativeWindowState>> children;
  std::function<void()> on_run;
  int wakeup_requests = 0;
};

class ChurnFrame final : public cgpui::RenderFrame {
 public:
  void clear(cgpui::Color) override {}
  void draw_rect(const cgpui::SolidRect&) override {}
  cgpui::Result<void> present() override { return {}; }
};

class ChurnRenderer final : public cgpui::Renderer {
 public:
  cgpui::Result<void> resize(cgpui::Size, cgpui::DpiScale) override {
    return {};
  }
  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    return std::make_unique<ChurnFrame>();
  }
};

class ChurnView final : public cgpui::View {
 public:
  explicit ChurnView(int& destruction_count)
      : destruction_count_(&destruction_count) {}
  ~ChurnView() override { ++*destruction_count_; }
  void paint(cgpui::PaintList&, cgpui::Size) override {}

 private:
  int* destruction_count_;
};

} // namespace

int main() {
  ChurnApplication application;
  int root_view_destruction_count = 0;
  ChurnView root_view(root_view_destruction_count);
  ChurnRenderer renderer;
  int renderer_factory_count = 0;
  int child_view_destruction_count = 0;
  cgpui::WindowRuntime runtime(
      application,
      root_view,
      [&](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<cgpui::Renderer*> {
        ++renderer_factory_count;
        return &renderer;
      });
  const cgpui::ThemeTokenId accent = cgpui::theme_token("churn-accent");
  cgpui::App app{runtime};
  int failure = 0;

  application.on_run = [&] {
    for (int index = 0; index < 12 && failure == 0; ++index) {
      auto child_view =
          std::make_unique<ChurnView>(child_view_destruction_count);
      const cgpui::AppOpenedWindow opened = runtime.open_window(
          cgpui::WindowOptions{}.title("Churn Child").size(160.0F, 90.0F),
          std::move(child_view));
      auto child = application.children.back();
      cgpui::Theme child_theme;
      child_theme.set_color(accent, cgpui::rgb(10, 20, 30));
      runtime.set_window_theme(opened.runtime_id, std::move(child_theme));

      child->emit(cgpui::WindowCloseRequested{});
      const cgpui::WindowRuntimeRecord* closing =
          runtime.window_runtime_record(opened.runtime_id);
      const std::optional<cgpui::Window> closing_window =
          app.window(opened.runtime_id);
      if (closing == nullptr || closing->active ||
          closing->window != nullptr || !closing_window.has_value() ||
          closing_window->active() || child->destroyed ||
          child_view_destruction_count != index + 1) {
        failure = 1;
        return;
      }

      application.root->emit(cgpui::WindowWakeupRequested{});
      child->emit(cgpui::KeyboardKey{
          .key_code = 65,
          .action = cgpui::KeyAction::pressed,
      });
      if (!child->destroyed || child->destroyed_during_callback ||
          runtime.window_runtime_record(opened.runtime_id) != nullptr ||
          app.window(opened.runtime_id).has_value() ||
          !runtime.app_opened_windows().empty() ||
          runtime.window_runtime_records().size() != 1 ||
          runtime.find_view(opened.root_view_id) != nullptr ||
          runtime.is_view_id_allocated(opened.root_view_id) ||
          runtime.window_theme(opened.runtime_id) != nullptr) {
        failure = 2;
        return;
      }
    }
  };

  const int result = runtime.run(
      cgpui::WindowDescriptor{
          .title = "Root",
          .size = {.width = 640.0F, .height = 480.0F}},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  return result == 0 && failure == 0 &&
          child_view_destruction_count == 12 &&
          renderer_factory_count == 13 && application.wakeup_requests == 12
      ? 0
      : failure == 0 ? 3 : failure;
}
