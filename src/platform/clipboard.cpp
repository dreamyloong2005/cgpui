#include "cgpui/platform/clipboard.hpp"

namespace cgpui {

std::optional<std::string> MemoryClipboard::read_text() const {
  return text_;
}

bool MemoryClipboard::write_text(std::string_view text) {
  text_ = std::string(text);
  return true;
}

std::unique_ptr<Clipboard> create_platform_clipboard() {
  return std::make_unique<MemoryClipboard>();
}

} // namespace cgpui
