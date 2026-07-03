#include "wayland_window_internal.hpp"

namespace cgpui {

void WaylandWindow::drag_entered(
    Point position,
    DragDropPayload payload,
    DragDropAction action) {
  callback_(DragEntered{
      .position = position,
      .payload = std::move(payload),
      .action = action});
}

void WaylandWindow::drag_updated(
    Point position,
    DragDropPayload payload,
    DragDropAction action) {
  callback_(DragUpdated{
      .position = position,
      .payload = std::move(payload),
      .action = action});
}

void WaylandWindow::drag_dropped(
    Point position,
    DragDropPayload payload,
    DragDropAction action) {
  callback_(DragDropped{
      .position = position,
      .payload = std::move(payload),
      .action = action});
}

void WaylandWindow::drag_exited(Point position) {
  callback_(DragExited{
      .position = position,
      .payload = {},
      .action = DragDropAction::none});
}

} // namespace cgpui
