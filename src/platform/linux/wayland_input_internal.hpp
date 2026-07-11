#pragma once

#include "cgpui/platform/platform.hpp"
#include "cgpui/ui/text.hpp"
#include "wayland_protocol_internal.hpp"

#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>

#include <cstdint>
#include <atomic>
#include <string>
#include <string_view>
#include <vector>

namespace cgpui {

class WaylandWindow;
class WaylandTextInput;
class WaylandDataDevice;

Error wayland_error(ErrorCode code, std::string message);
MouseButton mouse_button_from_wayland(std::uint32_t button);
Point point_from_fixed(wl_fixed_t x, wl_fixed_t y);
std::vector<std::string> parse_uri_list(std::string_view payload);
KeyboardModifiers modifiers_from_xkb_state(xkb_state* state);

struct WaylandRegistryBindings {
  wl_compositor** compositor = nullptr;
  wl_shm** shm = nullptr;
  xdg_wm_base** shell = nullptr;
  zxdg_decoration_manager_v1** decoration_manager = nullptr;
  wl_seat** seat = nullptr;
  WaylandOutputScaleRegistry* output_scales = nullptr;
  wl_data_device_manager** data_device_manager = nullptr;
  zwp_text_input_manager_v3** text_input_manager = nullptr;
  wp_fractional_scale_manager_v1** fractional_scale_manager = nullptr;
  wp_viewporter** viewporter = nullptr;
  WaylandDataDevice* data_device = nullptr;
  WaylandTextInput* text_input = nullptr;
};

struct WaylandKeyboardState {
  xkb_context* context = nullptr;
  xkb_keymap* keymap = nullptr;
  xkb_state* state = nullptr;
  std::uint32_t mods_depressed = 0;
  std::uint32_t mods_latched = 0;
  std::uint32_t mods_locked = 0;
  std::uint32_t layout_group = 0;
};

enum class WaylandCursorThemeLoadStatus {
  unavailable,
  loaded,
};

enum class WaylandCursorImageStatus {
  cursor_image_unavailable,
  ready,
};

struct WaylandCursorImageState {
  CursorShape shape = CursorShape::default_arrow;
  std::string cursor_name = "left_ptr";
  WaylandCursorImageStatus status =
      WaylandCursorImageStatus::cursor_image_unavailable;
  std::int32_t hotspot_x = 0;
  std::int32_t hotspot_y = 0;
  std::int32_t buffer_scale = 1;
  std::string unavailable_reason;
};

struct WaylandCursorThemeState {
  WaylandCursorThemeLoadStatus cursor_theme_status =
      WaylandCursorThemeLoadStatus::unavailable;
  WaylandCursorImageState image;
  std::uint32_t serial = 0;
  std::uint32_t apply_count = 0;
};

[[nodiscard]] std::string cursor_name_for_shape(CursorShape shape);
int wayland_run_event_loop(
    wl_display* display,
    int wakeup_read_fd,
    std::atomic_bool& running,
    std::vector<WaylandWindow*>& windows);
void wayland_request_wakeup(int wakeup_write_fd);
void wayland_bind_registry_global(
    wl_registry* registry,
    std::uint32_t name,
    const char* interface,
    std::uint32_t version,
    const WaylandRegistryBindings& bindings);
void wayland_keyboard_load_keymap(
    WaylandKeyboardState& keyboard,
    std::uint32_t format,
    std::int32_t fd,
    std::uint32_t size);
void wayland_keyboard_reset(WaylandKeyboardState& keyboard);
void wayland_keyboard_update_modifiers(
    WaylandKeyboardState& keyboard,
    std::uint32_t mods_depressed,
    std::uint32_t mods_latched,
    std::uint32_t mods_locked,
    std::uint32_t group);
[[nodiscard]] KeyboardModifiers wayland_keyboard_modifiers(
    const WaylandKeyboardState& keyboard);
[[nodiscard]] std::string wayland_keyboard_text_for_key(
    const WaylandKeyboardState& keyboard,
    std::uint32_t key);

} // namespace cgpui
