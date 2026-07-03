#pragma once

#include <wayland-client.h>

#include <cstdint>

struct xdg_positioner;
struct xdg_wm_base;
struct xdg_surface;
struct xdg_toplevel;
struct zwp_text_input_manager_v3;
struct zwp_text_input_v3;

extern const wl_interface xdg_positioner_interface;
extern const wl_interface xdg_wm_base_interface;
extern const wl_interface xdg_surface_interface;
extern const wl_interface xdg_toplevel_interface;
extern const wl_interface zwp_text_input_manager_v3_interface;
extern const wl_interface zwp_text_input_v3_interface;

constexpr std::uint32_t xdg_toplevel_state_maximized = 1;
constexpr std::uint32_t xdg_toplevel_state_fullscreen = 2;
constexpr std::uint32_t xdg_toplevel_state_activated = 4;

struct xdg_wm_base_listener {
  void (*ping)(void* data, xdg_wm_base* shell, std::uint32_t serial);
};

struct xdg_surface_listener {
  void (*configure)(void* data, xdg_surface* surface, std::uint32_t serial);
};

struct xdg_toplevel_listener {
  void (*configure)(
      void* data,
      xdg_toplevel* toplevel,
      std::int32_t width,
      std::int32_t height,
      wl_array* states);
  void (*close)(void* data, xdg_toplevel* toplevel);
};

struct zwp_text_input_v3_listener {
  void (*enter)(void* data, zwp_text_input_v3* text_input, wl_surface* surface);
  void (*leave)(void* data, zwp_text_input_v3* text_input, wl_surface* surface);
  void (*preedit_string)(
      void* data,
      zwp_text_input_v3* text_input,
      const char* text,
      std::int32_t cursor_begin,
      std::int32_t cursor_end);
  void (*commit_string)(
      void* data,
      zwp_text_input_v3* text_input,
      const char* text);
  void (*delete_surrounding_text)(
      void* data,
      zwp_text_input_v3* text_input,
      std::uint32_t before_length,
      std::uint32_t after_length);
  void (*done)(
      void* data,
      zwp_text_input_v3* text_input,
      std::uint32_t serial);
};

int xdg_wm_base_add_listener(
    xdg_wm_base* shell,
    const xdg_wm_base_listener* listener,
    void* data);
void xdg_wm_base_destroy(xdg_wm_base* shell);
xdg_surface* xdg_wm_base_get_xdg_surface(
    xdg_wm_base* shell,
    wl_surface* surface);
void xdg_wm_base_pong(xdg_wm_base* shell, std::uint32_t serial);
int xdg_surface_add_listener(
    xdg_surface* surface,
    const xdg_surface_listener* listener,
    void* data);
void xdg_surface_destroy(xdg_surface* surface);
xdg_toplevel* xdg_surface_get_toplevel(xdg_surface* surface);
void xdg_surface_ack_configure(xdg_surface* surface, std::uint32_t serial);
int xdg_toplevel_add_listener(
    xdg_toplevel* toplevel,
    const xdg_toplevel_listener* listener,
    void* data);
void xdg_toplevel_destroy(xdg_toplevel* toplevel);
void xdg_toplevel_set_title(xdg_toplevel* toplevel, const char* title);
void zwp_text_input_manager_v3_destroy(zwp_text_input_manager_v3* manager);
zwp_text_input_v3* zwp_text_input_manager_v3_get_text_input(
    zwp_text_input_manager_v3* manager,
    wl_seat* seat);
int zwp_text_input_v3_add_listener(
    zwp_text_input_v3* text_input,
    const zwp_text_input_v3_listener* listener,
    void* data);
void zwp_text_input_v3_destroy(zwp_text_input_v3* text_input);
void zwp_text_input_v3_enable(zwp_text_input_v3* text_input);
void zwp_text_input_v3_disable(zwp_text_input_v3* text_input);
void zwp_text_input_v3_set_surrounding_text(
    zwp_text_input_v3* text_input,
    const char* text,
    std::int32_t cursor,
    std::int32_t anchor);
void zwp_text_input_v3_set_content_type(
    zwp_text_input_v3* text_input,
    std::uint32_t hint,
    std::uint32_t purpose);
void zwp_text_input_v3_set_cursor_rectangle(
    zwp_text_input_v3* text_input,
    std::int32_t x,
    std::int32_t y,
    std::int32_t width,
    std::int32_t height);
void zwp_text_input_v3_commit(zwp_text_input_v3* text_input);
