#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {
std::string read_source(const char* path) {
  const char* root = std::getenv("CGPUI_SOURCE_ROOT");
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
  if (!source) source.open(path);
  return source ? std::string{std::istreambuf_iterator<char>(source), {}}
                : std::string{};
}
bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}
std::size_t line_count(const std::string& text) {
  std::size_t lines = 0;
  for (char value : text) lines += value == '\n';
  return lines + (!text.empty() && text.back() != '\n');
}
}  // namespace

int main() {
  constexpr const char* files[] = {
      "src/platform/linux/x11/x11_clipboard_internal.hpp",
      "src/platform/linux/x11/x11_clipboard.cpp",
      "src/platform/linux/x11/x11_clipboard_events.cpp",
      "src/platform/linux/x11/x11_data_transfer_internal.hpp",
      "src/platform/linux/x11/x11_data_transfer.cpp",
      "src/platform/linux/x11/x11_drag_drop.cpp",
  };
  for (const char* file : files) {
    const std::string source = read_source(file);
    if (source.empty() || line_count(source) > 240) return 1;
  }
  const std::string clipboard = read_source(files[1]);
  const std::string events = read_source(files[2]);
  const std::string transfer = read_source(files[4]);
  const std::string drag = read_source(files[5]);
  const std::string connection =
      read_source("src/platform/linux/x11/x11_connection.cpp");
  if (!contains(clipboard, "xcb_set_selection_owner") ||
      !contains(events, "XCB_SELECTION_REQUEST") ||
      !contains(events, "XCB_SELECTION_NOTIFY") ||
      !contains(transfer, "text/uri-list") ||
      !contains(transfer, "UTF8_STRING") ||
      !contains(connection, "XdndEnter") ||
      !contains(connection, "XdndFinished") ||
      !contains(drag, "xcb_convert_selection") ||
      !contains(drag, "DragDropped")) {
    return 2;
  }
  const std::string factory = read_source("src/platform/clipboard.cpp");
  const std::string module =
      read_source("build/xmake/phase_i_linux_backends.lua");
  if (!contains(factory, "LinuxPlatformBackend::x11") ||
      !contains(factory, "create_x11_clipboard") ||
      !contains(module, "target(\"x11_clipboard_test\")") ||
      !contains(module, "target(\"x11_drag_drop_test\")")) {
    return 3;
  }
  return 0;
}
