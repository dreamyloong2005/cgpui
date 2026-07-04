#include "cgpui/ui/key_binding.hpp"

#include "key_binding_internal.hpp"

#include <cctype>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cgpui {
namespace {

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

[[nodiscard]] std::vector<std::string_view> split_key_sequence(
    std::string_view grammar) {
  std::vector<std::string_view> sequence;
  std::size_t index = 0;
  while (index < grammar.size()) {
    while (index < grammar.size() &&
           std::isspace(static_cast<unsigned char>(grammar[index])) != 0) {
      ++index;
    }
    if (index == grammar.size()) {
      break;
    }

    const std::size_t chord_start = index;
    while (index < grammar.size() &&
           std::isspace(static_cast<unsigned char>(grammar[index])) == 0) {
      ++index;
    }
    sequence.push_back(grammar.substr(chord_start, index - chord_start));
  }
  return sequence;
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
  const std::string lowered = lower_key_binding_token(token);
  if (lowered.size() == 1 &&
      std::isalnum(static_cast<unsigned char>(lowered.front())) != 0) {
    return static_cast<std::uint32_t>(
        std::toupper(static_cast<unsigned char>(lowered.front())));
  }
  return named_key_code(lowered);
}

[[nodiscard]] std::optional<KeyBindingChord> parse_key_binding_chord(
    std::string_view grammar,
    DesktopPlatformTarget platform) {
  const std::vector<std::string_view> tokens = split_key_tokens(grammar);
  if (tokens.empty()) {
    return std::nullopt;
  }

  KeyboardModifiers modifiers;
  for (std::size_t index = 0; index + 1 < tokens.size(); ++index) {
    if (!apply_key_binding_modifier_token(
            tokens[index],
            platform,
            modifiers)) {
      return std::nullopt;
    }
  }

  const std::optional<std::uint32_t> key_code = parse_key_token(tokens.back());
  if (!key_code.has_value()) {
    return std::nullopt;
  }

  return KeyBindingChord{
      .key_code = *key_code,
      .action = KeyAction::pressed,
      .modifiers = modifiers};
}

} // namespace

std::optional<KeyBinding> parse_key_binding(
    std::string_view grammar,
    std::string action_name,
    DesktopPlatformTarget platform) {
  if (grammar.empty() || action_name.empty()) {
    return std::nullopt;
  }

  const std::vector<std::string_view> chord_tokens =
      split_key_sequence(grammar);
  if (chord_tokens.empty()) {
    return std::nullopt;
  }

  std::vector<KeyBindingChord> sequence;
  sequence.reserve(chord_tokens.size());
  for (std::string_view chord_token : chord_tokens) {
    std::optional<KeyBindingChord> chord =
        parse_key_binding_chord(chord_token, platform);
    if (!chord.has_value()) {
      return std::nullopt;
    }
    sequence.push_back(*chord);
  }

  const KeyBindingChord first_chord = sequence.front();

  KeyBinding binding{
      .key_code = first_chord.key_code,
      .action = KeyAction::pressed,
      .modifiers = first_chord.modifiers,
      .action_name = std::move(action_name)};
  binding.sequence = std::move(sequence);
  return binding;
}

std::optional<KeyBinding> parse_key_binding(
    std::string_view grammar,
    std::string action_name) {
  return parse_key_binding(
      grammar,
      std::move(action_name),
      current_desktop_platform_target());
}

} // namespace cgpui
