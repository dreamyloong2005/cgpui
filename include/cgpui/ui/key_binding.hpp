#pragma once

#include "cgpui/core/event_keyboard.hpp"
#include "cgpui/platform/target.hpp"
#include "cgpui/ui/element_core.hpp"
#include "cgpui/ui/text_edit_actions.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace cgpui {

enum class KeyBindingContextKind {
  app,
  window,
  view,
  focused_element,
};

struct KeyBindingContext {
  KeyBindingContextKind kind = KeyBindingContextKind::app;
  std::optional<ViewId> view_id;
  std::optional<ElementId> element_id;

  [[nodiscard]] static KeyBindingContext app();
  [[nodiscard]] static KeyBindingContext window();
  [[nodiscard]] static KeyBindingContext view(ViewId view_id);
  [[nodiscard]] static KeyBindingContext focused_element(ElementId element_id);
};

struct KeyBinding {
  std::uint32_t key_code = 0;
  KeyAction action = KeyAction::pressed;
  KeyboardModifiers modifiers;
  std::string action_name;
  KeyBindingContext context;
};

struct TextEditBinding {
  std::uint32_t key_code = 0;
  KeyAction action = KeyAction::pressed;
  KeyboardModifiers modifiers;
  TextEditAction edit_action = TextEditAction::move_previous;
};

[[nodiscard]] std::optional<KeyBinding> parse_key_binding(
    std::string_view grammar,
    std::string action_name,
    DesktopPlatformTarget platform);

[[nodiscard]] std::optional<KeyBinding> parse_key_binding(
    std::string_view grammar,
    std::string action_name);

} // namespace cgpui
