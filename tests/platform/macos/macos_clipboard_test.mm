#include "macos_clipboard_internal.hpp"

#import <AppKit/AppKit.h>

int main() {
  @autoreleasepool {
    NSPasteboard* pasteboard = [NSPasteboard pasteboardWithUniqueName];
    auto clipboard = cgpui::create_macos_clipboard(pasteboard);
    if (!clipboard || !clipboard->write_text("macOS \xE6\xBC\xA2\xF0\x9F\x98\x80")) {
      return 1;
    }
    const auto text = clipboard->read_text();
    if (!text || *text != "macOS \xE6\xBC\xA2\xF0\x9F\x98\x80") return 2;

    const std::string paths[] = {
        "/tmp/cgpui one.txt",
        "/tmp/cgpui-\xE6\xBC\xA2.png",
    };
    if (!clipboard->write_files(paths)) return 3;
    const auto files = clipboard->read_files();
    if (!files || files->size() != 2 || (*files)[0] != paths[0] ||
        (*files)[1] != paths[1]) {
      return 4;
    }

    [pasteboard clearContents];
    [pasteboard setData:[NSData dataWithBytes:"png" length:3]
                forType:NSPasteboardTypePNG];
    if (!cgpui::macos_pasteboard_contains_image(pasteboard)) return 5;
    [pasteboard clearContents];
    [pasteboard setData:[NSData dataWithBytes:"tiff" length:4]
                forType:NSPasteboardTypeTIFF];
    if (!cgpui::macos_pasteboard_contains_image(pasteboard)) return 6;

    [pasteboard releaseGlobally];
  }
  return 0;
}
