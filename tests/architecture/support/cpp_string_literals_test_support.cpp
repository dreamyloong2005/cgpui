#include "cpp_string_literals_test_support.hpp"

#include <algorithm>
#include <cctype>
#include <optional>
#include <utility>

namespace architecture_test {
namespace {
struct SplicedSource {
  std::string_view original;
  std::string text;
  std::vector<std::size_t> offsets;
};

SplicedSource splice_lines(std::string_view source) {
  SplicedSource result{source, {}, {}};
  result.text.reserve(source.size());
  result.offsets.reserve(source.size() + 1);
  for (std::size_t at = 0; at < source.size();) {
    if (source[at] == '\\') {
      auto end = at + 1;
      if (end < source.size() && source[end] == '\r') ++end;
      if (end < source.size() && source[end] == '\n') { at = end + 1; continue; }
    }
    result.offsets.push_back(at);
    result.text += source[at++];
  }
  result.offsets.push_back(source.size());
  return result;
}

std::size_t skip_trivia(std::string_view source, std::size_t at) {
  while (at < source.size()) {
    if (std::isspace(static_cast<unsigned char>(source[at]))) { ++at; continue; }
    if (source.substr(at, 2) == "//") {
      const auto end = source.find('\n', at + 2);
      at = end == source.npos ? source.size() : end + 1;
    } else if (source.substr(at, 2) == "/*") {
      const auto end = source.find("*/", at + 2);
      at = end == source.npos ? source.size() : end + 2;
    } else break;
  }
  return at;
}

bool identifier_start(unsigned char value) {
  return std::isalpha(value) || value == '_' || value >= 0x80;
}
bool identifier_part(unsigned char value) {
  return identifier_start(value) || std::isdigit(value);
}
struct StringToken { std::size_t quote; bool raw; };
std::optional<StringToken> string_token(std::string_view source, std::size_t at) {
  for (const std::string_view prefix : {"u8R\"", "uR\"", "UR\"", "LR\"", "R\"",
                                        "u8\"", "u\"", "U\"", "L\"", "\""}) {
    if (source.substr(at).starts_with(prefix))
      return StringToken{at + prefix.size() - 1, prefix.ends_with("R\"")};
  }
  return {};
}

int digit_value(char value) {
  if (value >= '0' && value <= '9') return value - '0';
  if (value >= 'a' && value <= 'f') return value - 'a' + 10;
  if (value >= 'A' && value <= 'F') return value - 'A' + 10;
  return -1;
}
void append_codepoint(std::string& value, unsigned codepoint) {
  if (codepoint > 0x10ffff || (codepoint >= 0xd800 && codepoint <= 0xdfff)) codepoint = 0xfffd;
  if (codepoint <= 0x7f) { value += static_cast<char>(codepoint); return; }
  const unsigned bytes = codepoint <= 0x7ff ? 2 : codepoint <= 0xffff ? 3 : 4;
  value += static_cast<char>(((0xf0u << (4 - bytes)) & 0xffu) | (codepoint >> (6 * (bytes - 1))));
  for (unsigned remaining = bytes - 1; remaining != 0; --remaining)
    value += static_cast<char>(0x80u | ((codepoint >> (6 * (remaining - 1))) & 0x3fu));
}
void read_escape(std::string_view source, std::size_t& at, std::string& value) {
  const char escaped = source[at++];
  switch (escaped) {
    case 'a': value += '\a'; return;
    case 'b': value += '\b'; return;
    case 'f': value += '\f'; return;
    case 'n': value += '\n'; return;
    case 'r': value += '\r'; return;
    case 't': value += '\t'; return;
    case 'v': value += '\v'; return;
  }
  const bool octal = escaped >= '0' && escaped <= '7';
  const bool braced = (escaped == 'x' || escaped == 'o' || escaped == 'u') &&
      at < source.size() && source[at] == '{';
  if (!octal && escaped != 'x' && escaped != 'u' && escaped != 'U' && !braced) {
    value += escaped; return;
  }
  const unsigned base = octal || escaped == 'o' ? 8 : 16;
  auto limit = octal ? std::size_t{3} : escaped == 'u' ? std::size_t{4} :
      escaped == 'U' ? std::size_t{8} : source.size();
  if (octal) --at;
  else if (braced) { ++at; limit = source.size(); }
  unsigned codepoint = 0;
  std::size_t count = 0;
  while (at < source.size() && count < limit) {
    const int digit = digit_value(source[at]);
    if (digit < 0 || static_cast<unsigned>(digit) >= base) break;
    if (codepoint <= 0x10ffff) codepoint = codepoint * base + static_cast<unsigned>(digit);
    ++at; ++count;
  }
  if (braced && at < source.size() && source[at] == '}') ++at;
  if (count != 0) append_codepoint(value, codepoint);
  else value += escaped;
}

std::string read_string(const SplicedSource& input, StringToken token, std::size_t& at) {
  const std::string_view source{input.text};
  const auto quote = token.quote;
  if (token.raw) {
    const auto opening = source.substr(quote + 1, 17).find('(');
    if (opening != source.npos) {
      const auto begin = quote + 1 + opening;
      const std::string delimiter = ")" + std::string(source.substr(quote + 1, opening)) + "\"";
      // Translation-phase splicing is reverted inside a raw string's contents.
      const auto original_begin = input.offsets[begin] + 1;
      const auto end = input.original.find(delimiter, original_begin);
      const auto finish = end == source.npos ? input.original.size() : end + delimiter.size();
      at = static_cast<std::size_t>(std::lower_bound(input.offsets.begin(), input.offsets.end(), finish) - input.offsets.begin());
      return std::string(input.original.substr(original_begin, (end == source.npos ? input.original.size() : end) - original_begin));
    }
  }
  std::string value;
  at = quote + 1;
  while (at < source.size()) {
    const char character = source[at++];
    if (character == '"') break;
    if (character == '\\' && at < source.size()) read_escape(source, at, value);
    else value += character;
  }
  return value;
}
} // namespace

std::vector<std::string> cpp_string_literals(std::string_view source) {
  const auto input = splice_lines(source);
  source = input.text;
  std::vector<std::string> values;
  std::size_t at = 0;
  bool adjacent = false;
  while ((at = skip_trivia(source, at)) < source.size()) {
    if (const auto token = string_token(source, at)) {
      auto value = read_string(input, *token, at);
      if (adjacent) values.back() += value;
      else values.push_back(std::move(value));
      adjacent = true;
      // A user-defined suffix is part of this token, not an intervening identifier.
      if (at < source.size() && identifier_start(static_cast<unsigned char>(source[at])))
        do { ++at; } while (at < source.size() && identifier_part(static_cast<unsigned char>(source[at])));
      continue;
    }
    adjacent = false;
    if (identifier_start(static_cast<unsigned char>(source[at]))) {
      do { ++at; } while (at < source.size() && identifier_part(static_cast<unsigned char>(source[at])));
    } else if (std::isdigit(static_cast<unsigned char>(source[at]))) {
      // Digit separators are not character literals (for example 10'000).
      do { ++at; } while (at < source.size() &&
          (identifier_part(static_cast<unsigned char>(source[at])) ||
           source[at] == '\'' || source[at] == '.'));
    } else if (source[at++] == '\'') {
      while (at < source.size()) {
        const char character = source[at++];
        if (character == '\\' && at < source.size()) ++at;
        else if (character == '\'') break;
      }
    }
  }
  return values;
}
} // namespace architecture_test
