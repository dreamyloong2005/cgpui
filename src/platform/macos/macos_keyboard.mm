#include "macos_input_internal.hpp"
#include "macos_window_internal.hpp"

#include <cctype>

namespace cgpui {
namespace {

std::uint32_t special_key(std::uint16_t key_code) {
  switch (key_code) {
    case 36: return 0x0D;
    case 48: return 0x09;
    case 51: return 0x08;
    case 53: return 0x1B;
    case 115: return 0x24;
    case 116: return 0x21;
    case 117: return 0x2E;
    case 119: return 0x23;
    case 121: return 0x22;
    case 123: return 0x25;
    case 124: return 0x27;
    case 125: return 0x28;
    case 126: return 0x26;
    default: return key_code;
  }
}

std::uint32_t logical_key(std::uint16_t key_code, const std::string& text) {
  if (text.empty()) return special_key(key_code);
  const auto first = static_cast<unsigned char>(text.front());
  if (first < 0x80) return static_cast<std::uint32_t>(std::toupper(first));
  if ((first & 0xE0) == 0xC0 && text.size() >= 2) {
    return ((first & 0x1F) << 6) |
        (static_cast<unsigned char>(text[1]) & 0x3F);
  }
  if ((first & 0xF0) == 0xE0 && text.size() >= 3) {
    return ((first & 0x0F) << 12) |
        ((static_cast<unsigned char>(text[1]) & 0x3F) << 6) |
        (static_cast<unsigned char>(text[2]) & 0x3F);
  }
  if ((first & 0xF8) == 0xF0 && text.size() >= 4) {
    return ((first & 0x07) << 18) |
        ((static_cast<unsigned char>(text[1]) & 0x3F) << 12) |
        ((static_cast<unsigned char>(text[2]) & 0x3F) << 6) |
        (static_cast<unsigned char>(text[3]) & 0x3F);
  }
  return special_key(key_code);
}

}  // namespace

bool macos_modifier_key_pressed(std::uint16_t key_code, std::uint64_t flags) {
  const auto modifiers = static_cast<NSEventModifierFlags>(flags);
  switch (key_code) {
    case 54:
    case 55: return (modifiers & NSEventModifierFlagCommand) != 0;
    case 56:
    case 60: return (modifiers & NSEventModifierFlagShift) != 0;
    case 57: return (modifiers & NSEventModifierFlagCapsLock) != 0;
    case 58:
    case 61: return (modifiers & NSEventModifierFlagOption) != 0;
    case 59:
    case 62: return (modifiers & NSEventModifierFlagControl) != 0;
    default: return false;
  }
}

KeyboardKey macos_translate_key_event(
    std::uint16_t key_code,
    std::string text,
    std::uint64_t flags,
    bool pressed,
    bool repeat) {
  const auto modifiers = static_cast<NSEventModifierFlags>(flags);
  const bool command = (modifiers & NSEventModifierFlagCommand) != 0;
  return KeyboardKey{
      .key_code = logical_key(key_code, text),
      .action = pressed ? KeyAction::pressed : KeyAction::released,
      .modifiers = KeyboardModifiers{
          .shift = (modifiers & NSEventModifierFlagShift) != 0,
          .control = (modifiers & NSEventModifierFlagControl) != 0,
          .alt = (modifiers & NSEventModifierFlagOption) != 0,
          .super = command,
      },
      .scan_code = key_code,
      .repeat_count = static_cast<std::uint16_t>(repeat ? 2 : 1),
      .repeated = repeat,
      .system = command,
  };
}

void MacOSWindow::keyboard_key(NSEvent* event, bool pressed) {
  NSString* characters = [event charactersIgnoringModifiers];
  const char* utf8 = characters == nil ? nullptr : [characters UTF8String];
  callback_(macos_translate_key_event(
      [event keyCode],
      utf8 == nullptr ? std::string{} : std::string{utf8},
      [event modifierFlags],
      pressed,
      [event isARepeat]));
}

}  // namespace cgpui
