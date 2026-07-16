#include "cgpui/platform/clipboard.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

int main() {
  const char* display = std::getenv("DISPLAY");
  if (display == nullptr || *display == '\0') return 0;
  setenv("CGPUI_LINUX_BACKEND", "x11", 1);
  unsetenv("WAYLAND_DISPLAY");

  auto owner = cgpui::create_platform_clipboard();
  auto reader = cgpui::create_platform_clipboard();
  const std::string expected = "CGPUI X11 selection: \xE4\xB8\xAD\xE6\x96\x87";
  if (!owner->write_text(expected)) return 1;
  const auto text = reader->read_text();
  if (!text || *text != expected) {
    std::cerr << "X11 clipboard UTF-8 readback failed\n";
    return 2;
  }

  const std::vector<std::string> files{
      "/tmp/cgpui one.txt", "/home/cgpui/two.txt"};
  if (!owner->write_files(files)) return 3;
  const auto read_files = reader->read_files();
  if (!read_files || *read_files != files) {
    std::cerr << "X11 clipboard URI-list readback failed\n";
    return 4;
  }
  return 0;
}
