#include "x11_clipboard_internal.hpp"

#include <poll.h>

namespace cgpui {

void X11Clipboard::event_loop() {
  pollfd descriptor{
      .fd = xcb_get_file_descriptor(connection_), .events = POLLIN, .revents = 0};
  while (!stopping_.load()) {
    const int ready = poll(&descriptor, 1, 50);
    if (ready <= 0 || (descriptor.revents & POLLIN) == 0) continue;
    while (xcb_generic_event_t* event = xcb_poll_for_event(connection_)) {
      handle_event(*event);
      std::free(event);
    }
  }
}

void X11Clipboard::handle_event(const xcb_generic_event_t& event) {
  switch (event.response_type & 0x7f) {
    case XCB_SELECTION_REQUEST:
      handle_selection_request(
          reinterpret_cast<const xcb_selection_request_event_t&>(event));
      break;
    case XCB_SELECTION_NOTIFY:
      handle_selection_notify(
          reinterpret_cast<const xcb_selection_notify_event_t&>(event));
      break;
    case XCB_SELECTION_CLEAR: {
      std::scoped_lock lock(state_mutex_);
      owned_text_.reset();
      owned_uri_list_.reset();
      break;
    }
    default:
      break;
  }
}

void X11Clipboard::handle_selection_request(
    const xcb_selection_request_event_t& request) {
  xcb_selection_notify_event_t notify{};
  notify.response_type = XCB_SELECTION_NOTIFY;
  notify.time = request.time;
  notify.requestor = request.requestor;
  notify.selection = request.selection;
  notify.target = request.target;
  notify.property = XCB_ATOM_NONE;
  const xcb_atom_t property =
      request.property == XCB_ATOM_NONE ? request.target : request.property;
  {
    std::scoped_lock lock(state_mutex_);
    if (request.target == atoms_.targets) {
      const xcb_atom_t targets[] = {
          atoms_.targets, atoms_.utf8_string, atoms_.text_plain_utf8,
          atoms_.uri_list, XCB_ATOM_STRING};
      xcb_change_property(
          connection_, XCB_PROP_MODE_REPLACE, request.requestor, property,
          XCB_ATOM_ATOM, 32, std::size(targets), targets);
      notify.property = property;
    } else {
      const std::string* payload = nullptr;
      if ((request.target == atoms_.utf8_string ||
           request.target == atoms_.text_plain_utf8 ||
           request.target == XCB_ATOM_STRING) && owned_text_) {
        payload = &*owned_text_;
      } else if (request.target == atoms_.uri_list && owned_uri_list_) {
        payload = &*owned_uri_list_;
      }
      if (payload != nullptr) {
        xcb_change_property(
            connection_, XCB_PROP_MODE_REPLACE, request.requestor, property,
            request.target, 8, static_cast<std::uint32_t>(payload->size()),
            payload->data());
        notify.property = property;
      }
    }
  }
  xcb_send_event(
      connection_, false, request.requestor, XCB_EVENT_MASK_NO_EVENT,
      reinterpret_cast<const char*>(&notify));
  xcb_flush(connection_);
}

void X11Clipboard::handle_selection_notify(
    const xcb_selection_notify_event_t& notify) {
  std::optional<std::string> payload;
  if (notify.property != XCB_ATOM_NONE) {
    constexpr std::uint32_t max_words = 4U * 1024U * 1024U;
    const auto cookie = xcb_get_property(
        connection_, true, window_, notify.property, XCB_GET_PROPERTY_TYPE_ANY,
        0, max_words);
    xcb_get_property_reply_t* reply =
        xcb_get_property_reply(connection_, cookie, nullptr);
    if (reply != nullptr && reply->format == 8 && reply->bytes_after == 0) {
      const auto* bytes = static_cast<const char*>(xcb_get_property_value(reply));
      payload = std::string(bytes, bytes + xcb_get_property_value_length(reply));
    }
    std::free(reply);
  }
  {
    std::scoped_lock lock(state_mutex_);
    response_ = std::move(payload);
    response_pending_ = false;
  }
  response_ready_.notify_all();
}

}  // namespace cgpui
