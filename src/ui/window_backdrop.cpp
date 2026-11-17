#include "cgpui/app/window.hpp"

#include "cgpui/ui/window_runtime.hpp"
#include "runtime_window_backdrop_internal.hpp"

namespace cgpui {
namespace {

PlatformWindowBackdropState inactive_state(WindowBackdrop requested) {
  return PlatformWindowBackdropState{
      .requested = requested, .reason = "native window is not active"};
}

} // namespace

PlatformWindowBackdropState Window::backdrop() const {
  const WindowRuntimeRecord* runtime_record = record();
  if (runtime_record == nullptr || runtime_record->window == nullptr) {
    return inactive_state(descriptor().backdrop);
  }
  return runtime_record->window->window_backdrop_state();
}

PlatformWindowBackdropState Window::set_backdrop(
    WindowBackdrop backdrop) const {
  const WindowRuntimeRecord* runtime_record = record();
  if (runtime_record == nullptr || runtime_record->window == nullptr) {
    return inactive_state(backdrop);
  }
  PlatformWindowBackdropState state = runtime_apply_window_backdrop(
      *runtime_record->window, runtime_record->renderer, backdrop);
  runtime_->request_render();
  return state;
}

RendererSurfaceComposition Window::surface_composition() const {
  const WindowRuntimeRecord* runtime_record = record();
  return runtime_record == nullptr || runtime_record->renderer == nullptr
      ? RendererSurfaceComposition{}
      : runtime_record->renderer->surface_composition();
}

} // namespace cgpui
