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
      "src/platform/macos/macos_text_services_internal.hpp",
      "src/platform/macos/macos_font_discovery.mm",
      "src/platform/macos/macos_text_input.mm",
      "src/platform/macos/macos_ime_placement.mm",
      "src/platform/macos/macos_text_services_diagnostics.mm",
  };
  for (const char* file : files) {
    const std::string contents = read_source(file);
    if (contents.empty() || line_count(contents) > 220 ||
        contains(contents, "autorelease") || contains(contents, " retain]") ||
        contains(contents, " release]")) {
      return 1;
    }
  }
  const std::string header = read_source(files[0]);
  if (!contains(header, "macos_discover_fonts") ||
      !contains(header, "macos_nsrange_to_utf8_bytes") ||
      !contains(header, "macos_candidate_rect_to_screen")) {
    return 2;
  }
  const std::string content =
      read_source("src/platform/macos/macos_content_view.mm");
  if (!contains(content, "NSTextInputClient") ||
      !contains(content, "setMarkedText:") ||
      !contains(content, "insertText:") ||
      !contains(content, "interpretKeyEvents:") ||
      !contains(content, "firstRectForCharacterRange:")) {
    return 3;
  }
  const std::string module = read_source("build/xmake/phase_h_macos.lua");
  const std::string targets =
      read_source("build/xmake/phase_h_structure_targets.lua");
  if (!contains(module, "target(\"macos_font_discovery_test\")") ||
      !contains(module, "target(\"macos_text_input_test\")") ||
      !contains(targets, "target(\"phase_h_macos_text_structure_test\")")) {
    return 4;
  }
  return 0;
}
