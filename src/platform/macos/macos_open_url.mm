#include "macos_platform_services_internal.hpp"

namespace cgpui {

NSURL* macos_url(std::string_view value) {
  if (value.empty() || value.find('\0') != std::string_view::npos) return nil;
  NSString* string = [[NSString alloc] initWithBytes:value.data()
                                                length:value.size()
                                              encoding:NSUTF8StringEncoding];
  if (string == nil) return nil;
  NSURL* url = [NSURL URLWithString:string];
  return [url scheme].length == 0 ? nil : url;
}

PlatformOpenUrlResult macos_open_url(std::string_view value) {
  NSURL* url = macos_url(value);
  if (url == nil) {
    return {.supported = true, .backend = "macos", .error_message = "invalid URL"};
  }
  const bool opened = [[NSWorkspace sharedWorkspace] openURL:url];
  return PlatformOpenUrlResult{
      .supported = true,
      .opened = opened,
      .backend = "macos",
      .error_message = opened ? "" : "NSWorkspace openURL failed"};
}

}  // namespace cgpui
