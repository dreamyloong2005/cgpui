#pragma once

#include "cgpui/core/geometry.hpp"

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace cgpui {

enum class MouseButton {
  left,
  right,
  middle,
  other,
};

enum class KeyAction {
  pressed,
  released,
};

enum class CursorShape {
  default_arrow,
  pointing_hand,
  text,
  crosshair,
  resize_left_right,
  resize_up_down,
  not_allowed,
};

struct KeyboardModifiers {
  bool shift = false;
  bool control = false;
  bool alt = false;
  bool super = false;
};

struct WindowCloseRequested {};

struct WindowRedrawRequested {};

struct WindowResized {
  Size size;
  DpiScale scale;
};

struct WindowActivated {
  bool active = false;
};

struct WindowFocused {
  bool focused = false;
};

struct WindowMinimized {
  bool minimized = false;
};

struct WindowRestored {};

struct PointerMoved {
  Point position;
};

struct PointerButton {
  MouseButton button;
  bool pressed = false;
  Point position;
};

struct PointerScrolled {
  Point delta;
  Point position;
};

enum class DragDropPayloadKind {
  none,
  text,
  files,
};

struct DragDropPayload {
  DragDropPayloadKind kind = DragDropPayloadKind::none;
  std::string text;
  std::vector<std::string> files;
};

struct DragEntered {
  Point position;
  DragDropPayload payload;
};

struct DragUpdated {
  Point position;
  DragDropPayload payload;
};

struct DragDropped {
  Point position;
  DragDropPayload payload;
};

struct DragExited {
  Point position;
  DragDropPayload payload;
};

struct KeyboardKey {
  std::uint32_t key_code = 0;
  KeyAction action = KeyAction::pressed;
  KeyboardModifiers modifiers;
};

struct TextInput {
  std::string text;
  KeyboardModifiers modifiers;
};

enum class ImeCompositionPhase {
  update,
  commit,
  cancel,
};

struct ImeComposition {
  ImeCompositionPhase phase = ImeCompositionPhase::update;
  std::string text;
  KeyboardModifiers modifiers;
};

using PlatformEvent = std::variant<
    WindowCloseRequested,
    WindowRedrawRequested,
    WindowResized,
    WindowActivated,
    WindowFocused,
    WindowMinimized,
    WindowRestored,
    PointerMoved,
    PointerButton,
    PointerScrolled,
    DragEntered,
    DragUpdated,
    DragDropped,
    DragExited,
    KeyboardKey,
    TextInput,
    ImeComposition>;

} // namespace cgpui
