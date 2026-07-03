#include "clipboard_wayland_internal.hpp"

namespace cgpui {

#if defined(__linux__)
void WaylandClipboard::Connection::handle_data_offer(
    void* data,
    wl_data_device*,
    wl_data_offer* offer) {
  auto* connection = static_cast<Connection*>(data);
  connection->clear_offer(connection->pending_offer_);
  connection->pending_offer_ = std::make_unique<Offer>();
  connection->pending_offer_->offer = offer;
  static const wl_data_offer_listener offer_listener{
      .offer = &Connection::handle_offer_mime_type,
      .source_actions = &Connection::handle_offer_source_actions,
      .action = &Connection::handle_offer_action,
  };
  wl_data_offer_add_listener(
      offer,
      &offer_listener,
      connection->pending_offer_.get());
}

void WaylandClipboard::Connection::handle_offer_mime_type(
    void* data,
    wl_data_offer*,
    const char* mime_type) {
  auto* offer = static_cast<Offer*>(data);
  if (offer != nullptr && mime_type != nullptr) {
    offer->mime_types.emplace_back(mime_type);
  }
}

void WaylandClipboard::Connection::handle_offer_source_actions(
    void*,
    wl_data_offer*,
    std::uint32_t) {}

void WaylandClipboard::Connection::handle_offer_action(
    void*,
    wl_data_offer*,
    std::uint32_t) {}

void WaylandClipboard::Connection::handle_enter(
    void*,
    wl_data_device*,
    std::uint32_t,
    wl_surface*,
    wl_fixed_t,
    wl_fixed_t,
    wl_data_offer*) {}

void WaylandClipboard::Connection::handle_leave(void*, wl_data_device*) {}

void WaylandClipboard::Connection::handle_motion(
    void*,
    wl_data_device*,
    std::uint32_t,
    wl_fixed_t,
    wl_fixed_t) {}

void WaylandClipboard::Connection::handle_drop(void*, wl_data_device*) {}

void WaylandClipboard::Connection::handle_selection(
    void* data,
    wl_data_device*,
    wl_data_offer* offer) {
  auto* connection = static_cast<Connection*>(data);
  if (offer == nullptr) {
    connection->clear_offer(connection->selection_offer_);
    return;
  }

  if (connection->pending_offer_ != nullptr &&
      connection->pending_offer_->offer == offer) {
    connection->clear_offer(connection->selection_offer_);
    connection->selection_offer_ = std::move(connection->pending_offer_);
  }
}
#endif

} // namespace cgpui
