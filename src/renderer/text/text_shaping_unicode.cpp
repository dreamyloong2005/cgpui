#include "text_shaping_native_internal.hpp"

namespace cgpui {

char32_t text_shaping_decode_utf8(
    std::string_view text,
    std::size_t byte_offset,
    std::size_t* byte_length) {
  std::size_t length = 1;
  if (byte_offset >= text.size()) {
    if (byte_length != nullptr) {
      *byte_length = 0;
    }
    return U'\0';
  }
  while (byte_offset + length < text.size() &&
         is_utf8_continuation_byte(text[byte_offset + length]) && length < 4) {
    length += 1;
  }
  if (byte_length != nullptr) {
    *byte_length = length;
  }
  const auto byte = [&](std::size_t index) {
    return static_cast<char32_t>(
        static_cast<unsigned char>(text[byte_offset + index]));
  };
  const char32_t first = byte(0);
  if (length == 1 || first < 0x80U) {
    return first;
  }
  if (length == 2) {
    return ((first & 0x1FU) << 6U) | (byte(1) & 0x3FU);
  }
  if (length == 3) {
    return ((first & 0x0FU) << 12U) | ((byte(1) & 0x3FU) << 6U) |
        (byte(2) & 0x3FU);
  }
  return ((first & 0x07U) << 18U) | ((byte(1) & 0x3FU) << 12U) |
      ((byte(2) & 0x3FU) << 6U) | (byte(3) & 0x3FU);
}

TextShapingScript text_shaping_script_for_codepoint(char32_t codepoint) {
  if (0x1F000U <= codepoint && codepoint <= 0x1FAFFU) {
    return TextShapingScript::emoji;
  }
  if ((U'A' <= codepoint && codepoint <= U'Z') ||
      (U'a' <= codepoint && codepoint <= U'z') ||
      (0x00C0U <= codepoint && codepoint <= 0x024FU &&
       codepoint != 0x00D7U && codepoint != 0x00F7U)) {
    return TextShapingScript::latin;
  }
  if ((0x0300U <= codepoint && codepoint <= 0x036FU) ||
      (0xFE00U <= codepoint && codepoint <= 0xFE0FU) || codepoint == 0x200DU) {
    return TextShapingScript::inherited;
  }
  if ((0x4E00U <= codepoint && codepoint <= 0x9FFFU) ||
      (0x3400U <= codepoint && codepoint <= 0x4DBFU)) {
    return TextShapingScript::han;
  }
  if (0x3040U <= codepoint && codepoint <= 0x309FU) {
    return TextShapingScript::hiragana;
  }
  if (0x30A0U <= codepoint && codepoint <= 0x30FFU) {
    return TextShapingScript::katakana;
  }
  if (0xAC00U <= codepoint && codepoint <= 0xD7AFU) {
    return TextShapingScript::hangul;
  }
  if (0x0590U <= codepoint && codepoint <= 0x05FFU) {
    return TextShapingScript::hebrew;
  }
  if (0x0600U <= codepoint && codepoint <= 0x06FFU) {
    return TextShapingScript::arabic;
  }
  if (0x0900U <= codepoint && codepoint <= 0x097FU) {
    return TextShapingScript::devanagari;
  }
  return TextShapingScript::common;
}

} // namespace cgpui
