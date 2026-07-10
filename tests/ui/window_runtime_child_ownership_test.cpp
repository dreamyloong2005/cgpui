#include "window_runtime_test_support.hpp"

namespace {

class BorrowedPlatformWindow final : public cgpui::PlatformWindow {
 public:
  explicit BorrowedPlatformWindow(FakeWindow& window) : window_(window) {}

  cgpui::NativeSurfaceHandle native_surface() const override {
    return window_.native_surface();
  }
  cgpui::WindowState state() const override { return window_.state(); }
  void request_redraw() override { window_.request_redraw(); }
  void request_close() override { window_.request_close(); }
  void set_title(std::string_view title) override { window_.set_title(title); }
  void set_cursor(cgpui::CursorShape cursor) override {
    window_.set_cursor(cursor);
  }
  void set_ime_text_input_placement(
      std::optional<cgpui::ImeTextInputPlacement> placement) override {
    window_.set_ime_text_input_placement(std::move(placement));
  }

 private:
  FakeWindow& window_;
};

class OwnershipApplication final : public cgpui::PlatformApplication {
 public:
  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_window(
      const cgpui::WindowDescriptor&,
      cgpui::PlatformEventCallback callback) override {
    creation_order.push_back("root");
    root.callback = std::move(callback);
    auto window = std::make_unique<BorrowedPlatformWindow>(root);
    root_platform_window = window.get();
    return window;
  }

  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_child_window(
      const cgpui::WindowDescriptor&,
      cgpui::PlatformWindow& parent,
      cgpui::PlatformEventCallback callback) override {
    creation_order.push_back("child");
    received_root_parent = &parent == root_platform_window;
    child.callback = std::move(callback);
    return std::make_unique<BorrowedPlatformWindow>(child);
  }

  int run() override {
    if (on_run) {
      on_run();
    }
    return 0;
  }
  void quit() override {}

  FakeWindow root{cgpui::WindowState{
      .framebuffer_size = {640.0F, 480.0F},
      .scale = cgpui::DpiScale{1.0F}}};
  FakeWindow child{cgpui::WindowState{
      .framebuffer_size = {320.0F, 240.0F},
      .scale = cgpui::DpiScale{1.0F}}};
  cgpui::PlatformWindow* root_platform_window = nullptr;
  std::vector<std::string> creation_order;
  std::function<void()> on_run;
  bool received_root_parent = false;
};

} // namespace

int main() {
  OwnershipApplication app;
  RecordingView view;
  RecordingFrame frame;
  std::vector<std::unique_ptr<RecordingRenderer>> renderers;
  cgpui::WindowRuntime runtime(
      app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<cgpui::Renderer*> {
        auto renderer = std::make_unique<RecordingRenderer>(frame);
        cgpui::Renderer* raw = renderer.get();
        renderers.push_back(std::move(renderer));
        return raw;
      });
  const cgpui::AppOpenedWindow opened = runtime.open_window(
      cgpui::WindowOptions{}.title("Owned Child").size(320.0F, 240.0F));
  const cgpui::WindowRuntimeRecord* pending =
      runtime.window_runtime_record(opened.runtime_id);
  if (pending == nullptr || pending->active || pending->window != nullptr ||
      opened.parent_runtime_id != runtime.root_window_runtime_id() ||
      pending->parent_runtime_id != runtime.root_window_runtime_id()) {
    return 1;
  }
  bool active_during_run = false;
  app.on_run = [&] {
    const cgpui::WindowRuntimeRecord* record =
        runtime.window_runtime_record(opened.runtime_id);
    active_during_run = record != nullptr && record->active &&
        record->window != nullptr && record->renderer != nullptr;
  };
  const int result = runtime.run(
      cgpui::WindowDescriptor{.title = "Root"},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  return result == 0 && app.received_root_parent && active_during_run &&
          app.creation_order == std::vector<std::string>{"root", "child"} &&
          renderers.size() == 2
      ? 0
      : 2;
}
