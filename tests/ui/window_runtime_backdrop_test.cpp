#include "runtime_window_backdrop_internal.hpp"

#include <iostream>

namespace {

using cgpui::WindowBackdrop;

class FakeWindow final : public cgpui::PlatformWindow {
 public:
  bool backend_supported = true;
  int apply_count = 0;
  cgpui::PlatformWindowBackdropState last;

  cgpui::NativeSurfaceHandle native_surface() const override { return {}; }
  cgpui::WindowState state() const override { return {}; }
  void request_redraw() override {}
  void request_close() override {}
  void set_title(std::string_view) override {}
  void set_cursor(cgpui::CursorShape) override {}
  void set_ime_text_input_placement(
      std::optional<cgpui::ImeTextInputPlacement>) override {}
  cgpui::PlatformWindowBackdropState apply_window_backdrop(
      WindowBackdrop backdrop, bool transparent_surface) override {
    ++apply_count;
    last = cgpui::PlatformWindowBackdropState{
        .supported = backend_supported,
        .transparent_surface = transparent_surface,
        .backend = "fake",
        .requested = backdrop,
        .applied = backend_supported && transparent_surface
            ? backdrop : WindowBackdrop::none};
    return last;
  }
  cgpui::PlatformWindowBackdropState window_backdrop_state() const override {
    return last;
  }
};

class FakeRenderer final : public cgpui::Renderer {
 public:
  cgpui::RendererSurfaceComposition composition{
      .supported = true,
      .composite_alpha = cgpui::RendererCompositeAlpha::opaque};
  cgpui::RendererCompositeAlpha transparent_alpha =
      cgpui::RendererCompositeAlpha::pre_multiplied;
  int switch_count = 0;

  cgpui::Result<void> resize(cgpui::Size, cgpui::DpiScale) override { return {}; }
  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    return std::unique_ptr<cgpui::RenderFrame>{};
  }
  cgpui::RendererSurfaceComposition surface_composition() const override {
    return composition;
  }
  cgpui::Result<cgpui::RendererSurfaceComposition> set_transparent_surface(
      bool transparent) override {
    ++switch_count;
    composition.transparent_requested = transparent;
    composition.composite_alpha = transparent_alpha;
    return composition;
  }
};

int fail(int code, const char* message) {
  std::cerr << "window runtime backdrop: " << message << '\n';
  return code;
}

} // namespace

int main() {
  FakeWindow window;
  (void)window.apply_window_backdrop(WindowBackdrop::mica, false);
  FakeRenderer renderer;
  const auto applied =
      cgpui::runtime_apply_window_backdrop(window, &renderer, WindowBackdrop::mica);
  if (applied.applied != WindowBackdrop::mica || renderer.switch_count != 1 ||
      window.apply_count != 3) {
    return fail(3, "an opaque surface must be switched, then re-applied");
  }
  if (cgpui::runtime_window_clear_color(&window).a != 0.0F) {
    return fail(4, "a visible material needs a transparent clear");
  }
  FakeRenderer inherit;
  inherit.transparent_alpha = cgpui::RendererCompositeAlpha::inherit;
  const auto title_only =
      cgpui::runtime_apply_window_backdrop(window, &inherit, WindowBackdrop::acrylic);
  if (title_only.applied != WindowBackdrop::none || title_only.transparent_surface ||
      cgpui::runtime_window_clear_color(&window).a != 1.0F) {
    return fail(5, "inherit alpha must not be reported as transparent");
  }
  // A renderer that already asked for transparency is not switched again.
  (void)cgpui::runtime_apply_window_backdrop(window, &inherit, WindowBackdrop::acrylic);
  if (inherit.switch_count != 1) return fail(6, "transparency is requested once");
  window.backend_supported = false;
  FakeRenderer unsupported;
  const auto absent =
      cgpui::runtime_apply_window_backdrop(window, &unsupported, WindowBackdrop::blur);
  if (absent.applied != WindowBackdrop::none || unsupported.switch_count != 0) {
    return fail(7, "an unsupported backend must leave the surface opaque");
  }
  if (cgpui::runtime_window_clear_color(nullptr).a != 1.0F) {
    return fail(8, "no window clears opaque");
  }
  return 0;
}
