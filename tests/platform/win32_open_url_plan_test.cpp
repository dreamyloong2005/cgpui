#include "win32_open_url_internal.hpp"

int main() {
  const auto url = cgpui::win32_open_url_plan("https://example.com/a?b=c#d");
  if (!url.has_value() || *url != L"https://example.com/a?b=c#d") return 1;
  if (cgpui::win32_open_url_plan("").has_value() ||
      cgpui::win32_open_url_plan(std::string_view{"a\0b", 3}).has_value()) return 2;
  if (cgpui::win32_shell_execute_succeeded(32) ||
      !cgpui::win32_shell_execute_succeeded(33)) return 3;
  return 0;
}
