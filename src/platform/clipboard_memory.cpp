#include "cgpui/platform/clipboard.hpp"

#include <optional>
#include <string>
#include <string_view>

namespace cgpui {

std::optional<std::string> MemoryClipboard::read_text() const {
  return text_;
}

bool MemoryClipboard::write_text(std::string_view text) {
  text_ = std::string(text);
  return true;
}

} // namespace cgpui
