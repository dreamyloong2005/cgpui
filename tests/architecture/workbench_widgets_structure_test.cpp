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

// C9 module layout: one public leaf header per widget, sources under
// src/ui/workbench split into model / layout / input / paint files, and a
// thin aggregate header. Widgets take the runtime context by forward
// declaration only.
namespace {

std::string read(const std::string& path) {
  const char* root = CGPUI_SOURCE_ROOT;
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

const std::vector<std::pair<std::string, std::size_t>> header_limits{
    {"include/cgpui/ui/anchored.hpp", 80},
    {"include/cgpui/ui/popover_menu.hpp", 200},
    {"include/cgpui/ui/splitter.hpp", 160},
    {"include/cgpui/ui/tab_strip.hpp", 160},
    {"include/cgpui/ui/tooltip.hpp", 130},
    {"include/cgpui/ui/tree_view.hpp", 200},
    {"include/cgpui/ui/widget_tree.hpp", 130},
    {"include/cgpui/ui/workbench_theme.hpp", 70},
    {"include/cgpui/ui/workbench_widgets.hpp", 20},
};

const std::vector<std::pair<std::string, std::size_t>> source_limits{
    {"src/ui/workbench/anchored.cpp", 130},
    {"src/ui/workbench/popover_menu_geometry.cpp", 80},
    {"src/ui/workbench/popover_menu_input.cpp", 220},
    {"src/ui/workbench/popover_menu_layout.cpp", 220},
    {"src/ui/workbench/popover_menu_paint.cpp", 160},
    {"src/ui/workbench/splitter_input.cpp", 240},
    {"src/ui/workbench/splitter_layout.cpp", 230},
    {"src/ui/workbench/splitter_paint.cpp", 120},
    {"src/ui/workbench/tab_strip_input.cpp", 180},
    {"src/ui/workbench/tab_strip_layout.cpp", 220},
    {"src/ui/workbench/tab_strip_paint.cpp", 170},
    {"src/ui/workbench/tooltip.cpp", 190},
    {"src/ui/workbench/tooltip_paint.cpp", 120},
    {"src/ui/workbench/tooltip_popup.cpp", 60},
    {"src/ui/workbench/popover_menu_popups.cpp", 120},
    {"src/ui/workbench/popover_menu_native.cpp", 60},
    {"src/ui/workbench/widget_popup.cpp", 150},
    {"src/ui/workbench/widget_popup_internal.hpp", 80},
    {"src/ui/workbench/tree_view_input.cpp", 250},
    {"src/ui/workbench/tree_view_layout.cpp", 120},
    {"src/ui/workbench/tree_view_model.cpp", 250},
    {"src/ui/workbench/tree_view_paint.cpp", 120},
    {"src/ui/workbench/widget_node_element.cpp", 70},
    {"src/ui/workbench/widget_tree_builder.cpp", 120},
    {"src/ui/workbench/workbench_internal.hpp", 70},
    {"src/ui/workbench/workbench_theme.cpp", 140},
};

} // namespace

int main() {
  try {
    for (const auto& limits : {header_limits, source_limits}) {
      for (const auto& [path, limit] : limits) {
        require(lines(read(path)) <= limit,
                path + " exceeds " + std::to_string(limit) + " lines");
      }
    }
    for (const auto& [path, limit] : header_limits) {
      const std::string header = read(path);
      require(!has(header, "runtime_context.hpp") &&
                  !has(header, "window_runtime.hpp") &&
                  !has(header, "windows.h") && !has(header, "wayland"),
              path + " must not pull in the runtime or platform headers");
    }
    const std::string aggregate = read("include/cgpui/ui/workbench_widgets.hpp");
    require(!has(aggregate, "class ") && !has(aggregate, "struct "),
            "workbench_widgets.hpp stays an include-only aggregate");
    // Painting, input and layout live in separate files per widget.
    require(has(read("src/ui/workbench/splitter_paint.cpp"), "Splitter::paint(") &&
                !has(read("src/ui/workbench/splitter_input.cpp"), "Splitter::paint("),
            "splitter paint belongs to splitter_paint.cpp");
    require(has(read("src/ui/workbench/tree_view_layout.cpp"), "UniformListVisibleRange"),
            "the tree view reports its rows as a uniform_list range");
    require(has(read("src/ui/workbench/popover_menu_paint.cpp"), "draw_box_shadow") &&
                has(read("src/ui/workbench/popover_menu_paint.cpp"), "draw_backdrop_blur"),
            "the menu surface uses the C5 shadow and optional C6 blur");
    require(has(read("src/ui/workbench/popover_menu_layout.cpp"), "measure_text("),
            "menu labels and accelerators use real text metrics");
    const std::string module = read("packages/ui/mcpp.toml") +
      read("examples/workbench/mcpp.toml");
    require(has(module, "\"../../src/ui/workbench/*.cpp\"") && has(module, "[targets.example_workbench]"),
            "the capability module owns the sources and the example");
    require(!has(read("mcpp.toml"), "workbench_"),
            "workbench targets stay out of the root mcpp.toml");
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
