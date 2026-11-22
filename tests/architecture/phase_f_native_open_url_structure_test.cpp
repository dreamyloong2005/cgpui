#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {
std::string read_source(const char* path) {
  const char* root = CGPUI_SOURCE_ROOT;
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
  return source ? std::string{std::istreambuf_iterator<char>(source), {}}
                : std::string{};
}
bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}
std::size_t line_count(const std::string& text) {
  std::size_t lines = 0;
  for (const char value : text) lines += value == '\n' ? 1U : 0U;
  return lines;
}
} // namespace

int main() {
  const std::string public_header = read_source("include/cgpui/platform/platform_open_url.hpp");
  const std::string application = read_source("include/cgpui/platform/platform_application.hpp");
  const std::string empty = read_source("src/platform/empty.cpp");
  const std::string wayland = read_source("src/platform/linux/wayland_application_services.cpp");
  const std::string header = read_source("src/platform/win32/win32_open_url_internal.hpp");
  const std::string source = read_source("src/platform/win32/win32_open_url.cpp");
  const std::string win32_application = read_source("src/platform/win32/win32_application.cpp");
  const std::string behavior = read_source("tests/platform/win32_open_url_plan_test.cpp");
  const std::string manifests = read_source("tests/platform/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{&public_header, &application, &empty, &wayland,
      &header, &source, &win32_application, &behavior, &manifests};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(public_header, "struct PlatformOpenUrlResult") ||
      !contains(public_header, "bool opened = false") ||
      !contains(application, "PlatformOpenUrlResult open_url(std::string url)")) return 2;
  if (!contains(empty, ".backend = \"unsupported\"") ||
      !contains(wayland, ".backend = \"wayland\"") ||
      !contains(wayland, "open URL unsupported by wayland")) return 3;
  if (!contains(header, "win32_open_url_plan(") ||
      !contains(source, "url.find('\\0')") ||
      !contains(source, "ShellExecuteW(") ||
      !contains(source, "result > 32") ||
      !contains(win32_application, "return win32_open_url(url)")) return 4;
  if (!contains(behavior, "https://example.com/a?b=c#d") ||
      !contains(behavior, "std::string_view{\"a\\0b\", 3}") ||
      !contains(behavior, "win32_shell_execute_succeeded(33)")) return 5;
  if (line_count(public_header) > 20 || line_count(header) > 30 ||
      line_count(source) > 55 || line_count(behavior) > 25 ||
      line_count(wayland) > 50 || line_count(win32_application) > 220) return 6;
  if (!contains(manifests, "\"win32_open_url_plan_test.cpp\"") ||
      !contains(manifests, "\"phase_f_native_open_url_structure_test.cpp\"")) return 7;
  return 0;
}
