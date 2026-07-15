#pragma once

#include "cgpui/platform/platform.hpp"

#import <AppKit/AppKit.h>

namespace cgpui {

struct MacOSDragSourceResult {
  DragDropAction action = DragDropAction::none;
  bool cancelled = false;
};

[[nodiscard]] DragDropPayload macos_drag_payload_from_pasteboard(
    NSPasteboard* pasteboard);
[[nodiscard]] DragDropAction macos_drag_action_from_operation(
    NSDragOperation operation);
[[nodiscard]] NSDragOperation macos_drag_operation_from_action(
    DragDropAction action);
[[nodiscard]] MacOSDragSourceResult macos_drag_source_result(
    NSDragOperation operation,
    bool cancelled);

}  // namespace cgpui

@interface CGPUIMacOSDragSource : NSObject <NSDraggingSource> {
 @private
  NSDragOperation allowed_operations_;
  cgpui::MacOSDragSourceResult result_;
}
- (instancetype)initWithAllowedOperations:(NSDragOperation)operations;
- (cgpui::MacOSDragSourceResult)result;
@end
