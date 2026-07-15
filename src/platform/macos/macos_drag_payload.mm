#include "macos_drag_internal.hpp"

#include <string>
#include <vector>

namespace cgpui {

DragDropPayload macos_drag_payload_from_pasteboard(NSPasteboard* pasteboard) {
  if (pasteboard == nil) return {};
  NSArray<NSURL*>* urls = [pasteboard
      readObjectsForClasses:@[[NSURL class]]
                    options:@{NSPasteboardURLReadingFileURLsOnlyKey : @YES}];
  if (urls.count != 0) {
    std::vector<std::string> files;
    files.reserve(urls.count);
    for (NSURL* url in urls) {
      const char* path = [[url path] UTF8String];
      if ([url isFileURL] && path != nullptr) files.emplace_back(path);
    }
    if (!files.empty()) {
      return DragDropPayload{
          .kind = DragDropPayloadKind::files,
          .files = std::move(files)};
    }
  }
  NSString* value = [pasteboard stringForType:NSPasteboardTypeString];
  const char* text = [value UTF8String];
  if (value != nil && text != nullptr) {
    return DragDropPayload{
        .kind = DragDropPayloadKind::text,
        .text = std::string(text)};
  }
  return {};
}

DragDropAction macos_drag_action_from_operation(NSDragOperation operation) {
  if ((operation & NSDragOperationMove) != 0) return DragDropAction::move;
  if ((operation & NSDragOperationCopy) != 0) return DragDropAction::copy;
  return DragDropAction::none;
}

NSDragOperation macos_drag_operation_from_action(DragDropAction action) {
  switch (action) {
    case DragDropAction::copy: return NSDragOperationCopy;
    case DragDropAction::move: return NSDragOperationMove;
    case DragDropAction::none: return NSDragOperationNone;
  }
  return NSDragOperationNone;
}

}  // namespace cgpui
