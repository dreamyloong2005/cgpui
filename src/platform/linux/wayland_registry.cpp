#include "wayland_internal.hpp"

namespace cgpui {

void wayland_bind_registry_global(
    wl_registry* registry,
    std::uint32_t name,
    const char* interface,
    std::uint32_t version,
    const WaylandRegistryBindings& bindings) {
  const std::string_view interface_name(interface);
  if (interface_name == wl_compositor_interface.name) {
    *bindings.compositor = static_cast<wl_compositor*>(wl_registry_bind(
        registry,
        name,
        &wl_compositor_interface,
        std::min<std::uint32_t>(version, 4)));
    return;
  }
  if (interface_name == xdg_wm_base_interface.name) {
    *bindings.shell = static_cast<xdg_wm_base*>(wl_registry_bind(
        registry,
        name,
        &xdg_wm_base_interface,
        1));
    return;
  }
  if (interface_name == wl_seat_interface.name) {
    *bindings.seat = static_cast<wl_seat*>(wl_registry_bind(
        registry,
        name,
        &wl_seat_interface,
        std::min<std::uint32_t>(version, 9)));
    return;
  }
  if (interface_name == wl_shm_interface.name) {
    *bindings.shm = static_cast<wl_shm*>(wl_registry_bind(
        registry, name, &wl_shm_interface, 1));
    return;
  }
  if (interface_name == zxdg_decoration_manager_v1_interface.name) {
    *bindings.decoration_manager =
        static_cast<zxdg_decoration_manager_v1*>(wl_registry_bind(
            registry, name, &zxdg_decoration_manager_v1_interface, 1));
    return;
  }
  if (interface_name == wp_fractional_scale_manager_v1_interface.name) {
    *bindings.fractional_scale_manager =
        static_cast<wp_fractional_scale_manager_v1*>(wl_registry_bind(
            registry, name, &wp_fractional_scale_manager_v1_interface, 1));
    return;
  }
  if (interface_name == wp_viewporter_interface.name) {
    *bindings.viewporter = static_cast<wp_viewporter*>(wl_registry_bind(
        registry, name, &wp_viewporter_interface, 1));
    return;
  }
  if (interface_name == wl_output_interface.name) {
    bindings.output_scales->bind(registry, name, version);
    return;
  }
  if (interface_name == wl_data_device_manager_interface.name) {
    *bindings.data_device_manager =
        static_cast<wl_data_device_manager*>(wl_registry_bind(
            registry,
            name,
            &wl_data_device_manager_interface,
            std::min<std::uint32_t>(version, 3)));
    wayland_data_device_set_manager(
        *bindings.data_device,
        *bindings.data_device_manager);
  }
  if (interface_name == zwp_text_input_manager_v3_interface.name) {
    *bindings.text_input_manager =
        static_cast<zwp_text_input_manager_v3*>(wl_registry_bind(
            registry,
            name,
            &zwp_text_input_manager_v3_interface,
            std::min<std::uint32_t>(version, 1)));
    wayland_text_input_set_manager(
        *bindings.text_input,
        *bindings.text_input_manager);
  }
}

} // namespace cgpui
