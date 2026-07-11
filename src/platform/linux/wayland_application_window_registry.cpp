#include "wayland_application_internal.hpp"

namespace cgpui {

void WaylandApplication::configure_data_device_lookup() {
  wayland_data_device_set_window_lookup(
      *data_device_,
      [this](wl_surface* surface) { return find_window(surface); });
}

void WaylandApplication::configure_text_input_lookup() {
  wayland_text_input_set_window_lookup(
      *text_input_,
      [this](wl_surface* surface) { return find_window(surface); });
  wayland_text_input_set_modifiers_provider(
      *text_input_,
      [this] { return wayland_keyboard_modifiers(keyboard_state_); });
}

void WaylandApplication::configure_output_scale_registry() {
  output_scales_.set_change_callback(
      [this](wl_output* output, std::int32_t scale, bool present) {
        for (WaylandWindow* window : windows_) {
          wayland_window_output_scale_changed(
              *window, output, scale, present);
        }
      });
}

void WaylandApplication::handle_global(
    void* data,
    wl_registry* registry,
    std::uint32_t name,
    const char* interface,
    std::uint32_t version) {
  auto* app = static_cast<WaylandApplication*>(data);
  wayland_bind_registry_global(
      registry,
      name,
      interface,
      version,
      WaylandRegistryBindings{
          .compositor = &app->compositor_,
          .shm = &app->shm_,
          .shell = &app->shell_,
          .decoration_manager = &app->decoration_manager_,
          .seat = &app->seat_,
          .output_scales = &app->output_scales_,
          .data_device_manager = &app->data_device_manager_,
          .text_input_manager = &app->text_input_manager_,
          .fractional_scale_manager = &app->fractional_scale_manager_,
          .viewporter = &app->viewporter_,
          .data_device = app->data_device_.get(),
          .text_input = app->text_input_.get(),
      });
}

void WaylandApplication::handle_global_remove(
    void* data,
    wl_registry* registry,
    std::uint32_t name) {
  (void)registry;
  auto* app = static_cast<WaylandApplication*>(data);
  app->output_scales_.remove(name);
}

void WaylandApplication::handle_shell_ping(
    void* data,
    xdg_wm_base* shell,
    std::uint32_t serial) {
  (void)data;
  xdg_wm_base_pong(shell, serial);
}

void WaylandApplication::register_window(WaylandWindow* window) {
  windows_.push_back(window);
}

void WaylandApplication::unregister_window(WaylandWindow* window) {
  std::erase(windows_, window);
  if (pointer_window_ == window) {
    pointer_window_ = nullptr;
    pointer_scroll_frame_ = {};
  }
  if (keyboard_window_ == window) {
    keyboard_window_ = nullptr;
  }
}

WaylandWindow* WaylandApplication::find_window(wl_surface* surface) const {
  for (WaylandWindow* window : windows_) {
    if (wayland_window_surface(*window) == surface) {
      return window;
    }
  }
  return nullptr;
}

} // namespace cgpui
