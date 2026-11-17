#include "wayland_window_title_bar_state_internal.hpp"
#include "wayland_protocol_xdg_toplevel_interactive_internal.hpp"

namespace cgpui {
namespace {

std::optional<std::uint32_t> resize_edge_at(Point point, Size surface) {
  constexpr float border = wayland_client_resize_border;
  constexpr float corner = border * 2.0F;
  const bool left = point.x < border;
  const bool right = point.x >= surface.width - border;
  const bool top = point.y < border;
  const bool bottom = point.y >= surface.height - border;
  if (!left && !right && !top && !bottom) return std::nullopt;
  const bool near_left = point.x < corner;
  const bool near_right = point.x >= surface.width - corner;
  const bool near_top = point.y < corner;
  const bool near_bottom = point.y >= surface.height - corner;
  if ((top && near_left) || (left && near_top)) {
    return xdg_toplevel_resize_edge_top_left;
  }
  if ((top && near_right) || (right && near_top)) {
    return xdg_toplevel_resize_edge_top_right;
  }
  if ((bottom && near_left) || (left && near_bottom)) {
    return xdg_toplevel_resize_edge_bottom_left;
  }
  if ((bottom && near_right) || (right && near_bottom)) {
    return xdg_toplevel_resize_edge_bottom_right;
  }
  if (top) return xdg_toplevel_resize_edge_top;
  if (bottom) return xdg_toplevel_resize_edge_bottom;
  return left ? xdg_toplevel_resize_edge_left : xdg_toplevel_resize_edge_right;
}

} // namespace

WaylandTitleBarHit wayland_title_bar_hit(
    std::span<const WindowControlRegion> regions, Point point, Size surface,
    bool resize_enabled) {
  if (resize_enabled) {
    if (const auto edge = resize_edge_at(point, surface)) {
      return WaylandTitleBarHit{.resize_edge = edge};
    }
  }
  for (auto region = regions.rbegin(); region != regions.rend(); ++region) {
    if (contains(region->bounds, point)) {
      return WaylandTitleBarHit{.area = region->area};
    }
  }
  return {};
}

std::uint32_t wayland_resize_edge(WindowResizeEdge edge) {
  switch (edge) {
    case WindowResizeEdge::top: return xdg_toplevel_resize_edge_top;
    case WindowResizeEdge::bottom: return xdg_toplevel_resize_edge_bottom;
    case WindowResizeEdge::left: return xdg_toplevel_resize_edge_left;
    case WindowResizeEdge::right: return xdg_toplevel_resize_edge_right;
    case WindowResizeEdge::top_left: return xdg_toplevel_resize_edge_top_left;
    case WindowResizeEdge::top_right: return xdg_toplevel_resize_edge_top_right;
    case WindowResizeEdge::bottom_left: return xdg_toplevel_resize_edge_bottom_left;
    case WindowResizeEdge::bottom_right: break;
  }
  return xdg_toplevel_resize_edge_bottom_right;
}

} // namespace cgpui
