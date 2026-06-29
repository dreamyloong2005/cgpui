#include "cgpui/platform/clipboard.hpp"

#include <memory>
#include <string_view>

namespace {

int test_memory_clipboard_round_trips_utf8_text() {
  cgpui::MemoryClipboard clipboard;
  if (clipboard.read_text().has_value()) {
    return 1;
  }

  if (!clipboard.write_text("hello")) {
    return 2;
  }
  const auto first = clipboard.read_text();
  if (!first || *first != std::string_view{"hello"}) {
    return 3;
  }

  if (!clipboard.write_text("\xE4\xB8\xAD")) {
    return 4;
  }
  const auto second = clipboard.read_text();
  return second && *second == std::string_view{"\xE4\xB8\xAD"} ? 0 : 5;
}

int test_platform_clipboard_uses_text_clipboard_contract() {
  const auto clipboard = cgpui::create_platform_clipboard();
  if (clipboard == nullptr) {
    return 6;
  }

  if (!clipboard->write_text("platform text")) {
    return 7;
  }
  const auto text = clipboard->read_text();
  return text && *text == std::string_view{"platform text"} ? 0 : 8;
}

} // namespace

int main() {
  if (const int result = test_memory_clipboard_round_trips_utf8_text();
      result != 0) {
    return result;
  }
  if (const int result = test_platform_clipboard_uses_text_clipboard_contract();
      result != 0) {
    return result;
  }
  return 0;
}
