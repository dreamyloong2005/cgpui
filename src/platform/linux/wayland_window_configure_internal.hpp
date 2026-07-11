#pragma once

void record_toplevel_configure_state(
    std::int32_t width,
    std::int32_t height,
    wl_array* states);
[[nodiscard]] bool acknowledge_configure(std::uint32_t serial);
void dispatch_configure_lifecycle_events(
    WaylandXdgToplevelState previous,
    WaylandXdgToplevelState current);
static void handle_surface_configure(
    void* data,
    xdg_surface* surface,
    std::uint32_t serial);
static void handle_toplevel_configure(
    void* data,
    xdg_toplevel* toplevel,
    std::int32_t width,
    std::int32_t height,
    wl_array* states);
static void handle_toplevel_close(void* data, xdg_toplevel* toplevel);
