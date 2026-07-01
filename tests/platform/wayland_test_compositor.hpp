#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace cgpui::test {

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
  void request_pointer_move(std::int32_t x, std::int32_t y);
  void request_pointer_button(std::uint32_t button, bool pressed);
  void request_pointer_scroll(float delta_x, float delta_y);
  void request_drag_enter(std::int32_t x, std::int32_t y);
  void request_drag_motion(std::int32_t x, std::int32_t y);
  void request_drag_drop();
  void request_drag_leave();
  void request_keyboard_modifiers(
      bool shift,
      bool control,
      bool alt,
      bool super);
  void request_keyboard_key(std::uint32_t key, bool pressed);
  void request_keyboard_leave();

  [[nodiscard]] bool wait_for_close_sent() const;
  [[nodiscard]] bool wait_for_resize_configure_sent() const;
  [[nodiscard]] bool wait_for_resize_configure_acked() const;
  [[nodiscard]] bool wait_for_pointer_move_sent() const;
  [[nodiscard]] bool wait_for_pointer_button_sent() const;
  [[nodiscard]] bool wait_for_pointer_scroll_sent() const;
  [[nodiscard]] bool wait_for_drag_enter_sent() const;
  [[nodiscard]] bool wait_for_drag_motion_sent() const;
  [[nodiscard]] bool wait_for_drag_drop_sent() const;
  [[nodiscard]] bool wait_for_drag_leave_sent() const;
  [[nodiscard]] bool wait_for_pointer_cursor_set() const;
  [[nodiscard]] bool wait_for_pointer_cursor_set_count(
      std::uint32_t count) const;
  [[nodiscard]] bool wait_for_keyboard_modifiers_sent() const;
  [[nodiscard]] bool wait_for_keyboard_key_sent() const;
  [[nodiscard]] bool wait_for_keyboard_leave_sent() const;

 private:
  struct State;
  std::unique_ptr<State> state_;
};

} // namespace cgpui::test
