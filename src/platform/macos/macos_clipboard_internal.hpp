#pragma once

#include "cgpui/platform/clipboard.hpp"

#import <AppKit/AppKit.h>

#include <memory>

namespace cgpui {

[[nodiscard]] std::unique_ptr<Clipboard> create_macos_clipboard(
    NSPasteboard* pasteboard);
[[nodiscard]] bool macos_pasteboard_contains_image(
    NSPasteboard* pasteboard);

}  // namespace cgpui
