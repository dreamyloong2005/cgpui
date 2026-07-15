#include "macos_drag_internal.hpp"

namespace cgpui {

MacOSDragSourceResult macos_drag_source_result(
    NSDragOperation operation,
    bool cancelled) {
  return MacOSDragSourceResult{
      .action = cancelled ? DragDropAction::none
                          : macos_drag_action_from_operation(operation),
      .cancelled = cancelled};
}

}  // namespace cgpui

@implementation CGPUIMacOSDragSource
- (instancetype)initWithAllowedOperations:(NSDragOperation)operations {
  self = [super init];
  if (self != nil) allowed_operations_ = operations;
  return self;
}
- (NSDragOperation)draggingSession:(NSDraggingSession*)session
    sourceOperationMaskForDraggingContext:(NSDraggingContext)context {
  (void)session;
  (void)context;
  return allowed_operations_;
}
- (BOOL)ignoreModifierKeysForDraggingSession:(NSDraggingSession*)session {
  (void)session;
  return NO;
}
- (void)draggingSession:(NSDraggingSession*)session
           endedAtPoint:(NSPoint)screenPoint
              operation:(NSDragOperation)operation {
  (void)session;
  (void)screenPoint;
  result_ = cgpui::macos_drag_source_result(
      operation, operation == NSDragOperationNone);
}
- (cgpui::MacOSDragSourceResult)result {
  return result_;
}
@end
