#pragma once

#include "cgpui/core/events.hpp"
#include "cgpui/core/window_popup.hpp"
#include "cgpui/ui/element_core.hpp"
#include "cgpui/ui/runtime_ids.hpp"

#include <functional>
#include <memory>
#include <optional>

namespace cgpui {

class PaintList;
class WindowRuntime;
struct WindowRuntimeContext;

namespace workbench_detail {

// What a popup window shows: the widget paints in root-window coordinates
// (the view translates by the popup's origin) and receives the popup's
// pointer events translated back into root-window coordinates.
struct WidgetPopupContent {
  std::function<void(PaintList&)> paint;
  std::function<EventResult(const PlatformEvent&, const WindowRuntimeContext&)> event;
};

// Event handler that hands the popup's pointer input (already in
// root-window coordinates) to the root window, so the widget sees it with
// the root view's context, as in the in-window overlay.
[[nodiscard]] std::function<EventResult(const PlatformEvent&, const WindowRuntimeContext&)>
forward_pointer_input_to_root();

// One popup window (runtime_popups.hpp) owned by a widget. Closing is
// deferred, because the popup's own view may be handling the event that
// closes it.
class WidgetPopup {
 public:
  WidgetPopup() = default;
  ~WidgetPopup();
  WidgetPopup(const WidgetPopup&) = delete;
  WidgetPopup& operator=(const WidgetPopup&) = delete;

  // Opens the popup (or moves it; a new size reopens it) so a window of
  // `size` sits at `placement`. Returns the window's bounds in root-window
  // coordinates, or nullopt where popups are unavailable or fail to open.
  std::optional<Rect> show(WindowRuntime& runtime, WindowPopupPlacement placement,
                           Size size, WindowPopupOptions options, WidgetPopupContent content);
  void redraw();
  void close();
  // False once closed, or once the runtime destroyed the popup's view
  // (popup closed by the runtime, or runtime shut down).
  [[nodiscard]] bool open() const { return id_.value != 0 && !view_alive_.expired(); }

 private:
  struct Shared {
    WidgetPopupContent content;
    Point origin;
    bool redraw_pending = false;
  };

  WindowRuntime* runtime_ = nullptr;
  WindowRuntimeId id_{};
  Size size_{};
  Rect bounds_{};
  std::optional<WindowPopupPlacement> placement_;  // last placed
  std::shared_ptr<Shared> shared_;
  std::weak_ptr<int> view_alive_;
};

}  // namespace workbench_detail
}  // namespace cgpui
