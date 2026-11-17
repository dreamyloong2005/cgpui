#include "wayland_test_background_effect.hpp"
#include "wayland_protocol_background_effect_internal.hpp"

#include <wayland-server-core.h>

namespace cgpui::test {
namespace {

WaylandFakeBackgroundEffect* fake(wl_resource* resource) {
  return static_cast<WaylandFakeBackgroundEffect*>(
      wl_resource_get_user_data(resource));
}

void destroy(wl_client*, wl_resource* resource) { wl_resource_destroy(resource); }

void set_blur_region(wl_client*, wl_resource* resource, wl_resource* region) {
  auto& counter = region != nullptr ? fake(resource)->blur_regions_set
                                    : fake(resource)->blur_regions_cleared;
  counter.fetch_add(1);
}

struct SurfaceImplementation {
  void (*destroy)(wl_client*, wl_resource*);
  void (*set_blur_region)(wl_client*, wl_resource*, wl_resource*);
};
const SurfaceImplementation surface_implementation{&destroy, &set_blur_region};

void get_background_effect(
    wl_client* client, wl_resource* resource, std::uint32_t id, wl_resource*) {
  wl_resource* effect = wl_resource_create(
      client, &ext_background_effect_surface_v1_interface, 1, id);
  wl_resource_set_implementation(
      effect, &surface_implementation, fake(resource), nullptr);
  fake(resource)->effects_created.fetch_add(1);
}

struct ManagerImplementation {
  void (*destroy)(wl_client*, wl_resource*);
  void (*get_background_effect)(
      wl_client*, wl_resource*, std::uint32_t, wl_resource*);
};
const ManagerImplementation manager_implementation{
    &destroy, &get_background_effect};

void bind_manager(wl_client* client, void* data, std::uint32_t version,
                  std::uint32_t id) {
  wl_resource* resource = wl_resource_create(
      client, &ext_background_effect_manager_v1_interface,
      static_cast<int>(version), id);
  auto* state = static_cast<WaylandFakeBackgroundEffect*>(data);
  wl_resource_set_implementation(
      resource, &manager_implementation, state, nullptr);
  wl_resource_post_event(resource, 0, state->capabilities);
}

} // namespace

void WaylandFakeBackgroundEffect::install(wl_display* display) {
  wl_global_create(
      display, &ext_background_effect_manager_v1_interface, 1, this,
      &bind_manager);
}

} // namespace cgpui::test
