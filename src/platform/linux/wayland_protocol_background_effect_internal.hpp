#pragma once

#include <wayland-client.h>

#include <cstdint>

// Hand-written client bindings for ext-background-effect-v1 (wayland-protocols
// staging) and KDE's org_kde_kwin_blur (plasma-wayland-protocols blur.xml),
// matching the request and event order of those XML files.
struct ext_background_effect_manager_v1;
struct ext_background_effect_surface_v1;
struct org_kde_kwin_blur_manager;
struct org_kde_kwin_blur;

extern const wl_interface ext_background_effect_manager_v1_interface;
extern const wl_interface ext_background_effect_surface_v1_interface;
extern const wl_interface org_kde_kwin_blur_manager_interface;
extern const wl_interface org_kde_kwin_blur_interface;

inline constexpr std::uint32_t ext_background_effect_manager_v1_capability_blur = 1;

struct ext_background_effect_manager_v1_listener {
  void (*capabilities)(
      void* data,
      ext_background_effect_manager_v1* manager,
      std::uint32_t flags);
};

int ext_background_effect_manager_v1_add_listener(
    ext_background_effect_manager_v1* manager,
    const ext_background_effect_manager_v1_listener* listener,
    void* data);
void ext_background_effect_manager_v1_destroy(
    ext_background_effect_manager_v1* manager);
ext_background_effect_surface_v1*
ext_background_effect_manager_v1_get_background_effect(
    ext_background_effect_manager_v1* manager,
    wl_surface* surface);
void ext_background_effect_surface_v1_destroy(
    ext_background_effect_surface_v1* effect);
void ext_background_effect_surface_v1_set_blur_region(
    ext_background_effect_surface_v1* effect,
    wl_region* region);

void org_kde_kwin_blur_manager_destroy(org_kde_kwin_blur_manager* manager);
org_kde_kwin_blur* org_kde_kwin_blur_manager_create(
    org_kde_kwin_blur_manager* manager,
    wl_surface* surface);
void org_kde_kwin_blur_manager_unset(
    org_kde_kwin_blur_manager* manager,
    wl_surface* surface);
void org_kde_kwin_blur_commit(org_kde_kwin_blur* blur);
void org_kde_kwin_blur_set_region(org_kde_kwin_blur* blur, wl_region* region);
void org_kde_kwin_blur_release(org_kde_kwin_blur* blur);
