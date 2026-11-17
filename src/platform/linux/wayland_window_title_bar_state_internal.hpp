#pragma once

#include "cgpui/platform/platform_window_title_bar.hpp"

#include <cstdint>
#include <optional>
#include <span>
#include <vector>

struct wl_seat;

namespace cgpui {

// Client-side decorations draw inside the surface, so the resize band is
// inside it too: 6 logical pixels on each edge, corners 12 pixels long.
inline constexpr float wayland_client_resize_border = 6.0F;
inline constexpr std::uint32_t wayland_double_click_ms = 400;
inline constexpr float wayland_double_click_distance = 4.0F;

struct WaylandTitleBarHit {
  std::optional<std::uint32_t> resize_edge;  // xdg_toplevel resize_edge
  std::optional<WindowControlArea> area;
};

// Resize edges first (when `resize_enabled`), then regions from last to
// first (topmost painted wins).
[[nodiscard]] WaylandTitleBarHit wayland_title_bar_hit(
    std::span<const WindowControlRegion> regions, Point point, Size surface,
    bool resize_enabled);

[[nodiscard]] std::uint32_t wayland_resize_edge(WindowResizeEdge edge);

struct WaylandClientTitleBar {
  std::vector<WindowControlRegion> regions;
  wl_seat* seat = nullptr;
  std::uint32_t press_serial = 0;  // last pointer button press on the window
  std::optional<WindowControlArea> pressed_button;
  std::optional<std::uint32_t> last_caption_press_time;
  Point last_caption_press_position;
};

} // namespace cgpui
