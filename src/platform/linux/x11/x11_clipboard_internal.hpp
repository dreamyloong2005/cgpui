#pragma once

#include "../../clipboard_internal.hpp"
#include "x11_internal.hpp"

#include <condition_variable>
#include <thread>

namespace cgpui {

struct X11ClipboardAtoms {
  xcb_atom_t clipboard = XCB_ATOM_NONE;
  xcb_atom_t targets = XCB_ATOM_NONE;
  xcb_atom_t utf8_string = XCB_ATOM_NONE;
  xcb_atom_t text_plain_utf8 = XCB_ATOM_NONE;
  xcb_atom_t uri_list = XCB_ATOM_NONE;
  xcb_atom_t property = XCB_ATOM_NONE;
};

class X11Clipboard final : public Clipboard {
 public:
  static std::unique_ptr<X11Clipboard> create();
  ~X11Clipboard() override;

  [[nodiscard]] std::optional<std::string> read_text() const override;
  [[nodiscard]] bool write_text(std::string_view text) override;
  [[nodiscard]] std::optional<std::vector<std::string>>
  read_files() const override;
  [[nodiscard]] bool write_files(
      std::span<const std::string> paths) override;

 private:
  X11Clipboard() = default;
  [[nodiscard]] bool initialize();
  [[nodiscard]] std::optional<std::string> read_payload(
      xcb_atom_t target) const;
  void own_selections();
  void event_loop();
  void handle_event(const xcb_generic_event_t& event);
  void handle_selection_request(const xcb_selection_request_event_t& request);
  void handle_selection_notify(const xcb_selection_notify_event_t& notify);

  xcb_connection_t* connection_ = nullptr;
  xcb_screen_t* screen_ = nullptr;
  xcb_window_t window_ = XCB_WINDOW_NONE;
  X11ClipboardAtoms atoms_;
  mutable std::mutex request_mutex_;
  mutable std::mutex state_mutex_;
  mutable std::condition_variable response_ready_;
  mutable bool response_pending_ = false;
  mutable std::optional<std::string> response_;
  std::optional<std::string> owned_text_;
  std::optional<std::string> owned_uri_list_;
  std::thread event_thread_;
  std::atomic_bool stopping_{false};
};

}  // namespace cgpui
