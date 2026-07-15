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
      "src/platform/macos/macos_accessibility_internal.hpp",
      "src/platform/macos/macos_accessibility_element.mm",
      "src/platform/macos/macos_accessibility_tree.mm",
      "src/platform/macos/macos_accessibility_actions.mm",
      "src/platform/macos/macos_accessibility_events.mm",
      "src/platform/macos/macos_accessibility_lifetime.mm",
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
  if (!contains(header, "MacOSAccessibilityProviderKey") ||
      !contains(header, "window_id") || !contains(header, "element_id") ||
      !contains(header, "generation")) {
    return 2;
  }
  const std::string tree = read_source(files[2]);
  if (!contains(tree, "accessibilityChildren") ||
      !contains(tree, "accessibilityFocusedUIElement") ||
      !contains(tree, "NSThread isMainThread")) {
    return 3;
  }
  const std::string window =
      read_source("src/platform/macos/macos_window_internal.hpp");
  if (!contains(window, "update_accessibility_tree") ||
      !contains(window, "MacOSAccessibilityState")) {
    return 4;
  }
  const std::string module = read_source("build/xmake/phase_h_macos.lua");
  const std::string targets =
      read_source("build/xmake/phase_h_structure_targets.lua");
  if (!contains(module, "target(\"macos_accessibility_test\")") ||
      !contains(targets,
                "target(\"phase_h_macos_accessibility_structure_test\")")) {
    return 5;
  }
  return 0;
}
