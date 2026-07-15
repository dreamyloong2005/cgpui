#include "macos_drag_internal.hpp"
#include "macos_window_internal.hpp"

#include "cgpui/platform/platform.hpp"

#import <AppKit/AppKit.h>

#include <variant>
#include <vector>

int main() {
  @autoreleasepool {
    if (cgpui::macos_drag_action_from_operation(NSDragOperationCopy) !=
            cgpui::DragDropAction::copy ||
        cgpui::macos_drag_action_from_operation(NSDragOperationMove) !=
            cgpui::DragDropAction::move ||
        cgpui::macos_drag_action_from_operation(NSDragOperationLink) !=
            cgpui::DragDropAction::none ||
        cgpui::macos_drag_operation_from_action(cgpui::DragDropAction::copy) !=
            NSDragOperationCopy) {
      return 1;
    }
    const auto cancelled =
        cgpui::macos_drag_source_result(NSDragOperationMove, true);
    if (!cancelled.cancelled || cancelled.action != cgpui::DragDropAction::none) {
      return 2;
    }
    CGPUIMacOSDragSource* source = [[CGPUIMacOSDragSource alloc]
        initWithAllowedOperations:NSDragOperationCopy | NSDragOperationMove];
    NSDraggingSession* session = (NSDraggingSession*)[NSObject new];
    if ([source draggingSession:session
        sourceOperationMaskForDraggingContext:NSDraggingContextOutsideApplication] !=
        (NSDragOperationCopy | NSDragOperationMove)) {
      return 3;
    }
    [source draggingSession:session
               endedAtPoint:NSZeroPoint
                  operation:NSDragOperationMove];
    const auto source_result = [source result];
    if (source_result.cancelled ||
        source_result.action != cgpui::DragDropAction::move) {
      return 4;
    }

    NSPasteboard* pasteboard = [NSPasteboard pasteboardWithUniqueName];
    [pasteboard clearContents];
    [pasteboard setString:@"drag \u6F22" forType:NSPasteboardTypeString];
    const auto text_payload = cgpui::macos_drag_payload_from_pasteboard(pasteboard);
    if (text_payload.kind != cgpui::DragDropPayloadKind::text ||
        text_payload.text != "drag \xE6\xBC\xA2") {
      return 5;
    }
    [pasteboard clearContents];
    [pasteboard writeObjects:@[
      [NSURL fileURLWithPath:@"/tmp/drag one.txt"],
      [NSURL fileURLWithPath:@"/tmp/drag-two.txt"],
    ]];
    const auto file_payload = cgpui::macos_drag_payload_from_pasteboard(pasteboard);
    if (file_payload.kind != cgpui::DragDropPayloadKind::files ||
        file_payload.files.size() != 2) {
      return 6;
    }
    [pasteboard clearContents];
    [pasteboard setData:[NSData data] forType:NSPasteboardTypePNG];
    if (cgpui::macos_drag_payload_from_pasteboard(pasteboard).kind !=
        cgpui::DragDropPayloadKind::none) {
      return 7;
    }

    std::vector<cgpui::PlatformEvent> events;
    auto application = cgpui::create_platform_application();
    if (!application) return 8;
    auto window = (*application)->create_window(
        cgpui::WindowDescriptor{.title = "CGPUI macOS Drag Test",
                                .size = {160.0F, 90.0F}},
        [&](const cgpui::PlatformEvent& event) { events.push_back(event); });
    if (!window) return 9;
    auto* native = dynamic_cast<cgpui::MacOSWindow*>(window->get());
    if (native == nullptr) return 10;
    events.clear();

    native->drag_entered({10.0F, 20.0F}, text_payload,
                         cgpui::DragDropAction::copy);
    native->drag_updated({11.0F, 21.0F}, cgpui::DragDropAction::move);
    native->drag_dropped({12.0F, 22.0F}, file_payload,
                         cgpui::DragDropAction::move);
    native->drag_entered({13.0F, 23.0F}, text_payload,
                         cgpui::DragDropAction::copy);
    native->drag_exited({14.0F, 24.0F});
    native->drag_exited({15.0F, 25.0F});
    if (events.size() != 5 ||
        !std::holds_alternative<cgpui::DragEntered>(events[0]) ||
        !std::holds_alternative<cgpui::DragUpdated>(events[1]) ||
        !std::holds_alternative<cgpui::DragDropped>(events[2]) ||
        !std::holds_alternative<cgpui::DragEntered>(events[3]) ||
        !std::holds_alternative<cgpui::DragExited>(events[4])) {
      return 11;
    }
    const auto& updated = std::get<cgpui::DragUpdated>(events[1]);
    if (updated.payload.text != text_payload.text ||
        updated.action != cgpui::DragDropAction::move) {
      return 12;
    }
    native->drag_entered({16.0F, 26.0F}, text_payload,
                         cgpui::DragDropAction::copy);
    native->drag_dropped({17.0F, 27.0F}, {}, cgpui::DragDropAction::none);
    if (events.size() != 7 ||
        !std::holds_alternative<cgpui::DragEntered>(events[5]) ||
        !std::holds_alternative<cgpui::DragExited>(events[6])) {
      return 13;
    }

    std::size_t teardown_exits = 0;
    {
      auto teardown = (*application)->create_window(
          cgpui::WindowDescriptor{.title = "CGPUI macOS Drag Teardown",
                                  .size = {120.0F, 70.0F}},
          [&](const cgpui::PlatformEvent& event) {
            if (std::holds_alternative<cgpui::DragExited>(event)) {
              ++teardown_exits;
            }
          });
      if (!teardown) return 14;
      auto* teardown_native =
          dynamic_cast<cgpui::MacOSWindow*>(teardown->get());
      teardown_native->drag_entered(
          {}, text_payload, cgpui::DragDropAction::copy);
    }
    if (teardown_exits != 1) return 15;
    [pasteboard releaseGlobally];
  }
  return 0;
}
