#include "cgpui/cgpui.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

// WindowRuntime popups on a fake platform: open as WindowKind::popup with
// a transparent renderer, placement and redraw forwarding, keyboard events
// of the root window going to the popup that takes the keyboard, closing.
namespace {

struct NativeWindow {
  cgpui::WindowState state;
  cgpui::WindowDescriptor descriptor;
  cgpui::PlatformEventCallback callback;
  std::optional<cgpui::WindowPopupPlacement> placed;
  int redraws = 0;
};

class FakeWindow final : public cgpui::PlatformWindow {
 public:
  explicit FakeWindow(NativeWindow& window) : window_(window) {}
  cgpui::NativeSurfaceHandle native_surface() const override { return {}; }
  cgpui::WindowState state() const override { return window_.state; }
  void request_redraw() override { ++window_.redraws; }
  void request_close() override {}
  void set_title(std::string_view) override {}
  void set_cursor(cgpui::CursorShape) override {}
  void set_ime_text_input_placement(std::optional<cgpui::ImeTextInputPlacement>) override {}
  cgpui::PlatformPopupPlacementResult place_popup(
      cgpui::WindowPopupPlacement placement) override {
    window_.placed = placement;
    return {.supported = true, .placed = true, .bounds = placement.anchor};
  }

 private:
  NativeWindow& window_;
};

class FakeApplication final : public cgpui::PlatformApplication {
 public:
  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_window(
      const cgpui::WindowDescriptor& descriptor,
      cgpui::PlatformEventCallback callback) override {
    root = {.state = {.framebuffer_size = descriptor.size, .scale = {1.0F}},
            .descriptor = descriptor, .callback = std::move(callback)};
    return std::make_unique<FakeWindow>(root);
  }
  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_child_window(
      const cgpui::WindowDescriptor& descriptor, cgpui::PlatformWindow&,
      cgpui::PlatformEventCallback callback) override {
    popup = {.state = {.framebuffer_size = descriptor.size, .scale = {1.0F}},
             .descriptor = descriptor, .callback = std::move(callback)};
    return std::make_unique<FakeWindow>(popup);
  }
  bool popup_windows_supported() const override { return supported; }
  int run() override {
    on_run();
    return 0;
  }
  void quit() override {}

  bool supported = true;
  NativeWindow root;
  NativeWindow popup;
  std::function<void()> on_run;
};

class FakeFrame final : public cgpui::RenderFrame {
 public:
  void clear(cgpui::Color) override {}
  void draw_rect(const cgpui::SolidRect&) override {}
  cgpui::Result<void> present() override { return {}; }
};

class FakeRenderer final : public cgpui::Renderer {
 public:
  cgpui::Result<void> resize(cgpui::Size, cgpui::DpiScale) override { return {}; }
  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    return std::make_unique<FakeFrame>();
  }
};

class KeyCountingView final : public cgpui::View {
 public:
  explicit KeyCountingView(int& keys) : keys_(keys) {}
  cgpui::EventResult handle_event(const cgpui::PlatformEvent& event,
                                  const cgpui::WindowRuntimeContext&) override {
    if (std::holds_alternative<cgpui::KeyboardKey>(event)) ++keys_;
    return cgpui::EventResult::unhandled();
  }
  void paint(cgpui::PaintList&, cgpui::Size) override {}

 private:
  int& keys_;
};

}  // namespace

int main() {
  FakeApplication application;
  int root_keys = 0;
  int popup_keys = 0;
  KeyCountingView root_view(root_keys);
  std::vector<std::unique_ptr<FakeRenderer>> renderers;
  std::vector<bool> transparent;
  cgpui::WindowRuntime runtime(
      application, root_view,
      [&](const cgpui::RenderSurfaceDescriptor& descriptor)
          -> cgpui::Result<cgpui::Renderer*> {
        transparent.push_back(descriptor.transparent_background);
        renderers.push_back(std::make_unique<FakeRenderer>());
        return renderers.back().get();
      });
  const cgpui::KeyboardKey key{.key_code = 13, .action = cgpui::KeyAction::pressed};
  int failure = 0;
  application.on_run = [&] {
    const cgpui::WindowPopupPlacement placement{
        .anchor = {{10, 20}, {30, 10}}, .side = cgpui::WindowPopupSide::below, .gap = 2};
    application.supported = false;
    const auto unsupported = cgpui::open_popup(runtime,
        placement, {120, 80}, std::make_unique<KeyCountingView>(popup_keys));
    if (unsupported || unsupported.error().code != cgpui::ErrorCode::unsupported_platform) {
      failure = 1;
      return;
    }
    application.supported = true;
    const auto opened = cgpui::open_popup(runtime,
        placement, {120, 80}, std::make_unique<KeyCountingView>(popup_keys),
        cgpui::WindowPopupOptions{.takes_keyboard = true});
    const auto& descriptor = application.popup.descriptor;
    if (!opened || descriptor.kind != cgpui::WindowKind::popup || !descriptor.popup ||
        descriptor.popup->anchor.origin.y != 20.0F || descriptor.size.width != 120.0F ||
        transparent.size() != 2 || transparent[0] || !transparent[1]) {
      failure = 2;
      return;
    }
    // The root window's key events go to the popup while it is open.
    application.root.callback(key);
    if (popup_keys != 1 || root_keys != 0) {
      failure = 3;
      return;
    }
    const auto moved = cgpui::place_popup(runtime,
        opened->runtime_id, {.anchor = {{50, 60}, {10, 10}}, .side = cgpui::WindowPopupSide::right});
    if (!moved.placed || !application.popup.placed ||
        application.popup.placed->side != cgpui::WindowPopupSide::right ||
        !cgpui::request_popup_redraw(runtime, opened->runtime_id) || application.popup.redraws != 1) {
      failure = 4;
      return;
    }
    // Closing returns the keyboard to the root window.
    if (!cgpui::close_popup(runtime, opened->runtime_id) || cgpui::close_popup(runtime, opened->runtime_id) ||
        cgpui::place_popup(runtime, opened->runtime_id, placement).placed) {
      failure = 5;
      return;
    }
    application.root.callback(key);
    if (root_keys != 1 || popup_keys != 1) failure = 6;
  };
  const int result = runtime.run(
      cgpui::WindowDescriptor{.title = "Root", .size = {640.0F, 480.0F}},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  return result == 0 ? failure : 10;
}
