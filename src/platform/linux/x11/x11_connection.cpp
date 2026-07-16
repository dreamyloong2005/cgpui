#include "x11_internal.hpp"

namespace cgpui {

Error x11_error(ErrorCode code, std::string message) {
  return Error{.code = code, .message = std::move(message)};
}

xcb_screen_t* x11_screen(xcb_connection_t* connection, int screen_index) {
  const xcb_setup_t* setup = xcb_get_setup(connection);
  if (setup == nullptr) return nullptr;
  xcb_screen_iterator_t screens = xcb_setup_roots_iterator(setup);
  while (screen_index > 0 && screens.rem != 0) {
    xcb_screen_next(&screens);
    --screen_index;
  }
  return screens.rem == 0 ? nullptr : screens.data;
}

Result<xcb_atom_t> x11_intern_atom(
    xcb_connection_t* connection,
    std::string_view name) {
  const auto cookie = xcb_intern_atom(
      connection, false, static_cast<std::uint16_t>(name.size()), name.data());
  xcb_intern_atom_reply_t* reply =
      xcb_intern_atom_reply(connection, cookie, nullptr);
  if (reply == nullptr) {
    return std::unexpected(x11_error(
        ErrorCode::platform_initialization_failed,
        "xcb_intern_atom failed for " + std::string{name}));
  }
  const xcb_atom_t atom = reply->atom;
  std::free(reply);
  return atom;
}

Result<X11Atoms> x11_load_atoms(xcb_connection_t* connection) {
  X11Atoms atoms;
  struct Binding {
    const char* name;
    xcb_atom_t X11Atoms::*member;
  };
  constexpr Binding bindings[] = {
      {"WM_PROTOCOLS", &X11Atoms::wm_protocols},
      {"WM_DELETE_WINDOW", &X11Atoms::wm_delete_window},
      {"UTF8_STRING", &X11Atoms::utf8_string},
      {"_NET_WM_NAME", &X11Atoms::net_wm_name},
      {"_NET_WM_STATE", &X11Atoms::net_wm_state},
      {"_NET_WM_STATE_FULLSCREEN", &X11Atoms::net_wm_state_fullscreen},
      {"_NET_WM_STATE_MAXIMIZED_HORZ", &X11Atoms::net_wm_state_maximized_horz},
      {"_NET_WM_STATE_MAXIMIZED_VERT", &X11Atoms::net_wm_state_maximized_vert},
      {"_MOTIF_WM_HINTS", &X11Atoms::motif_wm_hints},
      {"XdndAware", &X11Atoms::xdnd_aware},
      {"XdndEnter", &X11Atoms::xdnd_enter},
      {"XdndPosition", &X11Atoms::xdnd_position},
      {"XdndStatus", &X11Atoms::xdnd_status},
      {"XdndLeave", &X11Atoms::xdnd_leave},
      {"XdndDrop", &X11Atoms::xdnd_drop},
      {"XdndFinished", &X11Atoms::xdnd_finished},
      {"XdndSelection", &X11Atoms::xdnd_selection},
      {"XdndTypeList", &X11Atoms::xdnd_type_list},
      {"XdndActionCopy", &X11Atoms::xdnd_action_copy},
      {"XdndActionMove", &X11Atoms::xdnd_action_move},
      {"CGPUI_XDND_TRANSFER", &X11Atoms::xdnd_transfer},
      {"text/plain;charset=utf-8", &X11Atoms::text_plain_utf8},
      {"text/uri-list", &X11Atoms::uri_list},
  };
  for (const Binding& binding : bindings) {
    auto atom = x11_intern_atom(connection, binding.name);
    if (!atom) return std::unexpected(atom.error());
    atoms.*(binding.member) = *atom;
  }
  return atoms;
}

}  // namespace cgpui
