#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {
std::string read_source(const char* path) {
  const char* root = CGPUI_SOURCE_ROOT;
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
      "src/platform/linux/linux_atspi_api_internal.hpp",
      "src/platform/linux/x11/x11_application_services.cpp",
      "src/platform/linux/x11/x11_accessibility.cpp",
  };
  for (const char* file : files) {
    const std::string source = read_source(file);
    if (source.empty() || line_count(source) > 180) return 1;
  }
  const std::string services = read_source(files[1]);
  const std::string accessibility = read_source(files[2]);
  if (!contains(services, ".backend = \"x11\"") ||
      !contains(services, "native_menu_item_count") ||
      !contains(accessibility, "linux_atspi_ensure_dbus_connection") ||
      !contains(accessibility, "linux_atspi_update_accessibility_tree")) {
    return 2;
  }
  const std::string module =
      read_source("packages/platform-linux-atspi/mcpp.toml") +
      read_source("tests/platform/mcpp.toml");
  const std::string manifests = read_source("examples/hello_window/mcpp.toml");
  if (!contains(module, "[targets.cgpui_platform_linux_atspi]") ||
      !contains(module, "\"x11_platform_services_test.cpp\"") ||
      !contains(manifests, "\"tests/x11_demo_smoke_flow.cpp\"")) {
    return 3;
  }
  return 0;
}
