#include "key_binding_internal.hpp"

#include <cctype>

namespace cgpui {
namespace {

[[nodiscard]] KeyboardModifiers platform_modifier_for(
    DesktopPlatformTarget platform) {
  switch (platform) {
    case DesktopPlatformTarget::windows:
    case DesktopPlatformTarget::linux_wayland:
    case DesktopPlatformTarget::macos_cocoa:
      return KeyboardModifiers{.super = true};
  }
  return KeyboardModifiers{};
}

[[nodiscard]] KeyboardModifiers secondary_modifier_for(
    DesktopPlatformTarget platform) {
  switch (platform) {
    case DesktopPlatformTarget::windows:
    case DesktopPlatformTarget::linux_wayland:
      return KeyboardModifiers{.control = true};
    case DesktopPlatformTarget::macos_cocoa:
      return KeyboardModifiers{.super = true};
  }
  return KeyboardModifiers{};
}

[[nodiscard]] bool merge_modifier(
    KeyboardModifiers modifier,
    KeyboardModifiers& modifiers) {
  if ((modifier.shift && modifiers.shift) ||
      (modifier.control && modifiers.control) ||
      (modifier.alt && modifiers.alt) ||
      (modifier.super && modifiers.super)) {
    return false;
  }
  modifiers.shift = modifiers.shift || modifier.shift;
  modifiers.control = modifiers.control || modifier.control;
  modifiers.alt = modifiers.alt || modifier.alt;
  modifiers.super = modifiers.super || modifier.super;
  return modifier.shift || modifier.control || modifier.alt || modifier.super;
}

} // namespace

std::string lower_key_binding_token(std::string_view value) {
  std::string lowered;
  lowered.reserve(value.size());
  for (const unsigned char ch : value) {
    lowered.push_back(static_cast<char>(std::tolower(ch)));
  }
  return lowered;
}

bool apply_key_binding_modifier_token(
    std::string_view token,
    DesktopPlatformTarget platform,
    KeyboardModifiers& modifiers) {
  const std::string lowered = lower_key_binding_token(token);
  if (lowered == "shift") {
    return merge_modifier(KeyboardModifiers{.shift = true}, modifiers);
  }
  if (lowered == "ctrl" || lowered == "control") {
    return merge_modifier(KeyboardModifiers{.control = true}, modifiers);
  }
  if (lowered == "alt" || lowered == "option") {
    return merge_modifier(KeyboardModifiers{.alt = true}, modifiers);
  }
  if (lowered == "cmd" || lowered == "command" || lowered == "super" ||
      lowered == "meta" || lowered == "platform" || lowered == "win" ||
      lowered == "windows") {
    return merge_modifier(platform_modifier_for(platform), modifiers);
  }
  if (lowered == "secondary") {
    return merge_modifier(secondary_modifier_for(platform), modifiers);
  }
  return false;
}

} // namespace cgpui
