#include "clipboard_wayland_internal.hpp"

namespace cgpui {

#if defined(__linux__)
namespace {

std::string_view trim_ascii(std::string_view value) {
  while (!value.empty() &&
         (value.front() == ' ' || value.front() == '\t')) {
    value.remove_prefix(1);
  }
  while (!value.empty() &&
         (value.back() == ' ' || value.back() == '\t')) {
    value.remove_suffix(1);
  }
  return value;
}

char ascii_lower(char value) {
  return value >= 'A' && value <= 'Z'
      ? static_cast<char>(value + ('a' - 'A'))
      : value;
}

bool ascii_equal(std::string_view lhs, std::string_view rhs) {
  if (lhs.size() != rhs.size()) return false;
  for (std::size_t index = 0; index < lhs.size(); ++index) {
    if (ascii_lower(lhs[index]) != ascii_lower(rhs[index])) return false;
  }
  return true;
}

std::string_view unquote(std::string_view value) {
  if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
    value.remove_prefix(1);
    value.remove_suffix(1);
  }
  return value;
}

} // namespace

int wayland_clipboard_text_mime_rank(std::string_view mime_type) {
  mime_type = trim_ascii(mime_type);
  const auto separator = mime_type.find(';');
  if (!ascii_equal(trim_ascii(mime_type.substr(0, separator)), "text/plain")) {
    return 0;
  }
  if (separator == std::string_view::npos) return 1;

  mime_type.remove_prefix(separator + 1U);
  while (!mime_type.empty()) {
    const auto next = mime_type.find(';');
    const auto parameter = trim_ascii(mime_type.substr(0, next));
    const auto equals = parameter.find('=');
    if (equals != std::string_view::npos &&
        ascii_equal(trim_ascii(parameter.substr(0, equals)), "charset")) {
      const auto charset =
          unquote(trim_ascii(parameter.substr(equals + 1U)));
      return ascii_equal(charset, "utf-8") || ascii_equal(charset, "utf8")
          ? 2
          : 0;
    }
    if (next == std::string_view::npos) break;
    mime_type.remove_prefix(next + 1U);
  }
  return 1;
}
#endif

} // namespace cgpui
