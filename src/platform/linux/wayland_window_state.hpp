#pragma once

#include "cgpui/platform/platform.hpp"
#include "cgpui/ui/text.hpp"

#include <cstdint>
#include <optional>
#include <string>

struct wl_array;

namespace cgpui {

class WaylandTextInputState {
 public:
  void set_available(bool available);
  [[nodiscard]] ImeTextInputSupport support() const;

  void set_placement(std::optional<ImeTextInputPlacement> placement);
  [[nodiscard]] std::optional<ImeTextInputPlacement> placement() const;

  void enter();
  std::optional<ImeComposition> leave(KeyboardModifiers modifiers);
  void set_surrounding_text(
      std::string surrounding_text,
      std::int32_t cursor,
      std::int32_t anchor);
  void set_content_type(std::uint32_t hint, std::uint32_t purpose);
  [[nodiscard]] ImeComposition preedit(
      std::string text,
      KeyboardModifiers modifiers);
  [[nodiscard]] ImeComposition commit(
      std::string text,
      KeyboardModifiers modifiers);

 private:
  bool available_ = false;
  bool entered_ = false;
  std::optional<ImeTextInputPlacement> placement_;
  std::string surrounding_text_;
  std::int32_t surrounding_text_cursor_ = 0;
  std::int32_t surrounding_text_anchor_ = 0;
  std::uint32_t content_type_hint_ = 0;
  std::uint32_t content_type_purpose_ = 0;
  std::string preedit_text_;
  std::string committed_text_;
};

struct WaylandXdgToplevelState {
  bool activated = false;
  bool maximized = false;
  bool fullscreen = false;
};

struct WaylandXdgConfigureState {
  std::int32_t pending_width = 0;
  std::int32_t pending_height = 0;
  std::uint32_t pending_serial = 0;
  std::uint32_t last_acked_configure_serial = 0;
  bool pending_size = false;
  bool configured = false;
  WaylandXdgToplevelState pending_toplevel_state;
  WaylandXdgToplevelState current_toplevel_state;
};

struct WaylandWindowChromeState {
  PlatformWindowChromeState platform;
  bool xdg_decoration_supported = false;
};

[[nodiscard]] std::string cursor_name_for_shape(CursorShape shape);
[[nodiscard]] WaylandXdgToplevelState parse_xdg_toplevel_states(
    const wl_array* states);

} // namespace cgpui
