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
  for (const char value : text) lines += value == '\n' ? 1U : 0U;
  return lines;
}
} // namespace

int main() {
  const std::string internal =
      read_source("src/platform/clipboard_wayland_internal.hpp");
  const std::string mime =
      read_source("src/platform/clipboard_wayland_mime.cpp");
  const std::string read =
      read_source("src/platform/clipboard_wayland_read.cpp");
  const std::string behavior =
      read_source("tests/platform/wayland_clipboard_mime_negotiation_test.cpp");
  const std::string platform_structure =
      read_source("tests/architecture/platform_source_structure_test.cpp");
  const std::string manifests = read_source("tests/platform/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{
      &internal, &mime, &read, &behavior, &platform_structure, &manifests};
  for (const std::string* value : required) if (value->empty()) return 1;
  if (!contains(internal, "wayland_clipboard_text_mime_rank(") ||
      !contains(mime, "trim_ascii(") || !contains(mime, "ascii_equal(") ||
      !contains(mime, "unquote(") || !contains(mime, "text/plain") ||
      !contains(mime, "utf-8") || !contains(mime, "utf8")) return 2;
  if (!contains(read, "wayland_clipboard_text_mime_rank(mime_type)") ||
      !contains(read, "rank > best_rank") ||
      !contains(read, "preferred = mime_type")) return 3;
  if (!contains(behavior, "Text/Plain; Charset=UTF-8") ||
      !contains(behavior, "text/plain;charset=iso-8859-1") ||
      !contains(behavior, "TEXT/PLAIN") ||
      !contains(behavior, "normalized utf8") ||
      !contains(behavior, "case folded plain")) return 4;
  if (!contains(platform_structure,
                "\"src/platform/clipboard_wayland_mime.cpp\"") ||
      !contains(manifests,
                "\"wayland_clipboard_mime_negotiation_test.cpp\"") ||
      !contains(manifests,
                "\"phase_f_wayland_clipboard_mime_structure_test.cpp\"")) {
    return 5;
  }
  if (line_count(internal) > 150 || line_count(mime) > 90 ||
      line_count(read) > 70 || line_count(behavior) > 90) return 6;
  return 0;
}
