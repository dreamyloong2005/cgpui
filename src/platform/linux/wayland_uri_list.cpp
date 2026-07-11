#include "wayland_uri_list_internal.hpp"

#include <optional>

namespace cgpui {
namespace {
std::optional<unsigned char> hex_digit(char value) {
  if (value >= '0' && value <= '9') return value - '0';
  if (value >= 'a' && value <= 'f') return value - 'a' + 10;
  if (value >= 'A' && value <= 'F') return value - 'A' + 10;
  return std::nullopt;
}

char ascii_lower(char value) {
  return value >= 'A' && value <= 'Z' ? static_cast<char>(value + ('a' - 'A'))
                                      : value;
}

bool ascii_iequals(std::string_view lhs, std::string_view rhs) {
  if (lhs.size() != rhs.size()) return false;
  for (std::size_t index = 0; index < lhs.size(); ++index) {
    if (ascii_lower(lhs[index]) != ascii_lower(rhs[index])) return false;
  }
  return true;
}

std::optional<std::string> percent_decode_path(std::string_view value) {
  std::string result;
  result.reserve(value.size());
  for (std::size_t index = 0; index < value.size(); ++index) {
    char decoded = value[index];
    if (decoded == '%') {
      if (index + 2 >= value.size()) return std::nullopt;
      const auto high = hex_digit(value[index + 1]);
      const auto low = hex_digit(value[index + 2]);
      if (!high || !low) return std::nullopt;
      decoded = static_cast<char>((*high << 4U) | *low);
      index += 2;
    }
    if (decoded == '\0') return std::nullopt;
    result.push_back(decoded);
  }
  return result;
}

std::optional<std::string> local_path_from_uri(std::string_view uri) {
  const auto colon = uri.find(':');
  if (colon == std::string_view::npos ||
      !ascii_iequals(uri.substr(0, colon), "file")) return std::nullopt;
  std::string_view remainder = uri.substr(colon + 1U);
  std::string_view path;
  if (remainder.starts_with("//")) {
    remainder.remove_prefix(2);
    const auto slash = remainder.find('/');
    if (slash == std::string_view::npos) return std::nullopt;
    const std::string_view authority = remainder.substr(0, slash);
    if (!authority.empty() && !ascii_iequals(authority, "localhost")) {
      return std::nullopt;
    }
    path = remainder.substr(slash);
  } else {
    path = remainder;
  }
  if (!path.starts_with('/') || path.find_first_of("?#") != std::string_view::npos)
    return std::nullopt;
  return percent_decode_path(path);
}
} // namespace

std::vector<std::string> parse_uri_list(std::string_view payload) {
  std::vector<std::string> files;
  while (!payload.empty()) {
    const auto line_end = payload.find('\n');
    std::string_view line = line_end == std::string_view::npos
        ? payload : payload.substr(0, line_end);
    if (!line.empty() && line.back() == '\r') line.remove_suffix(1);
    if (!line.empty() && line.front() != '#') {
      if (auto path = local_path_from_uri(line)) files.push_back(std::move(*path));
    }
    if (line_end == std::string_view::npos) break;
    payload.remove_prefix(line_end + 1U);
  }
  return files;
}

} // namespace cgpui
