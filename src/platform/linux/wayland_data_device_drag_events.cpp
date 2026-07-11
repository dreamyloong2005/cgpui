#include "wayland_data_device_internal.hpp"

namespace cgpui {

void WaylandDataDevice::handle_enter(
    void* data,
    wl_data_device* data_device,
    std::uint32_t serial,
    wl_surface* surface,
    wl_fixed_t x,
    wl_fixed_t y,
    wl_data_offer* offer) {
  (void)data_device;
  auto* self = static_cast<WaylandDataDevice*>(data);
  self->drag_window_ = self->find_window_ ? self->find_window_(surface) : nullptr;
  self->last_drag_position_ = point_from_fixed(x, y);
  self->replace_active_offer(offer);
  if (self->active_offer_ != nullptr) {
    self->active_offer_->enter_serial = serial;
    self->active_offer_->entered = true;
  }
  self->negotiate_active_offer(serial);
  if (self->drag_window_ != nullptr) {
    wayland_window_drag_entered(
        *self->drag_window_,
        self->last_drag_position_,
        self->payload_from_active_offer(),
        self->current_drag_action());
  }
}

void WaylandDataDevice::handle_leave(void* data, wl_data_device* data_device) {
  (void)data_device;
  auto* self = static_cast<WaylandDataDevice*>(data);
  if (self->drag_window_ != nullptr) {
    wayland_window_drag_exited(*self->drag_window_, self->last_drag_position_);
  }
  self->drag_window_ = nullptr;
  self->clear_active_offer();
}

void WaylandDataDevice::handle_motion(
    void* data,
    wl_data_device* data_device,
    std::uint32_t time,
    wl_fixed_t x,
    wl_fixed_t y) {
  (void)data_device;
  (void)time;
  auto* self = static_cast<WaylandDataDevice*>(data);
  self->last_drag_position_ = point_from_fixed(x, y);
  if (self->drag_window_ != nullptr) {
    wayland_window_drag_updated(
        *self->drag_window_,
        self->last_drag_position_,
        self->payload_from_active_offer(),
        self->current_drag_action());
  }
}

void WaylandDataDevice::handle_drop(void* data, wl_data_device* data_device) {
  (void)data_device;
  auto* self = static_cast<WaylandDataDevice*>(data);
  DragDropPayload payload = self->payload_from_active_offer();
  const DragDropAction action = self->current_drag_action();
  if (self->drag_window_ != nullptr) {
    wayland_window_drag_dropped(
        *self->drag_window_,
        self->last_drag_position_,
        std::move(payload),
        action);
  }
  self->finish_active_offer();
}

} // namespace cgpui
