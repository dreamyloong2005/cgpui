#include "macos_text_services_internal.hpp"

#import <CoreText/CoreText.h>

#include <algorithm>
#include <string>
#include <vector>

namespace cgpui {

PlatformFontDiscoveryResult macos_discover_fonts() {
  NSArray<NSString*>* names = CFBridgingRelease(
      CTFontManagerCopyAvailablePostScriptNames());
  std::vector<FontFaceDescriptor> records;
  for (NSString* postscript in names) {
    id font_object = CFBridgingRelease(
        CTFontCreateWithName((__bridge CFStringRef)postscript, 12.0, nullptr));
    CTFontRef font = (__bridge CTFontRef)font_object;
    if (font == nullptr) continue;
    NSString* family = CFBridgingRelease(CTFontCopyFamilyName(font));
    NSURL* url = CFBridgingRelease(CTFontCopyAttribute(font, kCTFontURLAttribute));
    NSString* path = [url path];
    if (family.length != 0 && postscript.length != 0) {
      std::string path_value = path.length == 0
          ? "/System/Library/Fonts"
          : std::string([path UTF8String]);
      records.push_back(FontFaceDescriptor{
          .font = FontDescriptor{.family = std::string([family UTF8String])},
          .postscript_name = std::string([postscript UTF8String]),
          .source = FontSource::platform,
          .path = std::move(path_value)});
    }
  }
  std::sort(records.begin(), records.end(), [](const auto& lhs, const auto& rhs) {
    if (lhs.font.family != rhs.font.family) return lhs.font.family < rhs.font.family;
    return lhs.postscript_name < rhs.postscript_name;
  });
  return make_platform_font_discovery_result(
      PlatformFontDiscoveryBackend::core_text,
      PlatformFontDiscoveryStatus::native_available,
      std::move(records));
}

}  // namespace cgpui
