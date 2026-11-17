#include "wayland_protocol_background_effect_internal.hpp"

namespace {

template <typename T>
wl_proxy* proxy(T* object) {
  return reinterpret_cast<wl_proxy*>(object);
}

template <typename T>
std::uint32_t version(T* object) {
  return wl_proxy_get_version(proxy(object));
}

} // namespace

int ext_background_effect_manager_v1_add_listener(
    ext_background_effect_manager_v1* manager,
    const ext_background_effect_manager_v1_listener* listener,
    void* data) {
  return wl_proxy_add_listener(
      proxy(manager),
      reinterpret_cast<void (**)(void)>(
          const_cast<ext_background_effect_manager_v1_listener*>(listener)),
      data);
}

void ext_background_effect_manager_v1_destroy(
    ext_background_effect_manager_v1* manager) {
  wl_proxy_marshal_flags(proxy(manager), 0, nullptr, version(manager),
                         WL_MARSHAL_FLAG_DESTROY);
}

ext_background_effect_surface_v1*
ext_background_effect_manager_v1_get_background_effect(
    ext_background_effect_manager_v1* manager,
    wl_surface* surface) {
  return reinterpret_cast<ext_background_effect_surface_v1*>(
      wl_proxy_marshal_flags(
          proxy(manager), 1, &ext_background_effect_surface_v1_interface,
          version(manager), 0, nullptr, surface));
}

void ext_background_effect_surface_v1_destroy(
    ext_background_effect_surface_v1* effect) {
  wl_proxy_marshal_flags(proxy(effect), 0, nullptr, version(effect),
                         WL_MARSHAL_FLAG_DESTROY);
}

void ext_background_effect_surface_v1_set_blur_region(
    ext_background_effect_surface_v1* effect,
    wl_region* region) {
  wl_proxy_marshal_flags(proxy(effect), 1, nullptr, version(effect), 0,
                         region);
}

// org_kde_kwin_blur_manager v1 has no destructor request.
void org_kde_kwin_blur_manager_destroy(org_kde_kwin_blur_manager* manager) {
  wl_proxy_destroy(proxy(manager));
}

org_kde_kwin_blur* org_kde_kwin_blur_manager_create(
    org_kde_kwin_blur_manager* manager,
    wl_surface* surface) {
  return reinterpret_cast<org_kde_kwin_blur*>(wl_proxy_marshal_flags(
      proxy(manager), 0, &org_kde_kwin_blur_interface, version(manager), 0,
      nullptr, surface));
}

void org_kde_kwin_blur_manager_unset(
    org_kde_kwin_blur_manager* manager,
    wl_surface* surface) {
  wl_proxy_marshal_flags(proxy(manager), 1, nullptr, version(manager), 0,
                         surface);
}

void org_kde_kwin_blur_commit(org_kde_kwin_blur* blur) {
  wl_proxy_marshal_flags(proxy(blur), 0, nullptr, version(blur), 0);
}

void org_kde_kwin_blur_set_region(org_kde_kwin_blur* blur, wl_region* region) {
  wl_proxy_marshal_flags(proxy(blur), 1, nullptr, version(blur), 0, region);
}

void org_kde_kwin_blur_release(org_kde_kwin_blur* blur) {
  wl_proxy_marshal_flags(proxy(blur), 2, nullptr, version(blur),
                         WL_MARSHAL_FLAG_DESTROY);
}
