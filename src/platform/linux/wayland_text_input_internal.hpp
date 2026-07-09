#pragma once

#include "wayland_internal.hpp"

namespace cgpui {

class WaylandTextInput {
 public:
  using WindowLookup = std::function<WaylandWindow*(wl_surface*)>;
  using ModifiersProvider = std::function<KeyboardModifiers()>;

  void set_display(wl_display* display);
  void set_manager(zwp_text_input_manager_v3* manager);
  void set_window_lookup(WindowLookup lookup);
  void set_modifiers_provider(ModifiersProvider provider);
  void bind_to_seat(wl_seat* seat);
  void reset_text_input();
  [[nodiscard]] bool available() const;
  void apply_placement(WaylandWindow& window);

 private:
  struct PendingPreedit {
    std::string text;
    std::int32_t cursor_begin = 0;
    std::int32_t cursor_end = 0;
  };

  struct PendingDeleteSurroundingText {
    std::uint32_t before_length = 0;
    std::uint32_t after_length = 0;
  };

  static void handle_enter(
      void* data,
      zwp_text_input_v3* text_input,
      wl_surface* surface);
  static void handle_leave(
      void* data,
      zwp_text_input_v3* text_input,
      wl_surface* surface);
  static void handle_preedit_string(
      void* data,
      zwp_text_input_v3* text_input,
      const char* text,
      std::int32_t cursor_begin,
      std::int32_t cursor_end);
  static void handle_commit_string(
      void* data,
      zwp_text_input_v3* text_input,
      const char* text);
  static void handle_delete_surrounding_text(
      void* data,
      zwp_text_input_v3* text_input,
      std::uint32_t before_length,
      std::uint32_t after_length);
  static void handle_done(
      void* data,
      zwp_text_input_v3* text_input,
      std::uint32_t serial);

  [[nodiscard]] KeyboardModifiers current_modifiers() const;

  wl_display* display_ = nullptr;
  zwp_text_input_manager_v3* manager_ = nullptr;
  zwp_text_input_v3* text_input_ = nullptr;
  WaylandWindow* active_window_ = nullptr;
  WindowLookup find_window_;
  ModifiersProvider modifiers_;
  std::optional<PendingPreedit> pending_preedit_;
  std::optional<std::string> pending_commit_;
  std::optional<PendingDeleteSurroundingText> pending_delete_surrounding_;
};

} // namespace cgpui
