#include "wayland_data_device_internal.hpp"

namespace cgpui {

void WaylandDataDevice::set_display(wl_display* display) {
  display_ = display;
}

void WaylandDataDevice::set_manager(wl_data_device_manager* manager) {
  manager_ = manager;
}

void WaylandDataDevice::set_window_lookup(WindowLookup lookup) {
  find_window_ = std::move(lookup);
}

void WaylandDataDevice::bind_to_seat(wl_seat* seat) {
  if (manager_ == nullptr || seat == nullptr || data_device_ != nullptr) {
    return;
  }

  data_device_ = wl_data_device_manager_get_data_device(manager_, seat);
  if (data_device_ == nullptr) {
    return;
  }

  static const wl_data_device_listener data_device_listener{
      .data_offer = &WaylandDataDevice::handle_data_offer,
      .enter = &WaylandDataDevice::handle_enter,
      .leave = &WaylandDataDevice::handle_leave,
      .motion = &WaylandDataDevice::handle_motion,
      .drop = &WaylandDataDevice::handle_drop,
      .selection = &WaylandDataDevice::handle_selection,
  };
  wl_data_device_add_listener(data_device_, &data_device_listener, this);
}

void WaylandDataDevice::reset_device() {
  clear_active_offer();
  clear_offer(pending_offer_);
  drag_window_ = nullptr;
  last_drag_position_ = {};
  if (data_device_ != nullptr) {
    wl_data_device_destroy(data_device_);
    data_device_ = nullptr;
  }
}

void WaylandDataDeviceDeleter::operator()(WaylandDataDevice* data_device) const {
  delete data_device;
}

WaylandDataDevicePtr create_wayland_data_device() {
  return WaylandDataDevicePtr(new WaylandDataDevice());
}

void wayland_data_device_set_display(
    WaylandDataDevice& data_device,
    wl_display* display) {
  data_device.set_display(display);
}

void wayland_data_device_set_manager(
    WaylandDataDevice& data_device,
    wl_data_device_manager* manager) {
  data_device.set_manager(manager);
}

void wayland_data_device_set_window_lookup(
    WaylandDataDevice& data_device,
    std::function<WaylandWindow*(wl_surface*)> lookup) {
  data_device.set_window_lookup(std::move(lookup));
}

void wayland_data_device_bind_to_seat(
    WaylandDataDevice& data_device,
    wl_seat* seat) {
  data_device.bind_to_seat(seat);
}

void wayland_data_device_reset(WaylandDataDevice& data_device) {
  data_device.reset_device();
}

} // namespace cgpui
