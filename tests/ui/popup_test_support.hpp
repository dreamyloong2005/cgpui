#pragma once

#include "cgpui/cgpui.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace popup_test {

struct NativeWindow {
  cgpui::WindowState state;
  cgpui::WindowDescriptor descriptor;
  cgpui::PlatformEventCallback callback;
  bool open = false;
  bool placement_succeeds = true;
  int placement_attempts = 0;
};

class Window final : public cgpui::PlatformWindow {
 public:
  explicit Window(NativeWindow& native) : native_(native) { native_.open = true; }
  ~Window() override { native_.open = false; }
  cgpui::NativeSurfaceHandle native_surface() const override { return {}; }
  cgpui::WindowState state() const override { return native_.state; }
  void request_redraw() override {}
  void request_close() override {}
  void set_title(std::string_view) override {}
  void set_cursor(cgpui::CursorShape) override {}
  void set_ime_text_input_placement(std::optional<cgpui::ImeTextInputPlacement>) override {}
  cgpui::PlatformPopupPlacementResult place_popup(
      cgpui::WindowPopupPlacement placement) override {
    ++native_.placement_attempts;
    if (!native_.placement_succeeds) {
      return {.supported = true, .reason = "test popup placement failure"};
    }
    return {.supported = true, .placed = true,
            .bounds = {placement.anchor.origin, native_.descriptor.size}};
  }

 private:
  NativeWindow& native_;
};

class Application final : public cgpui::PlatformApplication {
 public:
  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_window(
      const cgpui::WindowDescriptor& descriptor,
      cgpui::PlatformEventCallback callback) override {
    root = {.state = {.framebuffer_size = descriptor.size, .scale = {1.0F}},
            .descriptor = descriptor, .callback = std::move(callback)};
    return std::make_unique<Window>(root);
  }
  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_child_window(
      const cgpui::WindowDescriptor& descriptor, cgpui::PlatformWindow&,
      cgpui::PlatformEventCallback callback) override {
    ++child_creation_attempts;
    if (!child_creation_succeeds) {
      return std::unexpected(cgpui::Error{
          .code = cgpui::ErrorCode::window_creation_failed,
          .message = "test child window creation failure"});
    }
    children.push_back(std::make_unique<NativeWindow>(NativeWindow{
        .state = {.framebuffer_size = descriptor.size, .scale = {1.0F}},
        .descriptor = descriptor, .callback = std::move(callback),
        .placement_succeeds = popup_placement_succeeds}));
    return std::make_unique<Window>(*children.back());
  }
  bool popup_windows_supported() const override { return supports_popups; }
  int run() override { on_run(); return 0; }
  void quit() override {}

  bool supports_popups = true;
  bool child_creation_succeeds = true;
  bool popup_placement_succeeds = true;
  int child_creation_attempts = 0;
  NativeWindow root;
  std::vector<std::unique_ptr<NativeWindow>> children;
  std::function<void()> on_run;
};

class Frame final : public cgpui::RenderFrame {
 public:
  void clear(cgpui::Color) override {}
  void draw_rect(const cgpui::SolidRect&) override {}
  cgpui::Result<void> present() override { return {}; }
};

class Renderer final : public cgpui::Renderer {
 public:
  explicit Renderer(bool supported) : supported_(supported) {}
  bool popup_windows_supported() const override { return supported_; }
  cgpui::Result<void> resize(cgpui::Size, cgpui::DpiScale) override { return {}; }
  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    return std::make_unique<Frame>();
  }

 private:
  bool supported_;
};

class CountingView final : public cgpui::View {
 public:
  explicit CountingView(int& alive) : alive_(alive) { ++alive_; }
  ~CountingView() override { --alive_; }
  void paint(cgpui::PaintList&, cgpui::Size) override {}

 private:
  int& alive_;
};

inline cgpui::WindowPopupPlacement placement() {
  return {.anchor = {{10, 20}, {30, 10}}, .side = cgpui::WindowPopupSide::below};
}

inline cgpui::WindowOptions options() {
  return cgpui::WindowOptions{}.size(120, 80).popup(placement());
}

} // namespace popup_test
