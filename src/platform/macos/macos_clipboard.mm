#include "macos_clipboard_internal.hpp"

#import <AppKit/AppKit.h>

#include <memory>
#include <string>
#include <vector>

namespace cgpui {
namespace {

class MacOSClipboard final : public Clipboard {
 public:
  explicit MacOSClipboard(NSPasteboard* pasteboard)
      : pasteboard_(pasteboard == nil ? [NSPasteboard generalPasteboard]
                                     : pasteboard) {}

  std::optional<std::string> read_text() const override {
    NSString* value = [pasteboard_ stringForType:NSPasteboardTypeString];
    const char* bytes = [value UTF8String];
    if (value == nil || bytes == nullptr) return std::nullopt;
    return std::string(bytes);
  }

  bool write_text(std::string_view text) override {
    NSString* value = [[NSString alloc] initWithBytes:text.data()
                                                 length:text.size()
                                               encoding:NSUTF8StringEncoding];
    if (value == nil) return false;
    [pasteboard_ clearContents];
    return [pasteboard_ setString:value forType:NSPasteboardTypeString];
  }

  std::optional<std::vector<std::string>> read_files() const override {
    if ([pasteboard_ availableTypeFromArray:@[NSPasteboardTypeFileURL]] == nil) {
      return std::nullopt;
    }
    NSArray<NSURL*>* urls = [pasteboard_
        readObjectsForClasses:@[[NSURL class]]
                      options:@{NSPasteboardURLReadingFileURLsOnlyKey : @YES}];
    if (urls.count == 0) return std::nullopt;
    std::vector<std::string> paths;
    paths.reserve(urls.count);
    for (NSURL* url in urls) {
      const char* path = [[url path] UTF8String];
      if (![url isFileURL] || path == nullptr) continue;
      paths.emplace_back(path);
    }
    return paths.empty()
        ? std::optional<std::vector<std::string>>{}
        : std::optional<std::vector<std::string>>(std::move(paths));
  }

  bool write_files(std::span<const std::string> paths) override {
    NSMutableArray<NSURL*>* urls = [NSMutableArray arrayWithCapacity:paths.size()];
    for (const auto& path : paths) {
      if (path.empty()) return false;
      NSString* value = [[NSString alloc] initWithBytes:path.data()
                                                   length:path.size()
                                                 encoding:NSUTF8StringEncoding];
      if (value == nil) return false;
      [urls addObject:[NSURL fileURLWithPath:value]];
    }
    if (urls.count == 0) return false;
    [pasteboard_ clearContents];
    return [pasteboard_ writeObjects:urls];
  }

 private:
  __strong NSPasteboard* pasteboard_ = nil;
};

}  // namespace

std::unique_ptr<Clipboard> create_macos_clipboard(NSPasteboard* pasteboard) {
  return std::make_unique<MacOSClipboard>(pasteboard);
}

std::unique_ptr<Clipboard> create_macos_clipboard() {
  return create_macos_clipboard([NSPasteboard generalPasteboard]);
}

bool macos_pasteboard_contains_image(NSPasteboard* pasteboard) {
  return pasteboard != nil &&
      [pasteboard availableTypeFromArray:
          @[NSPasteboardTypePNG, NSPasteboardTypeTIFF]] != nil;
}

}  // namespace cgpui
