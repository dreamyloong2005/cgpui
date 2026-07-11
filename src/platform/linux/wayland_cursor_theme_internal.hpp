#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

struct wl_compositor;
struct wl_pointer;
struct wl_shm;

namespace cgpui {

class WaylandCursorThemeResources;
struct WaylandCursorThemeResourcesDeleter {
  void operator()(WaylandCursorThemeResources* resources) const;
};
using WaylandCursorThemeResourcesPtr = std::unique_ptr<
    WaylandCursorThemeResources,
    WaylandCursorThemeResourcesDeleter>;

struct WaylandCursorThemeApplyResult {
  bool theme_loaded = false;
  bool image_ready = false;
  bool applied = false;
  std::int32_t hotspot_x = 0;
  std::int32_t hotspot_y = 0;
  std::int32_t buffer_scale = 1;
  std::string unavailable_reason;
};

[[nodiscard]] WaylandCursorThemeResourcesPtr create_wayland_cursor_theme(
    wl_compositor* compositor,
    wl_shm* shm);
[[nodiscard]] WaylandCursorThemeApplyResult apply_wayland_cursor_theme(
    WaylandCursorThemeResources& resources,
    wl_pointer* pointer,
    std::uint32_t serial,
    std::string_view cursor_name,
    std::int32_t buffer_scale);

} // namespace cgpui
