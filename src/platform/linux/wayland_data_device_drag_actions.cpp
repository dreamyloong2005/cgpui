#include "wayland_data_device_internal.hpp"

namespace cgpui {

std::uint32_t WaylandDataDevice::drag_supported_actions() {
  return WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY |
         WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE;
}

DragDropAction WaylandDataDevice::drag_action_from_wayland(
    std::uint32_t action) {
  if (action == WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY) {
    return DragDropAction::copy;
  }
  if (action == WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE) {
    return DragDropAction::move;
  }
  return DragDropAction::none;
}

std::uint32_t WaylandDataDevice::data_offer_version(wl_data_offer* offer) {
  if (offer == nullptr) {
    return 0;
  }
  return wl_proxy_get_version(reinterpret_cast<wl_proxy*>(offer));
}

std::uint32_t WaylandDataDevice::active_offer_client_actions() const {
  if (active_offer_ == nullptr) {
    return WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;
  }
  return drag_supported_actions();
}

std::uint32_t WaylandDataDevice::preferred_drag_action() const {
  if (active_offer_ == nullptr) {
    return WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;
  }

  const std::uint32_t client_actions = active_offer_client_actions();
  const std::uint32_t selected_action = active_offer_->selected_action;
  if ((selected_action == WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY ||
       selected_action == WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE) &&
      (client_actions & selected_action) != 0) {
    return selected_action;
  }
  if ((client_actions & WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY) != 0) {
    return WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY;
  }
  if ((client_actions & WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE) != 0) {
    return WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE;
  }
  return WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;
}

void WaylandDataDevice::active_offer_negotiation_changed(Offer& offer) {
  if (active_offer_.get() == &offer && offer.entered) {
    negotiate_active_offer(offer.enter_serial);
  }
}

void WaylandDataDevice::negotiate_active_offer(std::uint32_t serial) {
  if (active_offer_ == nullptr || active_offer_->offer == nullptr) {
    return;
  }

  const std::optional<std::string> mime_type = preferred_drag_mime_type();
  active_offer_->accepted = mime_type.has_value();
  wl_data_offer_accept(
      active_offer_->offer,
      serial,
      mime_type.has_value() ? mime_type->c_str() : nullptr);

  if (data_offer_version(active_offer_->offer) >= 3) {
    wl_data_offer_set_actions(
        active_offer_->offer,
        active_offer_client_actions(),
        preferred_drag_action());
  }
  if (display_ != nullptr) {
    (void)wl_display_flush(display_);
  }
}

void WaylandDataDevice::finish_active_offer() {
  if (active_offer_ == nullptr || active_offer_->offer == nullptr ||
      active_offer_->finished || !active_offer_->accepted ||
      current_drag_action() == DragDropAction::none) {
    return;
  }
  if (data_offer_version(active_offer_->offer) < 3) {
    return;
  }
  active_offer_->finished = true;
  wl_data_offer_finish(active_offer_->offer);
  if (display_ != nullptr) {
    (void)wl_display_flush(display_);
  }
}

DragDropAction WaylandDataDevice::current_drag_action() const {
  if (active_offer_ == nullptr) {
    return DragDropAction::none;
  }
  return drag_action_from_wayland(active_offer_->selected_action);
}

} // namespace cgpui
