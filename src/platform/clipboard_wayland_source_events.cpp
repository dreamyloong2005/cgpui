#include "clipboard_wayland_internal.hpp"

namespace cgpui {

#if defined(__linux__)
namespace {

bool accepts_wayland_clipboard_source_mime_type(const char* mime_type) {
  if (mime_type == nullptr) {
    return false;
  }
  const std::string_view value(mime_type);
  return value == std::string_view{"text/plain;charset=utf-8"} ||
         value == std::string_view{"text/plain"};
}

} // namespace

bool WaylandClipboard::Connection::accepts_text_mime_type(
    const char* mime_type) {
  return accepts_wayland_clipboard_source_mime_type(mime_type);
}

void WaylandClipboard::Connection::handle_source_target(
    void*,
    wl_data_source*,
    const char*) {}

void WaylandClipboard::Connection::handle_source_send(
    void* data,
    wl_data_source* source,
    const char* mime_type,
    std::int32_t fd) {
  auto* connection = static_cast<Connection*>(data);
  if (connection == nullptr || fd < 0) {
    if (fd >= 0) {
      close(fd);
    }
    return;
  }

  std::string payload;
  {
    std::lock_guard lock(connection->owned_selection_mutex_);
    if (connection->owned_source_ != source ||
        !accepts_text_mime_type(mime_type)) {
      close(fd);
      return;
    }
    payload = connection->owned_text_;
  }

  connection->write_payload_to_fd(payload, fd);
}

void WaylandClipboard::Connection::handle_source_cancelled(
    void* data,
    wl_data_source* source) {
  auto* connection = static_cast<Connection*>(data);
  if (connection == nullptr || source == nullptr) {
    return;
  }

  bool should_destroy = false;
  {
    std::lock_guard lock(connection->owned_selection_mutex_);
    if (connection->owned_source_ == source) {
      connection->owned_source_ = nullptr;
      connection->owned_text_.clear();
      should_destroy = true;
    }
  }
  if (should_destroy) {
    connection->diagnostics_.record_ownership(false, 0);
    wl_data_source_destroy(source);
  }
}

void WaylandClipboard::Connection::handle_source_drop_performed(
    void*,
    wl_data_source*) {}

void WaylandClipboard::Connection::handle_source_dnd_finished(
    void*,
    wl_data_source*) {}

void WaylandClipboard::Connection::handle_source_action(
    void*,
    wl_data_source*,
    std::uint32_t) {}
#endif

} // namespace cgpui
