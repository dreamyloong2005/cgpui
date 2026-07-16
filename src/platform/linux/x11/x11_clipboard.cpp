#include "x11_clipboard_internal.hpp"
#include "x11_data_transfer_internal.hpp"

namespace cgpui {
namespace {
std::optional<X11ClipboardAtoms> load_atoms(xcb_connection_t* connection) {
  X11ClipboardAtoms atoms;
  struct Binding {
    const char* name;
    xcb_atom_t X11ClipboardAtoms::*member;
  };
  constexpr Binding bindings[] = {
      {"CLIPBOARD", &X11ClipboardAtoms::clipboard},
      {"TARGETS", &X11ClipboardAtoms::targets},
      {"UTF8_STRING", &X11ClipboardAtoms::utf8_string},
      {"text/plain;charset=utf-8", &X11ClipboardAtoms::text_plain_utf8},
      {"text/uri-list", &X11ClipboardAtoms::uri_list},
      {"CGPUI_SELECTION", &X11ClipboardAtoms::property},
  };
  for (const Binding& binding : bindings) {
    auto atom = x11_intern_atom(connection, binding.name);
    if (!atom) return std::nullopt;
    atoms.*(binding.member) = *atom;
  }
  return atoms;
}
}  // namespace

std::unique_ptr<X11Clipboard> X11Clipboard::create() {
  auto clipboard = std::unique_ptr<X11Clipboard>(new X11Clipboard());
  return clipboard->initialize() ? std::move(clipboard) : nullptr;
}

bool X11Clipboard::initialize() {
  int screen_index = 0;
  connection_ = xcb_connect(nullptr, &screen_index);
  if (connection_ == nullptr || xcb_connection_has_error(connection_) != 0) {
    return false;
  }
  screen_ = x11_screen(connection_, screen_index);
  const auto atoms = load_atoms(connection_);
  if (screen_ == nullptr || !atoms) return false;
  atoms_ = *atoms;
  window_ = xcb_generate_id(connection_);
  const std::uint32_t mask = XCB_EVENT_MASK_PROPERTY_CHANGE;
  const auto cookie = xcb_create_window_checked(
      connection_, XCB_COPY_FROM_PARENT, window_, screen_->root, 0, 0, 1, 1, 0,
      XCB_WINDOW_CLASS_INPUT_OUTPUT, screen_->root_visual, XCB_CW_EVENT_MASK,
      &mask);
  xcb_generic_error_t* error = xcb_request_check(connection_, cookie);
  if (error != nullptr) {
    std::free(error);
    window_ = XCB_WINDOW_NONE;
    return false;
  }
  xcb_flush(connection_);
  event_thread_ = std::thread([this] { event_loop(); });
  return true;
}

X11Clipboard::~X11Clipboard() {
  stopping_.store(true);
  if (event_thread_.joinable()) event_thread_.join();
  if (connection_ != nullptr && window_ != XCB_WINDOW_NONE) {
    xcb_destroy_window(connection_, window_);
  }
  if (connection_ != nullptr) xcb_disconnect(connection_);
}

void X11Clipboard::own_selections() {
  xcb_set_selection_owner(
      connection_, window_, atoms_.clipboard, XCB_CURRENT_TIME);
  xcb_set_selection_owner(
      connection_, window_, XCB_ATOM_PRIMARY, XCB_CURRENT_TIME);
  xcb_flush(connection_);
}

bool X11Clipboard::write_text(std::string_view text) {
  {
    std::scoped_lock lock(state_mutex_);
    owned_text_ = std::string{text};
    owned_uri_list_.reset();
  }
  own_selections();
  return true;
}

bool X11Clipboard::write_files(std::span<const std::string> paths) {
  std::string payload = x11_encode_uri_list(paths);
  if (payload.empty() && !paths.empty()) return false;
  {
    std::scoped_lock lock(state_mutex_);
    owned_text_.reset();
    owned_uri_list_ = std::move(payload);
  }
  own_selections();
  return true;
}

std::optional<std::string> X11Clipboard::read_text() const {
  return read_payload(atoms_.utf8_string);
}

std::optional<std::vector<std::string>> X11Clipboard::read_files() const {
  auto payload = read_payload(atoms_.uri_list);
  if (!payload) return std::nullopt;
  return x11_parse_uri_list(*payload);
}

std::optional<std::string> X11Clipboard::read_payload(xcb_atom_t target) const {
  std::unique_lock request_lock(request_mutex_);
  {
    std::scoped_lock state_lock(state_mutex_);
    response_.reset();
    response_pending_ = true;
  }
  xcb_convert_selection(
      connection_, window_, atoms_.clipboard, target, atoms_.property,
      XCB_CURRENT_TIME);
  xcb_flush(connection_);
  std::unique_lock state_lock(state_mutex_);
  const bool ready = response_ready_.wait_for(
      state_lock, std::chrono::seconds(3),
      [this] { return !response_pending_ || stopping_.load(); });
  if (!ready || stopping_.load()) {
    response_pending_ = false;
    return std::nullopt;
  }
  return response_;
}

std::unique_ptr<Clipboard> create_x11_clipboard() {
  if (auto clipboard = X11Clipboard::create()) return clipboard;
  return std::make_unique<MemoryClipboard>();
}

}  // namespace cgpui
