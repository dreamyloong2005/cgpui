#include "clipboard_wayland_internal.hpp"

namespace cgpui {

#if defined(__linux__)
bool WaylandClipboard::Connection::write_text(std::string_view text) {
  if (support_ != WaylandClipboardSupport::available || display_ == nullptr ||
      manager_ == nullptr || data_device_ == nullptr) {
    return false;
  }

  {
    std::lock_guard display_lock(display_mutex_);
    auto* source = wl_data_device_manager_create_data_source(manager_);
    if (source == nullptr) {
      return false;
    }

    static const wl_data_source_listener source_listener{
        .target = &Connection::handle_source_target,
        .send = &Connection::handle_source_send,
        .cancelled = &Connection::handle_source_cancelled,
        .dnd_drop_performed = &Connection::handle_source_drop_performed,
        .dnd_finished = &Connection::handle_source_dnd_finished,
        .action = &Connection::handle_source_action,
    };
    wl_data_source_add_listener(source, &source_listener, this);
    wl_data_source_offer(source, "text/plain;charset=utf-8");
    wl_data_source_offer(source, "text/plain");

    {
      std::lock_guard selection_lock(owned_selection_mutex_);
      if (owned_source_ != nullptr) {
        wl_data_source_destroy(owned_source_);
      }
      owned_source_ = source;
      owned_text_ = std::string(text);
    }

    wl_data_device_set_selection(data_device_, source, 0);
    if (wl_display_flush(display_) == -1) {
      std::lock_guard selection_lock(owned_selection_mutex_);
      if (owned_source_ == source) {
        wl_data_source_destroy(owned_source_);
        owned_source_ = nullptr;
        owned_text_.clear();
      }
      return false;
    }
  }

  start_dispatch_thread();
  return true;
}

void WaylandClipboard::Connection::clear_owned_source() {
  std::lock_guard display_lock(display_mutex_);
  std::lock_guard selection_lock(owned_selection_mutex_);
  if (owned_source_ != nullptr) {
    wl_data_source_destroy(owned_source_);
    owned_source_ = nullptr;
  }
  owned_text_.clear();
}

#endif

} // namespace cgpui
