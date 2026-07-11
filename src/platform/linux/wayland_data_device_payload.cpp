#include "wayland_data_device_internal.hpp"
#include "wayland_uri_list_internal.hpp"

namespace cgpui {

std::optional<std::string> WaylandDataDevice::preferred_drag_mime_type()
    const {
  if (active_offer_has_mime("text/plain;charset=utf-8")) {
    return std::string{"text/plain;charset=utf-8"};
  }
  if (active_offer_has_mime("text/plain")) {
    return std::string{"text/plain"};
  }
  if (active_offer_has_mime("text/uri-list")) {
    return std::string{"text/uri-list"};
  }
  return std::nullopt;
}

std::optional<std::string> WaylandDataDevice::read_offer_payload(
    const std::string& mime_type) {
  if (active_offer_ == nullptr || active_offer_->offer == nullptr ||
      display_ == nullptr) {
    return std::nullopt;
  }

  int pipe_fds[2] = {-1, -1};
  if (pipe2(pipe_fds, O_CLOEXEC) == -1) {
    return std::nullopt;
  }

  wl_data_offer_receive(active_offer_->offer, mime_type.c_str(), pipe_fds[1]);
  if (wl_display_flush(display_) == -1) {
    close(pipe_fds[0]);
    close(pipe_fds[1]);
    return std::nullopt;
  }

  close(pipe_fds[1]);
  pipe_fds[1] = -1;

  std::string payload;
  const auto deadline =
      std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (std::chrono::steady_clock::now() < deadline) {
    pollfd descriptor{
        .fd = pipe_fds[0],
        .events = POLLIN | POLLHUP,
        .revents = 0,
    };
    const int ready = poll(&descriptor, 1, 50);
    if (ready == -1 && errno == EINTR) {
      continue;
    }
    if (ready <= 0) {
      continue;
    }

    char buffer[4096];
    const auto bytes_read = read(pipe_fds[0], buffer, sizeof(buffer));
    if (bytes_read > 0) {
      payload.append(buffer, static_cast<std::size_t>(bytes_read));
      continue;
    }
    close(pipe_fds[0]);
    return bytes_read == 0 ? std::optional<std::string>{std::move(payload)}
                           : std::nullopt;
  }

  close(pipe_fds[0]);
  return std::nullopt;
}

bool WaylandDataDevice::active_offer_has_mime(std::string_view mime_type)
    const {
  if (active_offer_ == nullptr) {
    return false;
  }
  return std::ranges::find_if(
             active_offer_->mime_types,
             [mime_type](const std::string& candidate) {
               return candidate == mime_type;
             }) != active_offer_->mime_types.end();
}

DragDropPayload WaylandDataDevice::payload_from_active_offer() {
  if (active_offer_has_mime("text/plain;charset=utf-8")) {
    if (auto text = read_offer_payload("text/plain;charset=utf-8");
        text.has_value()) {
      return DragDropPayload{
          .kind = DragDropPayloadKind::text,
          .text = std::move(*text),
      };
    }
  }
  if (active_offer_has_mime("text/plain")) {
    if (auto text = read_offer_payload("text/plain"); text.has_value()) {
      return DragDropPayload{
          .kind = DragDropPayloadKind::text,
          .text = std::move(*text),
      };
    }
  }
  if (active_offer_has_mime("text/uri-list")) {
    if (auto uri_list = read_offer_payload("text/uri-list");
        uri_list.has_value()) {
      auto files = parse_uri_list(*uri_list);
      if (!files.empty()) {
        return DragDropPayload{
            .kind = DragDropPayloadKind::files,
            .files = std::move(files),
        };
      }
    }
  }
  return {};
}

} // namespace cgpui
