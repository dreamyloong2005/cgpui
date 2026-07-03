#include "clipboard_wayland_internal.hpp"

namespace cgpui {

#if defined(__linux__)
WaylandClipboard::WaylandClipboard() = default;

WaylandClipboard::WaylandClipboard(WaylandClipboardOptions options)
    : support_(
          !options.data_device_manager_available
              ? WaylandClipboardSupport::unsupported
              : (options.seat_available ? WaylandClipboardSupport::available
                                        : WaylandClipboardSupport::no_seat)) {
  if (options.connect_to_display) {
    connection_ = Connection::create(options.display_name);
    support_ = connection_ != nullptr ? connection_->support()
                                      : WaylandClipboardSupport::unsupported;
  }
}

WaylandClipboard::~WaylandClipboard() = default;

std::optional<std::string> WaylandClipboard::read_text() const {
  if (connection_ != nullptr) {
    if (auto text = connection_->read_text(); text.has_value()) {
      return text;
    }
    return fallback_.read_text();
  }
  return fallback_.read_text();
}

bool WaylandClipboard::write_text(std::string_view text) {
  const bool fallback_written = fallback_.write_text(text);
  if (connection_ != nullptr && connection_->write_text(text)) {
    return true;
  }
  return fallback_written;
}

WaylandClipboardSupport WaylandClipboard::support() const {
  return support_;
}

std::unique_ptr<Clipboard> create_wayland_clipboard() {
  return std::make_unique<WaylandClipboard>();
}
#endif

} // namespace cgpui
