#include "cgpui/cgpui.hpp"

#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <vector>

namespace {

struct Native {
  cgpui::PlatformEventCallback callback;
  int redraws = 0;
  bool redraw_pending = false;
};
class Window final : public cgpui::PlatformWindow {
 public:
  explicit Window(Native& native) : native_(native) {}
  cgpui::NativeSurfaceHandle native_surface() const override { return {}; }
  cgpui::WindowState state() const override {
    return {.framebuffer_size = {160, 90}, .scale = {1}};
  }
  void request_redraw() override {
    ++native_.redraws;
    native_.redraw_pending = true;
  }
  void request_close() override {}
  void set_title(std::string_view) override {}
  void set_cursor(cgpui::CursorShape) override {}
  void set_ime_text_input_placement(std::optional<cgpui::ImeTextInputPlacement>) override {}
 private:
  Native& native_;
};
class Application final : public cgpui::PlatformApplication {
 public:
  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_window(
      const cgpui::WindowDescriptor&, cgpui::PlatformEventCallback callback) override {
    root.callback = std::move(callback);
    return std::make_unique<Window>(root);
  }
  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_child_window(
      const cgpui::WindowDescriptor&, cgpui::PlatformWindow&,
      cgpui::PlatformEventCallback callback) override {
    child.callback = std::move(callback);
    return std::make_unique<Window>(child);
  }
  int run() override { on_run(); return 0; }
  void quit() override { ++quits; }
  void request_wakeup() override { wakeup_pending = true; }
  bool pump_wakeups() {
    for (int turn = 0; wakeup_pending && turn < 8; ++turn) {
      wakeup_pending = false;
      root.callback(cgpui::WindowWakeupRequested{});
      if (child.callback) child.callback(cgpui::WindowWakeupRequested{});
    }
    return !wakeup_pending;
  }
  bool paint_if_requested(Native& target) {
    if (!target.redraw_pending) return false;
    target.redraw_pending = false;
    target.callback(cgpui::WindowRedrawRequested{});
    return true;
  }
  Native root, child;
  std::function<void()> on_run;
  int quits = 0;
  bool wakeup_pending = false;
};
class View final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}
};
class Frame final : public cgpui::RenderFrame {
 public:
  Frame(int& attempts, cgpui::ErrorCode code) : attempts_(attempts), code_(code) {}
  void clear(cgpui::Color) override {}
  void draw_rect(const cgpui::SolidRect&) override {}
  cgpui::Result<void> present() override {
    if (++attempts_ != 1) return {};
    return std::unexpected(cgpui::Error{
        .code = code_,
        .message = code_ == cgpui::ErrorCode::frame_retry_required
            ? "opaque diagnostic: the typed result alone controls retry"
            : "swapchain was recreated before frame acquisition; retry the frame"});
  }
 private:
  int& attempts_;
  cgpui::ErrorCode code_;
};
class Renderer final : public cgpui::Renderer {
 public:
  explicit Renderer(cgpui::ErrorCode code) : code_(code) {}
  cgpui::Result<void> resize(cgpui::Size, cgpui::DpiScale) override { return {}; }
  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    return std::make_unique<Frame>(attempts, code_);
  }
  int attempts = 0;
 private:
  cgpui::ErrorCode code_;
};

int retry_case(bool child, cgpui::ErrorCode code) {
  const bool fatal = code != cgpui::ErrorCode::frame_retry_required;
  Application app;
  View view;
  std::vector<std::unique_ptr<Renderer>> renderers;
  cgpui::WindowRuntime runtime(app, view,
      [&](const cgpui::RenderSurfaceDescriptor&) -> cgpui::Result<cgpui::Renderer*> {
        renderers.push_back(std::make_unique<Renderer>(code));
        return renderers.back().get();
      });
  int failure = 0;
  app.on_run = [&] {
    if (child && !runtime.try_open_window(cgpui::WindowOptions{}, std::make_unique<View>())) {
      failure = 1; return;
    }
    Native& target = child ? app.child : app.root;
    const int redraws = target.redraws;
    const int other_redraws = child ? app.root.redraws : app.child.redraws;
    target.redraw_pending = true; // The initial OS paint, not a retry request.
    if (!app.paint_if_requested(target)) { failure = 1; return; }
    if (fatal) {
      if (app.quits != 1 || renderers.back()->attempts != 1) failure = 2;
      return;
    }
    // Native requests made inside the child callback are deferred to the
    // root wakeup, then queued as a later paint instead of drawn recursively.
    if ((child && !app.wakeup_pending) || !app.pump_wakeups() ||
        app.quits != 0 || target.redraws != redraws + 1 || !target.redraw_pending ||
        (child ? app.root.redraws : app.child.redraws) != other_redraws) {
      failure = 3; return;
    }
    if (!app.paint_if_requested(target) || renderers.back()->attempts != 2 ||
        target.redraw_pending || app.quits != 0 ||
        (child ? app.root.redraws : app.child.redraws) != other_redraws) failure = 4;
  };
  const int result = runtime.run({}, {.request_initial_redraw = false});
  if (failure != 0 || result != (fatal ? 1 : 0)) {
    std::cerr << "frame retry child=" << child << " fatal=" << fatal
              << " failure=" << failure << " exit=" << result << '\n';
    return 10 + failure;
  }
  return 0;
}

} // namespace

int main() {
  for (bool child : {false, true}) {
    for (auto code : {cgpui::ErrorCode::frame_retry_required,
                      cgpui::ErrorCode::frame_acquisition_failed,
                      cgpui::ErrorCode::renderer_initialization_failed}) {
      if (int result = retry_case(child, code)) return result;
    }
  }
  return 0;
}
