#pragma once

#include "cgpui/platform/platform_appearance.hpp"
#include "cgpui/platform/platform_window_backdrop.hpp"
#include "cgpui/platform/platform_window_title_bar.hpp"
#include "cgpui/renderer/renderer_backend_info.hpp"
#include "cgpui/renderer/renderer_surface_composition.hpp"
#include "cgpui/ui/runtime_input_state.hpp"
#include "cgpui/ui/runtime_window_options.hpp"

namespace cgpui {

class WindowRuntime;

class Window {
 public:
  Window(WindowRuntime& runtime, WindowRuntimeId runtime_id);

  [[nodiscard]] WindowRuntimeId runtime_id() const;
  [[nodiscard]] WindowDescriptor descriptor() const;
  [[nodiscard]] ViewId root_view_id() const;
  [[nodiscard]] bool active() const;
  [[nodiscard]] Size viewport_size() const;
  [[nodiscard]] DpiScale scale() const;
  [[nodiscard]] ViewInputState input_state() const;
  [[nodiscard]] bool focused() const;
  [[nodiscard]] SystemAppearance appearance() const;
  [[nodiscard]] PlatformWindowBackdropState backdrop() const;
  // Applies a backdrop to the live native window. The descriptor keeps the
  // creation-time request.
  PlatformWindowBackdropState set_backdrop(WindowBackdrop backdrop) const;
  [[nodiscard]] RendererSurfaceComposition surface_composition() const;
  [[nodiscard]] RendererBackendInfo backend_info() const;
  // Client title bar support: native frame metrics, and window actions for
  // app-drawn controls (see docs/client-title-bar.md).
  [[nodiscard]] PlatformWindowFrameMetrics frame_metrics() const;
  PlatformWindowActionResult perform_window_action(
      PlatformWindowActionRequest request) const;
  // Native context menu at a client point (macOS); other backends report
  // supported = false, so keep an in-window menu as the fallback.
  PlatformContextMenuResult show_context_menu(
      const NativeMenuModel& menu, Point position) const;

  void request_render() const;
  void request_layout() const;
  void request_paint() const;

 private:
  [[nodiscard]] const WindowRuntimeRecord* record() const;

  WindowRuntime* runtime_ = nullptr;
  WindowRuntimeId runtime_id_{};
};

} // namespace cgpui
