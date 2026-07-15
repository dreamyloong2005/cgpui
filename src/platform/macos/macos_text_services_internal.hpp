#pragma once

#include "cgpui/platform/platform_font_discovery.hpp"
#include "cgpui/platform/platform.hpp"

#import <AppKit/AppKit.h>

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace cgpui {

struct MacOSTextServicesDiagnostics {
  std::size_t committed_text_count = 0;
  std::size_t marked_text_update_count = 0;
  std::size_t unmark_count = 0;
  std::size_t delete_surrounding_count = 0;
};

struct MacOSTextInputState {
  std::string marked_text;
  std::size_t marked_cursor = 0;
  MacOSTextServicesDiagnostics diagnostics;
};

[[nodiscard]] PlatformFontDiscoveryResult macos_discover_fonts();
[[nodiscard]] std::pair<std::size_t, std::size_t> macos_nsrange_to_utf8_bytes(
    std::string_view text,
    std::size_t location,
    std::size_t length);
[[nodiscard]] Rect macos_candidate_rect_to_screen(
    Rect rect,
    Size framebuffer_size,
    DpiScale scale,
    Point window_origin);

[[nodiscard]] MacOSTextServicesDiagnostics macos_text_services_diagnostics(
    const PlatformWindow& window);

}  // namespace cgpui
