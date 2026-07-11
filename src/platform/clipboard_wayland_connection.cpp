#include "clipboard_wayland_internal.hpp"

namespace cgpui {

#if defined(__linux__)
std::unique_ptr<WaylandClipboard::Connection>
WaylandClipboard::Connection::create(std::string_view display_name) {
  auto connection = std::unique_ptr<Connection>(new Connection(display_name));
  connection->initialize();
  if (connection->support_ == WaylandClipboardSupport::unsupported) {
    return nullptr;
  }
  return connection;
}

WaylandClipboard::Connection::Connection(std::string_view display_name)
    : display_name_(display_name) {}

WaylandClipboard::Connection::~Connection() {
  stop_dispatch_thread();
  clear_owned_source();
  clear_offer(selection_offer_);
  clear_offer(pending_offer_);
  if (data_device_ != nullptr) {
    wl_data_device_destroy(data_device_);
  }
  if (manager_ != nullptr) {
    wl_data_device_manager_destroy(manager_);
  }
  if (seat_ != nullptr) {
    wl_seat_destroy(seat_);
  }
  if (registry_ != nullptr) {
    wl_registry_destroy(registry_);
  }
  if (display_ != nullptr) {
    wl_display_disconnect(display_);
  }
}

WaylandClipboardSupport WaylandClipboard::Connection::support() const {
  return support_;
}

WaylandClipboardDiagnostics WaylandClipboard::Connection::diagnostics() const {
  return diagnostics_.snapshot();
}

void WaylandClipboard::Connection::record_diagnostics(
    WaylandClipboardOperation operation,
    WaylandClipboardFailure failure,
    std::size_t bytes_transferred) {
  diagnostics_.record(operation, failure, bytes_transferred);
}

void WaylandClipboard::Connection::initialize() {
  display_ =
      wl_display_connect(display_name_.empty() ? nullptr : display_name_.c_str());
  if (display_ == nullptr) {
    support_ = WaylandClipboardSupport::unsupported;
    return;
  }

  registry_ = wl_display_get_registry(display_);
  if (registry_ == nullptr) {
    support_ = WaylandClipboardSupport::unsupported;
    return;
  }

  static const wl_registry_listener registry_listener{
      .global = &Connection::handle_global,
      .global_remove = &Connection::handle_global_remove,
  };
  wl_registry_add_listener(registry_, &registry_listener, this);
  if (wl_display_roundtrip(display_) == -1) {
    support_ = WaylandClipboardSupport::unsupported;
    return;
  }

  if (manager_ == nullptr) {
    support_ = WaylandClipboardSupport::unsupported;
    return;
  }
  if (seat_ == nullptr) {
    support_ = WaylandClipboardSupport::no_seat;
    return;
  }

  data_device_ = wl_data_device_manager_get_data_device(manager_, seat_);
  if (data_device_ == nullptr) {
    support_ = WaylandClipboardSupport::unsupported;
    return;
  }

  static const wl_data_device_listener data_device_listener{
      .data_offer = &Connection::handle_data_offer,
      .enter = &Connection::handle_enter,
      .leave = &Connection::handle_leave,
      .motion = &Connection::handle_motion,
      .drop = &Connection::handle_drop,
      .selection = &Connection::handle_selection,
  };
  wl_data_device_add_listener(data_device_, &data_device_listener, this);
  support_ = WaylandClipboardSupport::available;
  (void)wl_display_roundtrip(display_);
}

void WaylandClipboard::Connection::handle_global(
    void* data,
    wl_registry* registry,
    std::uint32_t name,
    const char* interface,
    std::uint32_t version) {
  auto* connection = static_cast<Connection*>(data);
  const std::string_view interface_name(interface);
  if (interface_name == wl_data_device_manager_interface.name) {
    connection->manager_ =
        static_cast<wl_data_device_manager*>(wl_registry_bind(
            registry,
            name,
            &wl_data_device_manager_interface,
            std::min<std::uint32_t>(version, 3)));
    return;
  }
  if (interface_name == wl_seat_interface.name) {
    connection->seat_ = static_cast<wl_seat*>(wl_registry_bind(
        registry,
        name,
        &wl_seat_interface,
        std::min<std::uint32_t>(version, 5)));
  }
}

void WaylandClipboard::Connection::handle_global_remove(
    void*,
    wl_registry*,
    std::uint32_t) {}

void WaylandClipboard::Connection::clear_offer(std::unique_ptr<Offer>& offer) {
  if (offer != nullptr && offer->offer != nullptr) {
    wl_data_offer_destroy(offer->offer);
  }
  offer.reset();
}
#endif

} // namespace cgpui
