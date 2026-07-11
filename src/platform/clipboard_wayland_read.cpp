#include "clipboard_wayland_internal.hpp"

namespace cgpui {

#if defined(__linux__)
std::optional<std::string> WaylandClipboard::Connection::read_text() {
  if (support_ != WaylandClipboardSupport::available || display_ == nullptr) {
    return std::nullopt;
  }

  stop_dispatch_thread();
  std::optional<std::string> result;
  {
    std::lock_guard display_lock(display_mutex_);
    if (wl_display_roundtrip(display_) != -1) {
      const auto mime_type = preferred_text_mime_type();
      if (mime_type.has_value()) {
        result = read_offer_payload(*mime_type);
      }
    }
  }

  bool owns_selection = false;
  {
    std::lock_guard selection_lock(owned_selection_mutex_);
    owns_selection = owned_source_ != nullptr;
  }
  if (owns_selection) start_dispatch_thread();
  return result;
}

std::optional<std::string>
WaylandClipboard::Connection::preferred_text_mime_type() const {
  if (selection_offer_ == nullptr) {
    return std::nullopt;
  }

  const auto& mime_types = selection_offer_->mime_types;
  const auto utf8 = std::ranges::find(
      mime_types,
      std::string_view{"text/plain;charset=utf-8"});
  if (utf8 != mime_types.end()) {
    return *utf8;
  }

  const auto plain =
      std::ranges::find(mime_types, std::string_view{"text/plain"});
  if (plain != mime_types.end()) {
    return *plain;
  }

  return std::nullopt;
}
#endif

} // namespace cgpui
