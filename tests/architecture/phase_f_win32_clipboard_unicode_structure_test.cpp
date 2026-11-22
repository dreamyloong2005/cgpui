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
  const std::string internal = read_source(
      "src/platform/clipboard_win32_internal.hpp");
  const std::string text = read_source(
      "src/platform/clipboard_win32_text.cpp");
  const std::string read = read_source(
      "src/platform/clipboard_win32_read.cpp");
  const std::string write = read_source(
      "src/platform/clipboard_win32_write.cpp");
  const std::string behavior = read_source(
      "tests/platform/win32_clipboard_unicode_test.cpp");
  const std::string manifests = read_source("tests/platform/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{
      &internal, &text, &read, &write, &behavior, &manifests};
  for (const std::string* source : required) if (source->empty()) return 1;
  if (!contains(internal, "std::optional<std::wstring> widen_clipboard_text") ||
      !contains(internal, "std::optional<std::string> narrow_clipboard_text")) {
    return 2;
  }
  if (!contains(text, "MB_ERR_INVALID_CHARS") ||
      !contains(text, "WC_ERR_INVALID_CHARS") ||
      !contains(text, "value.find('\\0')") ||
      !contains(text, "value.find(L'\\0')") ||
      !contains(text, "value.size() > INT_MAX") ||
      !contains(text, "written == required")) return 3;
  if (!contains(write, "if (!wide_text) return false;") ||
      !contains(read, "narrow_clipboard_text(")) return 4;
  if (!contains(behavior, "\\xF0\\x9F\\x98\\x80") ||
      !contains(behavior, "line one\\r\\nemoji") ||
      !contains(behavior, "invalid_utf8") ||
      !contains(behavior, "embedded_nul") ||
      !contains(behavior, "L\"baseline\"")) return 5;
  if (!contains(manifests, "\"win32_clipboard_unicode_test.cpp\"") ||
      !contains(manifests,
                "\"phase_f_win32_clipboard_unicode_structure_test.cpp\"")) {
    return 6;
  }
  if (line_count(internal) > 50 || line_count(text) > 90 ||
      line_count(read) > 70 || line_count(write) > 80 ||
      line_count(behavior) > 130) return 7;
  return 0;
}
