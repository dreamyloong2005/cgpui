#include "x11_data_transfer_internal.hpp"

#include <cctype>

namespace cgpui {
namespace {
char hex_digit(unsigned value) {
  return "0123456789ABCDEF"[value & 0x0fU];
}

std::string encode_path(std::string_view path) {
  std::string encoded{"file://"};
  encoded.reserve(encoded.size() + path.size());
  for (const unsigned char value : path) {
    if (std::isalnum(value) != 0 || value == '/' || value == '-' ||
        value == '_' || value == '.' || value == '~') {
      encoded.push_back(static_cast<char>(value));
    } else {
      encoded.push_back('%');
      encoded.push_back(hex_digit(value >> 4U));
      encoded.push_back(hex_digit(value));
    }
  }
  return encoded;
}

int decode_digit(char value) {
  if (value >= '0' && value <= '9') return value - '0';
  if (value >= 'a' && value <= 'f') return value - 'a' + 10;
  if (value >= 'A' && value <= 'F') return value - 'A' + 10;
  return -1;
}

std::string decode_path(std::string_view path) {
  std::string decoded;
  decoded.reserve(path.size());
  for (std::size_t index = 0; index < path.size(); ++index) {
    if (path[index] != '%' || index + 2U >= path.size()) {
      decoded.push_back(path[index]);
      continue;
    }
    const int high = decode_digit(path[index + 1U]);
    const int low = decode_digit(path[index + 2U]);
    if (high < 0 || low < 0) {
      decoded.push_back(path[index]);
      continue;
    }
    decoded.push_back(static_cast<char>((high << 4) | low));
    index += 2U;
  }
  return decoded;
}
}  // namespace

std::string x11_encode_uri_list(std::span<const std::string> paths) {
  // X11 advertises this payload as text/uri-list with UTF8_STRING fallback.
  std::string result;
  for (const std::string& path : paths) {
    if (path.empty() || path.front() != '/') continue;
    result += encode_path(path);
    result += "\r\n";
  }
  return result;
}

std::vector<std::string> x11_parse_uri_list(std::string_view payload) {
  std::vector<std::string> result;
  while (!payload.empty()) {
    const std::size_t newline = payload.find('\n');
    std::string_view line = payload.substr(0, newline);
    if (!line.empty() && line.back() == '\r') line.remove_suffix(1);
    constexpr std::string_view prefix = "file://";
    if (line.starts_with(prefix)) {
      line.remove_prefix(prefix.size());
      if (line.starts_with('/')) result.push_back(decode_path(line));
    }
    if (newline == std::string_view::npos) break;
    payload.remove_prefix(newline + 1U);
  }
  return result;
}

}  // namespace cgpui
