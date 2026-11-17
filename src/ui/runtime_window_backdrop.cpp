#include "ui_internal.hpp"

namespace cgpui {

PlatformWindowBackdropState runtime_apply_window_backdrop(
    PlatformWindow& window,
    Renderer* renderer,
    WindowBackdrop backdrop) {
  const RendererSurfaceComposition composition =
      renderer == nullptr ? RendererSurfaceComposition{}
                          : renderer->surface_composition();
  PlatformWindowBackdropState state =
      window.apply_window_backdrop(backdrop, composition.alpha_composited());
  if (renderer == nullptr || backdrop == WindowBackdrop::none ||
      !state.supported || !composition.supported ||
      composition.alpha_composited() || composition.transparent_requested) {
    return state;
  }
  auto transparent = renderer->set_transparent_surface(true);
  if (!transparent) {
    state.reason += "; transparent surface request failed: " +
                    transparent.error().message;
    return state;
  }
  return window.apply_window_backdrop(
      backdrop, transparent->alpha_composited());
}

Color runtime_window_clear_color(const PlatformWindow* window) {
  if (window == nullptr) return runtime_default_clear_color;
  const PlatformWindowBackdropState state = window->window_backdrop_state();
  return state.applied != WindowBackdrop::none && state.transparent_surface
      ? Color{.r = 0.0F, .g = 0.0F, .b = 0.0F, .a = 0.0F}
      : runtime_default_clear_color;
}

} // namespace cgpui
