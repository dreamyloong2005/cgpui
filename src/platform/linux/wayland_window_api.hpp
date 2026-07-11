#pragma once

#include "cgpui/platform/platform.hpp"
#include "cgpui/ui/text.hpp"
#include "wayland_protocol_internal.hpp"

#include <wayland-client.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>

namespace cgpui {

class WaylandWindow;

struct WaylandWindowDeleter {
  void operator()(WaylandWindow* window) const;
};
using WaylandWindowPtr = std::unique_ptr<WaylandWindow, WaylandWindowDeleter>;
using WaylandWindowUnregisterCallback = std::function<void(WaylandWindow*)>;
using WaylandWindowCursorCallback =
    std::function<void(WaylandWindow&, CursorShape)>;
using WaylandWindowImePlacementCallback = std::function<void(
    WaylandWindow&,
    std::optional<ImeTextInputPlacement>)>;
using WaylandOutputScaleLookup =
    std::function<std::int32_t(wl_output*)>;

Result<WaylandWindowPtr> create_wayland_window(
    wl_display* display,
    wl_compositor* compositor,
    xdg_wm_base* shell,
    zxdg_decoration_manager_v1* decoration_manager,
    xdg_toplevel* parent,
    const WindowDescriptor& descriptor,
    PlatformEventCallback callback,
    bool text_input_available,
    WaylandOutputScaleLookup output_scale_lookup);
std::unique_ptr<PlatformWindow> make_registered_wayland_window(
    WaylandWindowPtr window,
    WaylandWindowUnregisterCallback unregister,
    WaylandWindowCursorCallback set_cursor,
    WaylandWindowImePlacementCallback set_ime_placement);
[[nodiscard]] wl_surface* wayland_window_surface(WaylandWindow& window);
[[nodiscard]] WindowState wayland_window_state(const WaylandWindow& window);
[[nodiscard]] CursorShape wayland_window_cursor_shape(
    const WaylandWindow& window);
void wayland_window_set_cursor(WaylandWindow& window, CursorShape cursor_shape);
void wayland_window_set_ime_text_input_placement(
    WaylandWindow& window,
    std::optional<ImeTextInputPlacement> placement);
void wayland_window_pointer_moved(WaylandWindow& window, Point position);
void wayland_window_pointer_exited(WaylandWindow& window, Point position);
void wayland_window_pointer_button(
    WaylandWindow& window,
    MouseButton button,
    bool pressed,
    Point position);
void wayland_window_pointer_scrolled(
    WaylandWindow& window,
    Point delta,
    Point position);
void wayland_window_drag_entered(
    WaylandWindow& window,
    Point position,
    DragDropPayload payload,
    DragDropAction action);
void wayland_window_drag_updated(
    WaylandWindow& window,
    Point position,
    DragDropPayload payload,
    DragDropAction action);
void wayland_window_drag_dropped(
    WaylandWindow& window,
    Point position,
    DragDropPayload payload,
    DragDropAction action);
void wayland_window_drag_exited(WaylandWindow& window, Point position);
void wayland_window_wakeup_requested(WaylandWindow& window);
void wayland_window_keyboard_key(
    WaylandWindow& window,
    std::uint32_t key,
    KeyAction action,
    KeyboardModifiers modifiers);
void wayland_window_text_input(
    WaylandWindow& window,
    std::string text,
    KeyboardModifiers modifiers);
void wayland_window_text_input_entered(WaylandWindow& window);
void wayland_window_text_input_left(
    WaylandWindow& window,
    KeyboardModifiers modifiers);
void wayland_window_text_input_preedit(
    WaylandWindow& window,
    std::string text,
    std::int32_t cursor_begin,
    std::int32_t cursor_end,
    KeyboardModifiers modifiers,
    std::uint32_t serial);
void wayland_window_text_input_commit(
    WaylandWindow& window,
    std::string text,
    KeyboardModifiers modifiers,
    std::uint32_t serial);
void wayland_window_text_input_delete_surrounding(
    WaylandWindow& window,
    std::uint32_t before_length,
    std::uint32_t after_length,
    KeyboardModifiers modifiers,
    std::uint32_t serial);
void wayland_window_text_input_surrounding_text(
    WaylandWindow& window,
    std::string text,
    std::int32_t cursor,
    std::int32_t anchor);
void wayland_window_text_input_content_type(
    WaylandWindow& window,
    std::uint32_t hint,
    std::uint32_t purpose);
void wayland_window_focus_changed(WaylandWindow& window, bool focused);
void wayland_window_output_scale_changed(
    WaylandWindow& window,
    wl_output* output,
    std::int32_t scale,
    bool present);

} // namespace cgpui
