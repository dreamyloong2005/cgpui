#include "macos_drag_internal.hpp"
#include "macos_input_internal.hpp"
#include "macos_window_internal.hpp"

namespace cgpui {

void MacOSWindow::drag_entered(
    Point position,
    DragDropPayload payload,
    DragDropAction action) {
  if (payload.kind == DragDropPayloadKind::none ||
      action == DragDropAction::none) {
    drag_exited(position);
    return;
  }
  if (drag_active_) drag_exited(position);
  drag_active_ = true;
  drag_position_ = position;
  drag_payload_ = std::move(payload);
  callback_(DragEntered{
      .position = position,
      .payload = drag_payload_,
      .action = action});
}

void MacOSWindow::drag_updated(Point position, DragDropAction action) {
  if (!drag_active_) return;
  drag_position_ = position;
  callback_(DragUpdated{
      .position = position,
      .payload = drag_payload_,
      .action = action});
}

void MacOSWindow::drag_dropped(
    Point position,
    DragDropPayload payload,
    DragDropAction action) {
  if (!drag_active_) return;
  if (payload.kind == DragDropPayloadKind::none ||
      action == DragDropAction::none) {
    drag_exited(position);
    return;
  }
  callback_(DragDropped{
      .position = position,
      .payload = std::move(payload),
      .action = action});
  drag_active_ = false;
  drag_position_ = position;
  drag_payload_ = {};
}

void MacOSWindow::drag_exited(Point position) {
  if (!drag_active_) return;
  drag_active_ = false;
  drag_position_ = position;
  drag_payload_ = {};
  callback_(DragExited{
      .position = position,
      .payload = {},
      .action = DragDropAction::none});
}

}  // namespace cgpui

namespace {

cgpui::MacOSWindow* drag_adapter(void* value) {
  return static_cast<cgpui::MacOSWindow*>(value);
}

cgpui::Point drag_position(
    CGPUIMacOSContentView* view,
    id<NSDraggingInfo> sender) {
  const NSPoint point = [view convertPoint:[sender draggingLocation]
                                  fromView:nil];
  return {static_cast<float>(point.x), static_cast<float>(point.y)};
}

}  // namespace

@implementation CGPUIMacOSContentView (DraggingDestination)
- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender {
  if (window_adapter_ == nullptr) return NSDragOperationNone;
  const auto payload = cgpui::macos_drag_payload_from_pasteboard(
      [sender draggingPasteboard]);
  const auto action = cgpui::macos_drag_action_from_operation(
      [sender draggingSourceOperationMask]);
  if (payload.kind == cgpui::DragDropPayloadKind::none ||
      action == cgpui::DragDropAction::none) return NSDragOperationNone;
  drag_adapter(window_adapter_)->drag_entered(
      drag_position(self, sender), payload, action);
  return cgpui::macos_drag_operation_from_action(action);
}
- (NSDragOperation)draggingUpdated:(id<NSDraggingInfo>)sender {
  if (window_adapter_ == nullptr) return NSDragOperationNone;
  const auto action = cgpui::macos_drag_action_from_operation(
      [sender draggingSourceOperationMask]);
  drag_adapter(window_adapter_)->drag_updated(drag_position(self, sender), action);
  return cgpui::macos_drag_operation_from_action(action);
}
- (void)draggingExited:(id<NSDraggingInfo>)sender {
  if (window_adapter_ != nullptr) {
    drag_adapter(window_adapter_)->drag_exited(drag_position(self, sender));
  }
}
- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender {
  if (window_adapter_ == nullptr) return NO;
  const auto payload = cgpui::macos_drag_payload_from_pasteboard(
      [sender draggingPasteboard]);
  const auto action = cgpui::macos_drag_action_from_operation(
      [sender draggingSourceOperationMask]);
  drag_adapter(window_adapter_)->drag_dropped(
      drag_position(self, sender), payload, action);
  return payload.kind != cgpui::DragDropPayloadKind::none &&
      action != cgpui::DragDropAction::none;
}
@end
