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
  for (const char value : text) lines += value == '\n';
  return lines + (!text.empty() && text.back() != '\n');
}

}  // namespace

int main() {
  constexpr const char* files[] = {
      "src/platform/macos/macos_platform_services_internal.hpp",
      "src/platform/macos/macos_native_menu.mm",
      "src/platform/macos/macos_file_dialog.mm",
      "src/platform/macos/macos_message_dialog.mm",
      "src/platform/macos/macos_open_url.mm",
      "src/platform/macos/macos_window_chrome.mm",
      "src/platform/macos/macos_platform_services.mm",
  };
  for (const char* file : files) {
    const std::string contents = read_source(file);
    if (contents.empty() || line_count(contents) > 220 ||
        contains(contents, "autorelease") || contains(contents, " retain]") ||
        contains(contents, " release]")) {
      return 1;
    }
  }
  const std::string application =
      read_source("src/platform/macos/macos_application_internal.hpp");
  if (!contains(application, "install_native_menu") ||
      !contains(application, "show_native_file_dialog") ||
      !contains(application, "show_native_message_dialog") ||
      !contains(application, "open_url")) {
    return 2;
  }
  const std::string menu = read_source(files[1]);
  if (!contains(menu, "NSMenu") || !contains(menu, "setKeyEquivalent") ||
      !contains(menu, "dispatch_native_menu")) {
    return 3;
  }
  const std::string services = read_source(files[6]);
  const std::string chrome = read_source(files[5]);
  if (!contains(services, "NSThread isMainThread") ||
      !contains(chrome, "NSThread isMainThread")) {
    return 6;
  }
  const std::string delegate =
      read_source("src/platform/macos/macos_application_delegate.mm");
  if (!contains(delegate, "applicationShouldHandleReopen") ||
      !contains(delegate, "applicationShouldTerminateAfterLastWindowClosed")) {
    return 4;
  }
  const std::string module = read_source("build/xmake/phase_h_macos.lua");
  const std::string targets =
      read_source("build/xmake/phase_h_structure_targets.lua");
  if (!contains(module, "target(\"macos_platform_services_test\")") ||
      !contains(targets, "target(\"phase_h_macos_services_structure_test\")")) {
    return 5;
  }
  return 0;
}
