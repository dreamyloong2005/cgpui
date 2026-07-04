#include "cgpui/ui/key_binding.hpp"

#include <cctype>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cgpui {
namespace {

[[nodiscard]] std::string lower_ascii(std::string_view value) {
  std::string lowered;
  lowered.reserve(value.size());
  for (const unsigned char ch : value) {
    lowered.push_back(static_cast<char>(std::tolower(ch)));
  }
  return lowered;
}

[[nodiscard]] std::vector<std::string_view> split_key_tokens(
    std::string_view grammar) {
  std::vector<std::string_view> tokens;
  std::size_t token_start = 0;
  for (std::size_t index = 0; index <= grammar.size(); ++index) {
    if (index == grammar.size() || grammar[index] == '-' ||
        grammar[index] == '+') {
      if (index == token_start) {
        return {};
      }
      tokens.push_back(grammar.substr(token_start, index - token_start));
      token_start = index + 1;
    }
  }
  return tokens;
}

[[nodiscard]] bool apply_modifier_token(
    std::string_view token,
    KeyboardModifiers& modifiers) {
  const std::string lowered = lower_ascii(token);
  if (lowered == "shift") {
    const bool fresh = !modifiers.shift;
    modifiers.shift = true;
    return fresh;
  }
  if (lowered == "ctrl" || lowered == "control") {
    const bool fresh = !modifiers.control;
    modifiers.control = true;
    return fresh;
  }
  if (lowered == "alt" || lowered == "option") {
    const bool fresh = !modifiers.alt;
    modifiers.alt = true;
    return fresh;
  }
  if (lowered == "cmd" || lowered == "command" || lowered == "super" ||
      lowered == "meta") {
    const bool fresh = !modifiers.super;
    modifiers.super = true;
    return fresh;
  }
  return false;
}

[[nodiscard]] std::optional<std::uint32_t> named_key_code(
    const std::string& lowered) {
  if (lowered == "enter" || lowered == "return") {
    return 13;
  }
  if (lowered == "tab") {
    return 9;
  }
  if (lowered == "escape" || lowered == "esc") {
    return 27;
  }
  if (lowered == "space") {
    return 32;
  }
  if (lowered == "backspace") {
    return 8;
  }
  if (lowered == "delete" || lowered == "del") {
    return 46;
  }
  if (lowered == "left") {
    return 37;
  }
  if (lowered == "up") {
    return 38;
  }
  if (lowered == "right") {
    return 39;
  }
  if (lowered == "down") {
    return 40;
  }
  return std::nullopt;
}

[[nodiscard]] std::optional<std::uint32_t> parse_key_token(
    std::string_view token) {
  const std::string lowered = lower_ascii(token);
  if (lowered.size() == 1 &&
      std::isalnum(static_cast<unsigned char>(lowered.front())) != 0) {
    return static_cast<std::uint32_t>(
        std::toupper(static_cast<unsigned char>(lowered.front())));
  }
  return named_key_code(lowered);
}

} // namespace

std::optional<KeyBinding> parse_key_binding(
    std::string_view grammar,
    std::string action_name) {
  if (grammar.empty() || action_name.empty()) {
    return std::nullopt;
  }

  const std::vector<std::string_view> tokens = split_key_tokens(grammar);
  if (tokens.empty()) {
    return std::nullopt;
  }

  KeyboardModifiers modifiers;
  for (std::size_t index = 0; index + 1 < tokens.size(); ++index) {
    if (!apply_modifier_token(tokens[index], modifiers)) {
      return std::nullopt;
    }
  }

  const std::optional<std::uint32_t> key_code = parse_key_token(tokens.back());
  if (!key_code.has_value()) {
    return std::nullopt;
  }

  return KeyBinding{
      .key_code = *key_code,
      .action = KeyAction::pressed,
      .modifiers = modifiers,
      .action_name = std::move(action_name)};
}

} // namespace cgpui
