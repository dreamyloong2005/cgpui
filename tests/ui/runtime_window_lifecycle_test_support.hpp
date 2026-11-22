#pragma once

#include "cgpui/cgpui.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

namespace runtime_window_test {

struct Counters {
  int live_renderers = 0;
  int created_renderers = 0;
  int destroyed_renderers = 0;
  unsigned sequence = 0;
};

struct NativeState {
  cgpui::WindowState state;
  cgpui::WindowDescriptor descriptor;
  cgpui::PlatformEventCallback callback;
  cgpui::PlatformWindow* native_address = nullptr;
  Counters* counters = nullptr;
  bool destroyed = false;
  bool destroyed_during_callback = false;
  bool renderer_destroyed_after_window = false;
  int callback_depth = 0;
  int redraws = 0;
  int frames = 0;
  int presents = 0;
  int live_renderers = 0;
  unsigned window_destroy_order = 0;
  unsigned renderer_destroy_order = 0;

  void emit(const cgpui::PlatformEvent& event) {
    ++callback_depth;
    callback(event);
    --callback_depth;
  }
};

class Window final : public cgpui::PlatformWindow {
 public:
  explicit Window(std::shared_ptr<NativeState> state) : state_(std::move(state)) {
    state_->native_address = this;
  }
  ~Window() override {
    state_->destroyed = true;
    state_->destroyed_during_callback = state_->callback_depth != 0;
    state_->window_destroy_order = ++state_->counters->sequence;
  }
  cgpui::NativeSurfaceHandle native_surface() const override { return {}; }
  cgpui::WindowState state() const override { return state_->state; }
  void request_redraw() override { ++state_->redraws; }
  void request_close() override {}
  void set_title(std::string_view) override {}
  void set_cursor(cgpui::CursorShape) override {}
  void set_ime_text_input_placement(std::optional<cgpui::ImeTextInputPlacement>) override {}
  cgpui::PlatformPopupPlacementResult place_popup(cgpui::WindowPopupPlacement placement) override {
    return {.supported = true, .placed = true,
            .bounds = {placement.anchor.origin, state_->descriptor.size}};
  }

 private:
  std::shared_ptr<NativeState> state_;
};

class Application final : public cgpui::PlatformApplication {
 public:
  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_window(
      const cgpui::WindowDescriptor& descriptor, cgpui::PlatformEventCallback callback) override {
    root = make_state(descriptor, std::move(callback));
    return std::make_unique<Window>(root);
  }
  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_child_window(
      const cgpui::WindowDescriptor& descriptor, cgpui::PlatformWindow&,
      cgpui::PlatformEventCallback callback) override {
    if (fail_child_creation) {
      return std::unexpected(cgpui::Error{.code = cgpui::ErrorCode::window_creation_failed,
                                        .message = "injected child creation failure"});
    }
    children.push_back(make_state(descriptor, std::move(callback)));
    return std::make_unique<Window>(children.back());
  }
  bool popup_windows_supported() const override { return true; }
  std::uint64_t monotonic_time_ms() const override { return 0; }
  int run() override { if (on_run) on_run(); return 0; }
  void quit() override { ++quits; }
  void request_wakeup() override { ++wakeups; }
  void wake() { root->emit(cgpui::WindowWakeupRequested{}); }

  Counters counters;
  std::shared_ptr<NativeState> root;
  std::shared_ptr<NativeState> last_created;
  std::vector<std::shared_ptr<NativeState>> children;
  std::function<void()> on_run;
  bool fail_child_creation = false;
  int wakeups = 0;
  int quits = 0;

 private:
  std::shared_ptr<NativeState> make_state(const cgpui::WindowDescriptor& descriptor,
                                        cgpui::PlatformEventCallback callback) {
    last_created = std::make_shared<NativeState>();
    last_created->state = {.framebuffer_size = descriptor.size, .scale = {1.0F}};
    last_created->descriptor = descriptor;
    last_created->callback = std::move(callback);
    last_created->counters = &counters;
    return last_created;
  }
};

class Frame final : public cgpui::RenderFrame {
 public:
  explicit Frame(std::shared_ptr<NativeState> state) : state_(std::move(state)) {}
  void clear(cgpui::Color) override {}
  void draw_rect(const cgpui::SolidRect&) override {}
  cgpui::Result<void> present() override { ++state_->presents; return {}; }
 private:
  std::shared_ptr<NativeState> state_;
};

class Renderer final : public cgpui::Renderer {
 public:
  explicit Renderer(std::shared_ptr<NativeState> state, bool popup_support = true)
      : state_(std::move(state)), popup_support_(popup_support) {
    ++state_->live_renderers;
    ++state_->counters->live_renderers;
    ++state_->counters->created_renderers;
  }
  ~Renderer() override {
    --state_->live_renderers;
    --state_->counters->live_renderers;
    ++state_->counters->destroyed_renderers;
    state_->renderer_destroyed_after_window = state_->destroyed;
    state_->renderer_destroy_order = ++state_->counters->sequence;
  }
  bool popup_windows_supported() const override { return popup_support_; }
  cgpui::Result<void> resize(cgpui::Size, cgpui::DpiScale) override { return {}; }
  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    ++state_->frames;
    return std::make_unique<Frame>(state_);
  }
 private:
  std::shared_ptr<NativeState> state_;
  bool popup_support_;
};

class View final : public cgpui::View {
 public:
  ~View() override { if (on_destroy) on_destroy(); }
  cgpui::AnyElement render(cgpui::ViewContext& context) override {
    if (on_render) on_render(context);
    return {};
  }
  void paint(cgpui::PaintList&, cgpui::Size) override { ++paints; }
  cgpui::EventResult handle_event(const cgpui::PlatformEvent& event,
                                 const cgpui::WindowRuntimeContext& context) override {
    return on_event ? on_event(event, context) : cgpui::EventResult::unhandled();
  }
  std::function<void()> on_destroy;
  std::function<void(cgpui::ViewContext&)> on_render;
  std::function<cgpui::EventResult(const cgpui::PlatformEvent&,
                                  const cgpui::WindowRuntimeContext&)> on_event;
  int paints = 0;
};

inline cgpui::KeyboardKey key() {
  return {.key_code = 65, .action = cgpui::KeyAction::pressed};
}
inline cgpui::WindowOptions popup_options() {
  return cgpui::WindowOptions{}.size(120, 80).popup(
      {.anchor = {{10, 20}, {30, 10}}, .side = cgpui::WindowPopupSide::below});
}

} // namespace runtime_window_test
