#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <optional>
#include <vector>

namespace cgpui::test {

struct WaylandMimePayload {
  std::string mime_type;
  std::string payload;
};

using WaylandClipboardMimePayload = WaylandMimePayload;

struct WaylandTextInputRect {
  std::int32_t x = 0;
  std::int32_t y = 0;
  std::int32_t width = 0;
  std::int32_t height = 0;
};

struct WaylandTextInputClientState {
  std::string surrounding_text;
  std::int32_t cursor = 0;
  std::int32_t anchor = 0;
  std::uint32_t content_hint = 0;
  std::uint32_t content_purpose = 0;
  std::optional<WaylandTextInputRect> cursor_rect;
  bool enabled = false;
};

struct WaylandPointerAxisFrame {
  float delta_x = 0.0F;
  float delta_y = 0.0F;
  std::int32_t source = -1;
  std::int32_t value120_x = 0;
  std::int32_t value120_y = 0;
  std::int32_t discrete_x = 0;
  std::int32_t discrete_y = 0;
  bool stop_x = false;
  bool stop_y = false;
};

struct WaylandConfigureState {
  std::int32_t width = 0;
  std::int32_t height = 0;
  std::uint32_t serial = 0;
  std::uint32_t acked_serial = 0;
  bool activated = false;
  bool maximized = false;
  bool fullscreen = false;
  bool acked = false;
};

class WaylandTestCompositor {
 public:
  explicit WaylandTestCompositor(std::string name);
  ~WaylandTestCompositor();

  WaylandTestCompositor(const WaylandTestCompositor&) = delete;
  WaylandTestCompositor& operator=(const WaylandTestCompositor&) = delete;

  [[nodiscard]] bool start();
  void stop();

  [[nodiscard]] const std::string& socket_name() const;

  void set_close_on_initial_configure_ack(bool enabled);
  void request_close();
  void request_resize_configure(std::int32_t width, std::int32_t height);
  void request_resize_configure_state(
      std::int32_t width,
      std::int32_t height,
      bool activated,
      bool maximized,
      bool fullscreen);
  void request_output_scale(std::int32_t scale);
  void request_fractional_scale(std::uint32_t scale);
  void request_seat_capabilities(std::uint32_t capabilities);
  void request_pointer_enter(std::int32_t x, std::int32_t y);
  void request_pointer_move(std::int32_t x, std::int32_t y);
  void request_pointer_leave();
  void request_pointer_button(std::uint32_t button, bool pressed);
  void request_pointer_scroll(float delta_x, float delta_y);
  void request_pointer_axis_frame(WaylandPointerAxisFrame frame);
  void request_drag_enter(std::int32_t x, std::int32_t y);
  void request_drag_motion(std::int32_t x, std::int32_t y);
  void request_drag_drop();
  void request_drag_leave();
  void set_drag_payloads(std::vector<WaylandMimePayload> payloads);
  void set_drag_source_actions(
      std::uint32_t source_actions,
      std::uint32_t selected_action);
  void request_keyboard_modifiers(
      bool shift,
      bool control,
      bool alt,
      bool super);
  void request_keyboard_modifier_masks(
      std::uint32_t depressed,
      std::uint32_t latched,
      std::uint32_t locked,
      std::uint32_t group);
  void request_keyboard_keymap_reload();
  void request_keyboard_key(std::uint32_t key, bool pressed);
  void request_keyboard_leave();
  void request_text_input_enter();
  void request_text_input_preedit(std::string text);
  void request_text_input_preedit(
      std::string text,
      std::int32_t cursor_begin,
      std::int32_t cursor_end);
  void request_text_input_preedit_with_serial(
      std::string text,
      std::int32_t cursor_begin,
      std::int32_t cursor_end,
      std::uint32_t serial);
  void request_text_input_delete_surrounding(
      std::uint32_t before_length,
      std::uint32_t after_length);
  void request_text_input_commit(std::string text);
  void request_text_input_commit_with_serial(
      std::string text,
      std::uint32_t serial);
  void request_text_input_leave();
  void set_clipboard_selection(
      std::vector<WaylandMimePayload> payloads);
  void request_clipboard_client_selection(std::string_view mime_type);

