#pragma once

#include <wayland-client.h>

#include <cstdint>

struct wp_fractional_scale_manager_v1;
struct wp_fractional_scale_v1;
struct wp_viewporter;
struct wp_viewport;

extern const wl_interface wp_fractional_scale_manager_v1_interface;
extern const wl_interface wp_fractional_scale_v1_interface;
extern const wl_interface wp_viewporter_interface;
extern const wl_interface wp_viewport_interface;

struct wp_fractional_scale_v1_listener {
  void (*preferred_scale)(
      void* data,
      wp_fractional_scale_v1* fractional_scale,
      std::uint32_t scale);
};

void wp_fractional_scale_manager_v1_destroy(
    wp_fractional_scale_manager_v1* manager);
wp_fractional_scale_v1* wp_fractional_scale_manager_v1_get_fractional_scale(
    wp_fractional_scale_manager_v1* manager,
    wl_surface* surface);
int wp_fractional_scale_v1_add_listener(
    wp_fractional_scale_v1* fractional_scale,
    const wp_fractional_scale_v1_listener* listener,
    void* data);
void wp_fractional_scale_v1_destroy(
    wp_fractional_scale_v1* fractional_scale);
void wp_viewporter_destroy(wp_viewporter* viewporter);
wp_viewport* wp_viewporter_get_viewport(
    wp_viewporter* viewporter,
    wl_surface* surface);
void wp_viewport_destroy(wp_viewport* viewport);
void wp_viewport_set_destination(
    wp_viewport* viewport,
    std::int32_t width,
    std::int32_t height);
