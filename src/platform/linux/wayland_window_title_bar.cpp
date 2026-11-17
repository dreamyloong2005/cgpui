#include "wayland_window_internal.hpp"
#include "wayland_protocol_xdg_toplevel_interactive_internal.hpp"
#include "wayland_window_title_bar_api.hpp"

#include <cmath>

namespace cgpui {
namespace {

bool is_button(WindowControlArea area) {
  return area == WindowControlArea::minimize_button ||
      area == WindowControlArea::maximize_button ||
      area == WindowControlArea::close_button;
}

PlatformWindowActionResult missing_serial() {
  return PlatformWindowActionResult{
      .reason = "no pointer button press on this window to take a serial from"};
}

} // namespace

void wayland_window_title_bar_pointer_button(
    WaylandWindow& window, wl_seat* seat, std::uint32_t serial,
    std::uint32_t time, MouseButton button, bool pressed, Point position) {
  window.title_bar_pointer_button(seat, serial, time, button, pressed, position);
}

bool WaylandWindow::client_title_bar_active() const {
  return chrome_state_.platform.applied.title_bar == WindowTitleBarMode::client &&
      !pending_configure_.current_toplevel_state.fullscreen;
}

void WaylandWindow::set_window_control_regions(
    std::vector<WindowControlRegion> regions) {
  title_bar_.regions = std::move(regions);
}

void WaylandWindow::title_bar_pointer_button(
    wl_seat* seat, std::uint32_t serial, std::uint32_t time,
    MouseButton button, bool pressed, Point position) {
  if (pressed) {
    title_bar_.seat = seat;
    title_bar_.press_serial = serial;
  }
  if (!client_title_bar_active() || seat == nullptr || toplevel_ == nullptr) return;
  const WaylandXdgToplevelState current = pending_configure_.current_toplevel_state;
  const WaylandTitleBarHit hit = wayland_title_bar_hit(
      title_bar_.regions, position, logical_size_,
      chrome_state_.platform.applied.resizable && !current.maximized);
  const bool caption = hit.area == WindowControlArea::caption;
  if (button == MouseButton::right && pressed && caption) {
    xdg_toplevel_show_window_menu(
        toplevel_, seat, serial, static_cast<std::int32_t>(position.x),
        static_cast<std::int32_t>(position.y));
  } else if (button == MouseButton::left && pressed && hit.resize_edge) {
    xdg_toplevel_resize(toplevel_, seat, serial, *hit.resize_edge);
  } else if (button == MouseButton::left && pressed && caption) {
    const Point last = title_bar_.last_caption_press_position;
    const bool double_click = title_bar_.last_caption_press_time.has_value() &&
        time - *title_bar_.last_caption_press_time <= wayland_double_click_ms &&
        std::fabs(position.x - last.x) <= wayland_double_click_distance &&
        std::fabs(position.y - last.y) <= wayland_double_click_distance;
    title_bar_.last_caption_press_time.reset();
    if (double_click) {
      (void)perform_window_action({.action = PlatformWindowAction::toggle_maximize});
      return;
    }
    title_bar_.last_caption_press_time = time;
    title_bar_.last_caption_press_position = position;
    xdg_toplevel_move(toplevel_, seat, serial);
  } else if (button == MouseButton::left && pressed && hit.area && is_button(*hit.area)) {
    title_bar_.pressed_button = hit.area;
    return;
  } else if (button == MouseButton::left && !pressed && title_bar_.pressed_button) {
    const WindowControlArea pressed_button = *title_bar_.pressed_button;
    title_bar_.pressed_button.reset();
    if (hit.area != pressed_button) return;
    if (pressed_button == WindowControlArea::close_button) {
      close_requested(WindowCloseRequestSource::window_manager);
    } else {
      (void)perform_window_action({.action =
          pressed_button == WindowControlArea::minimize_button
              ? PlatformWindowAction::minimize
              : PlatformWindowAction::toggle_maximize});
    }
    return;
  } else {
    return;
  }
  (void)wl_display_flush(display_);
}

PlatformWindowFrameMetrics WaylandWindow::frame_metrics() const {
  const bool client = client_title_bar_active();
  const bool tiled = pending_configure_.current_toplevel_state.maximized;
  const float band = client && !tiled && chrome_state_.platform.applied.resizable
      ? wayland_client_resize_border : 0.0F;
  return PlatformWindowFrameMetrics{
      .supported = true,
      .client_title_bar = client,
      .backend = "wayland",
      .resize_border = wayland_client_resize_border,
      .client_resize_border = {.top = band, .right = band, .bottom = band, .left = band},
      .mechanism = chrome_state_.xdg_decoration_supported
          ? "zxdg_toplevel_decoration_v1 client_side + xdg_toplevel move/resize"
          : "no zxdg_decoration_manager_v1 (client-side by default) + "
            "xdg_toplevel move/resize",
      .reason = "Wayland has no title bar metric; caption_height is 0",
  };
}

PlatformWindowActionResult WaylandWindow::perform_window_action(
    PlatformWindowActionRequest request) {
  const bool interactive = request.action == PlatformWindowAction::begin_move ||
      request.action == PlatformWindowAction::begin_resize ||
      request.action == PlatformWindowAction::show_window_menu;
  if (!interactive) return PlatformWindow::perform_window_action(request);
  if (toplevel_ == nullptr || title_bar_.seat == nullptr ||
      title_bar_.press_serial == 0) {
    return missing_serial();
  }
  if (request.action == PlatformWindowAction::begin_move) {
    xdg_toplevel_move(toplevel_, title_bar_.seat, title_bar_.press_serial);
  } else if (request.action == PlatformWindowAction::begin_resize) {
    xdg_toplevel_resize(toplevel_, title_bar_.seat, title_bar_.press_serial,
                        wayland_resize_edge(request.edge));
  } else {
    xdg_toplevel_show_window_menu(
        toplevel_, title_bar_.seat, title_bar_.press_serial,
        static_cast<std::int32_t>(request.position.x),
        static_cast<std::int32_t>(request.position.y));
  }
  (void)wl_display_flush(display_);
  return PlatformWindowActionResult{.supported = true, .performed = true};
}

} // namespace cgpui