  [[nodiscard]] bool wait_for_close_sent() const;
  [[nodiscard]] bool wait_for_minimize_requested() const;
  [[nodiscard]] bool wait_for_maximize_requested() const;
  [[nodiscard]] bool wait_for_unmaximize_requested() const;
  [[nodiscard]] bool wait_for_fullscreen_requested() const;
  [[nodiscard]] bool wait_for_unfullscreen_requested() const;
  [[nodiscard]] bool wait_for_client_side_decoration_requested() const;
  [[nodiscard]] bool wait_for_server_side_decoration_requested() const;
  [[nodiscard]] bool wait_for_parent_requested() const;
  [[nodiscard]] bool wait_for_resize_configure_sent() const;
  [[nodiscard]] bool wait_for_resize_configure_acked() const;
  [[nodiscard]] WaylandConfigureState last_resize_configure_state() const;
  [[nodiscard]] bool wait_for_output_scale_sent() const;
  [[nodiscard]] std::int32_t last_surface_buffer_scale() const;
  [[nodiscard]] bool wait_for_fractional_scale_sent() const;
  [[nodiscard]] WaylandTextInputRect last_viewport_destination() const;
  [[nodiscard]] bool wait_for_pointer_bound(bool bound) const;
  [[nodiscard]] bool wait_for_keyboard_bound(bool bound) const;
  [[nodiscard]] bool wait_for_pointer_enter_sent() const;
  [[nodiscard]] bool wait_for_pointer_move_sent() const;
  [[nodiscard]] bool wait_for_pointer_leave_sent() const;
  [[nodiscard]] bool wait_for_pointer_button_sent() const;
  [[nodiscard]] bool wait_for_pointer_scroll_sent() const;
  [[nodiscard]] bool wait_for_drag_enter_sent() const;
  [[nodiscard]] bool wait_for_drag_motion_sent() const;
  [[nodiscard]] bool wait_for_drag_drop_sent() const;
  [[nodiscard]] bool wait_for_drag_leave_sent() const;
  [[nodiscard]] bool wait_for_drag_offer_accepted() const;
  [[nodiscard]] bool wait_for_drag_offer_actions_set() const;
  [[nodiscard]] bool wait_for_drag_offer_finished() const;
  [[nodiscard]] std::string last_drag_accept_mime_type() const;
  [[nodiscard]] std::uint32_t last_drag_offer_actions() const;
  [[nodiscard]] std::uint32_t last_drag_preferred_action() const;
  [[nodiscard]] bool wait_for_pointer_cursor_set() const;
  [[nodiscard]] bool wait_for_pointer_cursor_set_count(
      std::uint32_t count) const;
  [[nodiscard]] bool wait_for_keyboard_modifiers_sent() const;
  [[nodiscard]] bool wait_for_keyboard_keymap_sent_count(
      std::uint32_t count) const;
  [[nodiscard]] bool wait_for_keyboard_key_sent() const;
  [[nodiscard]] bool wait_for_keyboard_leave_sent() const;
  [[nodiscard]] bool wait_for_text_input_enter_sent() const;
  [[nodiscard]] bool wait_for_text_input_preedit_sent() const;
  [[nodiscard]] bool wait_for_text_input_delete_surrounding_sent() const;
  [[nodiscard]] bool wait_for_text_input_commit_sent() const;
  [[nodiscard]] bool wait_for_text_input_leave_sent() const;
  [[nodiscard]] bool wait_for_text_input_client_state_committed() const;
  [[nodiscard]] WaylandTextInputClientState text_input_client_state() const;
  [[nodiscard]] bool wait_for_clipboard_selection_sent() const;
  [[nodiscard]] std::string last_clipboard_receive_mime_type() const;
  [[nodiscard]] bool wait_for_clipboard_client_selection_set() const;
  [[nodiscard]] std::vector<std::string> clipboard_client_selection_mime_types()
      const;
  [[nodiscard]] bool wait_for_clipboard_client_selection_payload_received()
      const;
  [[nodiscard]] std::string last_clipboard_client_selection_payload() const;

 private:
  struct State;
  std::unique_ptr<State> state_;
};

} // namespace cgpui::test
