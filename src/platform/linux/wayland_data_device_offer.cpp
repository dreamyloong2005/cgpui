#include "wayland_data_device_internal.hpp"

namespace cgpui {

void WaylandDataDevice::handle_data_offer(
    void* data,
    wl_data_device* data_device,
    wl_data_offer* offer) {
  (void)data_device;
  auto* self = static_cast<WaylandDataDevice*>(data);
  self->clear_offer(self->pending_offer_);
  self->pending_offer_ = std::make_unique<Offer>();
  self->pending_offer_->offer = offer;
  static const wl_data_offer_listener offer_listener{
      .offer = &WaylandDataDevice::handle_offer_mime_type,
      .source_actions = &WaylandDataDevice::handle_offer_source_actions,
      .action = &WaylandDataDevice::handle_offer_action,
  };
  wl_data_offer_add_listener(
      offer,
      &offer_listener,
      self->pending_offer_.get());
}

void WaylandDataDevice::handle_offer_mime_type(
    void* data,
    wl_data_offer*,
    const char* mime_type) {
  auto* offer = static_cast<Offer*>(data);
  if (offer != nullptr && mime_type != nullptr) {
    offer->mime_types.emplace_back(mime_type);
  }
}

void WaylandDataDevice::handle_offer_source_actions(
    void* data,
    wl_data_offer*,
    std::uint32_t source_actions) {
  auto* offer = static_cast<Offer*>(data);
  if (offer != nullptr) {
    offer->source_actions = source_actions;
  }
}

void WaylandDataDevice::handle_offer_action(
    void* data,
    wl_data_offer*,
    std::uint32_t action) {
  auto* offer = static_cast<Offer*>(data);
  if (offer != nullptr) {
    offer->selected_action = action;
  }
}

void WaylandDataDevice::handle_selection(
    void* data,
    wl_data_device* data_device,
    wl_data_offer* offer) {
  (void)data_device;
  auto* self = static_cast<WaylandDataDevice*>(data);
  if (offer != nullptr) {
    if (self->pending_offer_ != nullptr &&
        self->pending_offer_->offer == offer) {
      self->clear_offer(self->pending_offer_);
    } else {
      wl_data_offer_destroy(offer);
    }
  }
}

void WaylandDataDevice::replace_active_offer(wl_data_offer* offer) {
  if (active_offer_ != nullptr && active_offer_->offer != offer) {
    clear_offer(active_offer_);
  }
  if (offer == nullptr) {
    return;
  }
  if (pending_offer_ != nullptr && pending_offer_->offer == offer) {
    active_offer_ = std::move(pending_offer_);
  } else if (active_offer_ == nullptr) {
    active_offer_ = std::make_unique<Offer>();
    active_offer_->offer = offer;
  }
}

void WaylandDataDevice::clear_active_offer() {
  clear_offer(active_offer_);
}

void WaylandDataDevice::clear_offer(std::unique_ptr<Offer>& offer) {
  if (offer != nullptr && offer->offer != nullptr) {
    wl_data_offer_destroy(offer->offer);
  }
  offer.reset();
}

} // namespace cgpui
