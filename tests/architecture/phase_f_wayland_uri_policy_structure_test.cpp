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
  const std::string header = read_source("src/platform/linux/wayland_uri_list_internal.hpp");
  const std::string source = read_source("src/platform/linux/wayland_uri_list.cpp");
  const std::string helpers = read_source("src/platform/linux/wayland_helpers.cpp");
  const std::string payload = read_source("src/platform/linux/wayland_data_device_payload.cpp");
  const std::string behavior = read_source("tests/platform/wayland_uri_list_test.cpp");
  const std::string compositor_behavior = read_source("tests/platform/wayland_pointer_button_test.cpp");
  const std::string manifests = read_source("tests/platform/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{&header, &source, &helpers, &payload, &behavior,
                                &compositor_behavior, &manifests};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(header, "parse_uri_list(std::string_view payload)") ||
      !contains(payload, "#include \"wayland_uri_list_internal.hpp\"") ||
      contains(helpers, "parse_uri_list(")) return 2;
  if (!contains(source, "ascii_iequals(authority, \"localhost\")") ||
      !contains(source, "percent_decode_path") ||
      !contains(source, "decoded == '\\0'") ||
      !contains(source, "path.find_first_of(\"?#\")")) return 3;
  if (!contains(behavior, "file://remote.example") ||
      !contains(behavior, "bad%2") || !contains(behavior, "nul%00suffix") ||
      !contains(compositor_behavior, "remote.example/share/ignored")) return 4;
  if (line_count(header) > 25 || line_count(source) > 110 ||
      line_count(helpers) > 80 || line_count(behavior) > 45 ||
      line_count(compositor_behavior) > 370) return 5;
  if (!contains(manifests, "\"wayland_uri_list_test.cpp\"") ||
      !contains(manifests, "\"phase_f_wayland_uri_policy_structure_test.cpp\"")) return 6;
  return 0;
}
