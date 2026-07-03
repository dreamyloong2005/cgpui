#pragma once

static void handle_seat_capabilities(
    void* data,
    wl_seat* seat,
    std::uint32_t capabilities);
static void handle_seat_name(void* data, wl_seat* seat, const char* name);
static void handle_keyboard_keymap(
    void* data,
    wl_keyboard* keyboard,
    std::uint32_t format,
    std::int32_t fd,
    std::uint32_t size);
static void handle_keyboard_enter(
    void* data,
    wl_keyboard* keyboard,
    std::uint32_t serial,
    wl_surface* surface,
    wl_array* keys);
static void handle_keyboard_leave(
    void* data,
    wl_keyboard* keyboard,
    std::uint32_t serial,
    wl_surface* surface);
static void handle_keyboard_key(
    void* data,
    wl_keyboard* keyboard,
    std::uint32_t serial,
    std::uint32_t time,
    std::uint32_t key,
    std::uint32_t state);
static void handle_keyboard_modifiers(
    void* data,
    wl_keyboard* keyboard,
    std::uint32_t serial,
    std::uint32_t mods_depressed,
    std::uint32_t mods_latched,
    std::uint32_t mods_locked,
    std::uint32_t group);
static void handle_keyboard_repeat_info(
    void* data,
    wl_keyboard* keyboard,
    std::int32_t rate,
    std::int32_t delay);
static void handle_pointer_enter(
    void* data,
    wl_pointer* pointer,
    std::uint32_t serial,
    wl_surface* surface,
    wl_fixed_t surface_x,
    wl_fixed_t surface_y);
static void handle_pointer_leave(
    void* data,
    wl_pointer* pointer,
    std::uint32_t serial,
    wl_surface* surface);
static void handle_pointer_motion(
    void* data,
    wl_pointer* pointer,
    std::uint32_t time,
    wl_fixed_t surface_x,
    wl_fixed_t surface_y);
static void handle_pointer_button(
    void* data,
    wl_pointer* pointer,
    std::uint32_t serial,
    std::uint32_t time,
    std::uint32_t button,
    std::uint32_t state);
static void handle_pointer_axis(
    void* data,
    wl_pointer* pointer,
    std::uint32_t time,
    std::uint32_t axis,
    wl_fixed_t value);
static void handle_pointer_frame(void* data, wl_pointer* pointer);
static void handle_pointer_axis_source(
    void* data,
    wl_pointer* pointer,
    std::uint32_t axis_source);
static void handle_pointer_axis_stop(
    void* data,
    wl_pointer* pointer,
    std::uint32_t time,
    std::uint32_t axis);
static void handle_pointer_axis_discrete(
    void* data,
    wl_pointer* pointer,
    std::uint32_t axis,
    std::int32_t discrete);
static void handle_pointer_axis_value120(
    void* data,
    wl_pointer* pointer,
    std::uint32_t axis,
    std::int32_t value120);
static void handle_pointer_axis_relative_direction(
    void* data,
    wl_pointer* pointer,
    std::uint32_t axis,
    std::uint32_t direction);

void dispatch_pointer_scroll();
void set_window_ime_text_input_placement(
    WaylandWindow& window,
    std::optional<ImeTextInputPlacement> placement);
