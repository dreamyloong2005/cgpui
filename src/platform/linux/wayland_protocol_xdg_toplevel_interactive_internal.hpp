#pragma once

#include <cstdint>

struct wl_seat;
struct xdg_toplevel;

// xdg_toplevel requests that start compositor-driven interaction. Each needs
// the serial of the pointer button press that triggered it.
inline constexpr std::uint32_t xdg_toplevel_resize_edge_top = 1;
inline constexpr std::uint32_t xdg_toplevel_resize_edge_bottom = 2;
inline constexpr std::uint32_t xdg_toplevel_resize_edge_left = 4;
inline constexpr std::uint32_t xdg_toplevel_resize_edge_top_left = 5;
inline constexpr std::uint32_t xdg_toplevel_resize_edge_bottom_left = 6;
inline constexpr std::uint32_t xdg_toplevel_resize_edge_right = 8;
inline constexpr std::uint32_t xdg_toplevel_resize_edge_top_right = 9;
inline constexpr std::uint32_t xdg_toplevel_resize_edge_bottom_right = 10;

void xdg_toplevel_show_window_menu(
    xdg_toplevel* toplevel, wl_seat* seat, std::uint32_t serial,
    std::int32_t x, std::int32_t y);
void xdg_toplevel_move(
    xdg_toplevel* toplevel, wl_seat* seat, std::uint32_t serial);
void xdg_toplevel_resize(
    xdg_toplevel* toplevel, wl_seat* seat, std::uint32_t serial,
    std::uint32_t edges);
