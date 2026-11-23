#pragma once

#include "cgpui/cgpui.hpp"
#include <functional>
#include <iostream>
#include <memory>
#include <vector>

namespace runtime_clear_test {
using namespace cgpui;

struct Native {
  WindowDescriptor descriptor;
  PlatformEventCallback callback;
  PlatformWindowBackdropState backdrop;
  bool material_visible = true;
  std::vector<Color> clears;
  unsigned presents = 0;
  void paint() { callback(WindowRedrawRequested{}); }
};

class Platform final : public PlatformWindow {
 public:
  explicit Platform(Native& native) : native_(native) {}
  NativeSurfaceHandle native_surface() const override { return {}; }
  WindowState state() const override { return {.framebuffer_size = {160, 90}, .scale = {1}}; }
  void request_redraw() override {}
  void request_close() override {}
  void set_title(std::string_view) override {}
  void set_cursor(CursorShape) override {}
  void set_ime_text_input_placement(std::optional<ImeTextInputPlacement>) override {}
  PlatformWindowBackdropState apply_window_backdrop(WindowBackdrop requested, bool alpha) override {
    native_.backdrop = {.supported = true, .transparent_surface = alpha,
        .requested = requested,
        .applied = alpha && native_.material_visible ? requested : WindowBackdrop::none};
    return native_.backdrop;
  }
  PlatformWindowBackdropState window_backdrop_state() const override { return native_.backdrop; }
 private:
  Native& native_;
};

class Application final : public PlatformApplication {
 public:
  Result<std::unique_ptr<PlatformWindow>> create_window(
      const WindowDescriptor& descriptor, PlatformEventCallback callback) override {
    root.descriptor = descriptor;
    root.callback = std::move(callback);
    last = &root;
    return std::make_unique<Platform>(root);
  }
  Result<std::unique_ptr<PlatformWindow>> create_child_window(
      const WindowDescriptor& descriptor, PlatformWindow&, PlatformEventCallback callback) override {
    child.descriptor = descriptor;
    child.callback = std::move(callback);
    last = &child;
    return std::make_unique<Platform>(child);
  }
  int run() override { on_run(); return 0; }
  void quit() override { ++quits; }
  Native root, child;
  Native* last = nullptr;
  std::function<void()> on_run;
  unsigned quits = 0;
};

class Frame final : public RenderFrame {
 public:
  explicit Frame(Native& native) : native_(native) {}
  void clear(Color color) override { native_.clears.push_back(color); }
  void draw_rect(const SolidRect&) override {}
  Result<void> present() override { ++native_.presents; return {}; }
 private:
  Native& native_;
};

class CaptureRenderer final : public Renderer {
 public:
  CaptureRenderer(Native& native, RendererSurfaceComposition composition)
      : composition(composition), native_(native) {}
  Result<void> resize(Size, DpiScale) override { return {}; }
  Result<std::unique_ptr<RenderFrame>> begin_frame() override { return std::make_unique<Frame>(native_); }
  RendererSurfaceComposition surface_composition() const override { return composition; }
  Result<RendererSurfaceComposition> set_transparent_surface(bool transparent) override {
    ++switches;
    composition.transparent_requested = transparent;
    return composition;
  }
  RendererSurfaceComposition composition;
  unsigned switches = 0;
 private:
  Native& native_;
};

class View final : public cgpui::View {
 public:
  void paint(PaintList&, Size) override {}
};

struct Fixture {
  Application app;
  View view;
  RendererSurfaceComposition composition;
  std::vector<std::unique_ptr<CaptureRenderer>> renderers;
  std::vector<RenderSurfaceDescriptor> surface_requests;
  WindowRuntime runtime;
  explicit Fixture(RendererSurfaceComposition actual)
      : composition(actual), runtime(app, view,
          [this](const RenderSurfaceDescriptor& surface) -> Result<Renderer*> {
            surface_requests.push_back(surface);
            auto actual = composition;
            actual.transparent_requested = surface.transparent_background;
            renderers.push_back(std::make_unique<CaptureRenderer>(*app.last, actual));
            return renderers.back().get();
          }) {}
};

inline bool painted(Native& native, bool transparent) {
  const auto count = native.clears.size();
  const auto presents = native.presents;
  native.paint();
  if (native.clears.size() != count + 1 || native.presents != presents + 1) return false;
  const auto color = native.clears.back();
  return transparent ? color.r == 0 && color.g == 0 && color.b == 0 && color.a == 0
                     : color.a == 1;
}
} // namespace runtime_clear_test
