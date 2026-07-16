#include "x11_window_internal.hpp"
#include "x11_data_transfer_internal.hpp"

namespace cgpui {
namespace {
std::vector<xcb_atom_t> offered_types(
    xcb_connection_t* connection,
    const X11Atoms& atoms,
    const xcb_client_message_event_t& message) {
  if ((message.data.data32[1] & 1U) == 0) {
    return {message.data.data32[2], message.data.data32[3],
            message.data.data32[4]};
  }
  const auto cookie = xcb_get_property(
      connection, false, message.data.data32[0], atoms.xdnd_type_list,
      XCB_ATOM_ATOM, 0, 256);
  xcb_get_property_reply_t* reply =
      xcb_get_property_reply(connection, cookie, nullptr);
  std::vector<xcb_atom_t> result;
  if (reply != nullptr && reply->format == 32) {
    const auto* values =
        static_cast<const xcb_atom_t*>(xcb_get_property_value(reply));
    result.assign(values, values + xcb_get_property_value_length(reply) / 4);
  }
  std::free(reply);
  return result;
}

xcb_atom_t preferred_type(
    const X11Atoms& atoms, const std::vector<xcb_atom_t>& offered) {
  for (xcb_atom_t candidate : offered) {
    if (candidate == atoms.uri_list) return candidate;
  }
  for (xcb_atom_t candidate : offered) {
    if (candidate == atoms.utf8_string ||
        candidate == atoms.text_plain_utf8) return candidate;
  }
  return XCB_ATOM_NONE;
}

DragDropAction drag_action(const X11Atoms& atoms, xcb_atom_t action) {
  if (action == atoms.xdnd_action_copy) return DragDropAction::copy;
  if (action == atoms.xdnd_action_move) return DragDropAction::move;
  return DragDropAction::none;
}
}  // namespace

bool X11Window::handle_drag_client_message(
    const xcb_client_message_event_t& message) {
  if (message.type == atoms_.xdnd_enter) {
    if (drag_.source != XCB_WINDOW_NONE && drag_.entered) {
      callback_(DragExited{.position = drag_.position});
    }
    reset_drag();
    drag_.source = message.data.data32[0];
    drag_.target = preferred_type(
        atoms_, offered_types(connection_, atoms_, message));
    if (drag_.target != XCB_ATOM_NONE) {
      xcb_convert_selection(
          connection_, window_, atoms_.xdnd_selection, drag_.target,
          atoms_.xdnd_transfer, XCB_CURRENT_TIME);
      xcb_flush(connection_);
    }
    return true;
  }
  if (message.type == atoms_.xdnd_position) {
    if (drag_.source != message.data.data32[0]) return true;
    const std::int16_t root_x =
        static_cast<std::int16_t>(message.data.data32[2] >> 16U);
    const std::int16_t root_y =
        static_cast<std::int16_t>(message.data.data32[2] & 0xffffU);
    const auto cookie = xcb_translate_coordinates(
        connection_, screen_->root, window_, root_x, root_y);
    xcb_translate_coordinates_reply_t* reply =
        xcb_translate_coordinates_reply(connection_, cookie, nullptr);
    drag_.position = reply == nullptr
        ? Point{static_cast<float>(root_x), static_cast<float>(root_y)}
        : Point{static_cast<float>(reply->dst_x),
                static_cast<float>(reply->dst_y)};
    std::free(reply);
    drag_.action = message.data.data32[4];
    send_drag_status();
    if (drag_.entered && drag_.payload_ready) {
      callback_(DragUpdated{
          .position = drag_.position,
          .payload = drag_.payload,
          .action = drag_action(atoms_, drag_.action)});
    }
    return true;
  }
  if (message.type == atoms_.xdnd_leave) {
    if (drag_.source == message.data.data32[0]) {
      callback_(DragExited{.position = drag_.position});
      reset_drag();
    }
    return true;
  }
  if (message.type == atoms_.xdnd_drop) {
    if (drag_.source != message.data.data32[0]) return true;
    if (!drag_.payload_ready) {
      drag_.drop_pending = true;
    } else {
      callback_(DragDropped{
          .position = drag_.position,
          .payload = drag_.payload,
          .action = drag_action(atoms_, drag_.action)});
      finish_drag(true);
    }
    return true;
  }
  return false;
}

void X11Window::handle_drag_selection_notify(
    const xcb_selection_notify_event_t& notify) {
  if (drag_.source == XCB_WINDOW_NONE ||
      notify.selection != atoms_.xdnd_selection ||
      notify.property == XCB_ATOM_NONE) {
    if (drag_.drop_pending) finish_drag(false);
    return;
  }
  const auto cookie = xcb_get_property(
      connection_, true, window_, notify.property, XCB_GET_PROPERTY_TYPE_ANY,
      0, 4U * 1024U * 1024U);
  xcb_get_property_reply_t* reply =
      xcb_get_property_reply(connection_, cookie, nullptr);
  if (reply == nullptr || reply->format != 8 || reply->bytes_after != 0) {
    std::free(reply);
    if (drag_.drop_pending) finish_drag(false);
    return;
  }
  const auto* bytes = static_cast<const char*>(xcb_get_property_value(reply));
  const std::string payload(
      bytes, bytes + xcb_get_property_value_length(reply));
  std::free(reply);
  if (drag_.target == atoms_.uri_list) {
    auto files = x11_parse_uri_list(payload);
    if (!files.empty()) {
      drag_.payload = DragDropPayload{
          .kind = DragDropPayloadKind::files, .files = std::move(files)};
    }
  } else if (drag_.target == atoms_.utf8_string ||
             drag_.target == atoms_.text_plain_utf8) {
    drag_.payload = DragDropPayload{
        .kind = DragDropPayloadKind::text, .text = payload};
  }
  drag_.payload_ready = drag_.payload.kind != DragDropPayloadKind::none;
  if (!drag_.payload_ready) {
    if (drag_.drop_pending) finish_drag(false);
    return;
  }
  if (!drag_.entered) {
    drag_.entered = true;
    callback_(DragEntered{
        .position = drag_.position,
        .payload = drag_.payload,
        .action = drag_action(atoms_, drag_.action)});
  }
  if (drag_.drop_pending) {
    callback_(DragDropped{
        .position = drag_.position,
        .payload = drag_.payload,
        .action = drag_action(atoms_, drag_.action)});
    finish_drag(true);
  }
}

void X11Window::send_drag_status() {
  xcb_client_message_event_t response{};
  response.response_type = XCB_CLIENT_MESSAGE;
  response.format = 32;
  response.window = drag_.source;
  response.type = atoms_.xdnd_status;
  response.data.data32[0] = window_;
  response.data.data32[1] = drag_.target == XCB_ATOM_NONE ? 0U : 1U;
  response.data.data32[4] = drag_.action;
  xcb_send_event(
      connection_, false, drag_.source, XCB_EVENT_MASK_NO_EVENT,
      reinterpret_cast<const char*>(&response));
  xcb_flush(connection_);
}

void X11Window::finish_drag(bool accepted) {
  xcb_client_message_event_t response{};
  response.response_type = XCB_CLIENT_MESSAGE;
  response.format = 32;
  response.window = drag_.source;
  response.type = atoms_.xdnd_finished;
  response.data.data32[0] = window_;
  response.data.data32[1] = accepted ? 1U : 0U;
  response.data.data32[2] = accepted ? drag_.action : XCB_ATOM_NONE;
  xcb_send_event(
      connection_, false, drag_.source, XCB_EVENT_MASK_NO_EVENT,
      reinterpret_cast<const char*>(&response));
  xcb_flush(connection_);
  reset_drag();
}

void X11Window::reset_drag() { drag_ = {}; }

}  // namespace cgpui
