#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// Framework fixes after C9: each fix lives in a focused file with a line
// limit; the workbench widgets rely on the element layer for placement and
// hit testing instead of a private walk.
namespace {

std::string read(const std::string& path) {
  const char* root = std::getenv("CGPUI_SOURCE_ROOT");
  std::ifstream stream(std::filesystem::path(root ? root : ".") / path);
  if (!stream) throw std::runtime_error("missing focused module: " + path);
  return {std::istreambuf_iterator<char>(stream), {}};
}

void require(bool value, const std::string& message) {
  if (!value) throw std::runtime_error(message);
}

bool has(const std::string& source, std::string_view text) {
  return source.find(text) != std::string::npos;
}

std::size_t lines(const std::string& source) {
  return static_cast<std::size_t>(
             std::count(source.begin(), source.end(), '\n')) + 1U;
}

const std::vector<std::pair<std::string, std::size_t>> source_limits{
    {"src/ui/element_layout_placement.cpp", 120},
    {"src/ui/element_stack_paint.cpp", 40},
    {"src/ui/element_tree_traversal.cpp", 150},
};

} // namespace

int main() {
  try {
    for (const auto& [path, limit] : source_limits) {
      require(lines(read(path)) <= limit,
              path + " exceeds " + std::to_string(limit) + " lines");
    }
    // Item 1: containers place children with place_layout (window
    // coordinates at any depth); the widget tree uses ElementTree hit tests.
    require(has(read("src/ui/element_style_nodes.cpp"), "place_layout(") &&
                has(read("src/ui/element_flex_layout.cpp"), "place_layout("),
            "containers place children through Element::place_layout");
    require(has(read("src/ui/element_tree_traversal.cpp"), "hit_test_subtree("),
            "ElementTree hit-tests its tree children");
    require(!std::filesystem::exists(
                std::filesystem::path(std::getenv("CGPUI_SOURCE_ROOT")
                                          ? std::getenv("CGPUI_SOURCE_ROOT")
                                          : ".") /
                "src/ui/workbench/widget_root_element.cpp") &&
                !has(read("include/cgpui/ui/widget_tree.hpp"), "WidgetRootElement"),
            "the widget tree has no private hit-test root");
    require(!has(read("xmake.lua"), "framework_fixes"),
            "framework fix targets stay out of the root xmake.lua");
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
