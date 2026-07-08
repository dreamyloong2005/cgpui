#include <array>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string source_root() {
  if (const char* root = std::getenv("CGPUI_SOURCE_ROOT"); root != nullptr) {
    return root;
  }
  return ".";
}

std::string read_source(const char* path) {
  const std::string root = source_root();
  std::ifstream source(root + "/" + path);
  if (!source) {
    source.open(path);
  }
  if (!source) {
    return {};
  }

  return std::string{
      std::istreambuf_iterator<char>(source),
      std::istreambuf_iterator<char>()};
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

template <std::size_t N>
bool contains_any(const std::string& text,
                  const std::array<const char*, N>& values) {
  for (const char* value : values) {
    if (contains(text, value)) {
      return true;
    }
  }
  return false;
}

} // namespace

int main() {
  const std::string xmake = read_source("xmake.lua");
  const std::string ids_header =
      read_source("include/cgpui/ui/element_ids.hpp");
  const std::string dynamic_header =
      read_source("include/cgpui/ui/element_core.hpp");
  const std::string static_header =
      read_source("include/cgpui/ui/static_element_tree.hpp");
  const std::string static_source =
      read_source("src/ui/static_element_tree.cpp");
  const std::string render_header = read_source("include/cgpui/ui/render.hpp");
  const std::string view_header = read_source("include/cgpui/ui/view.hpp");
  const std::string runtime_header =
      read_source("include/cgpui/ui/window_runtime.hpp");
  const std::string diagnostics_header =
      read_source("include/cgpui/ui/runtime_diagnostics.hpp");
  const std::string static_runtime_source =
      read_source("src/ui/runtime_static_rendering.cpp");
  const std::string static_runtime_elements_source =
      read_source("src/ui/runtime_static_elements.cpp");
  const std::string static_runtime_accessibility_source =
      read_source("src/ui/runtime_static_accessibility.cpp");
  const std::string frame_results_source =
      read_source("src/ui/runtime_renderer_frame_results.cpp");
  const std::string event_input_source =
      read_source("src/ui/runtime_event_input.cpp");
  const std::string event_routes_source =
      read_source("src/ui/runtime_event_routes.cpp");
  const std::string ui_header = read_source("include/cgpui/ui/ui.hpp");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");

  if (xmake.empty() || ids_header.empty() || dynamic_header.empty() ||
      static_header.empty() || static_source.empty() || render_header.empty() ||
      view_header.empty() || runtime_header.empty() ||
      diagnostics_header.empty() || static_runtime_source.empty() ||
      static_runtime_elements_source.empty() ||
      static_runtime_accessibility_source.empty() ||
      frame_results_source.empty() || event_input_source.empty() ||
      event_routes_source.empty() ||
      ui_header.empty() || task_plan.empty() || findings.empty()) {
    return 1;
  }

  if (!contains(xmake, "target(\"static_element_tree_test\")") ||
      !contains(xmake, "tests/ui/static_element_tree_test.cpp") ||
      !contains(xmake, "target(\"static_render_runtime_test\")") ||
      !contains(xmake, "tests/ui/static_render_runtime_test.cpp") ||
      !contains(xmake, "target(\"zero_cost_abstraction_structure_test\")") ||
      !contains(xmake,
                "tests/architecture/zero_cost_abstraction_structure_test.cpp")) {
    return 2;
  }

  if (!contains(ids_header, "struct ElementId") ||
      !contains(ids_header, "struct ViewId") ||
      !contains(ids_header, "struct ElementKey") ||
      !contains(dynamic_header, "#include \"cgpui/ui/element_ids.hpp\"") ||
      contains(dynamic_header, "struct ElementId") ||
      contains(dynamic_header, "struct ViewId") ||
      contains(dynamic_header, "struct ElementKey")) {
    return 3;
  }

  if (!contains(static_header, "enum class StaticElementKind") ||
      !contains(static_header, "struct StaticElementChildRange") ||
      !contains(static_header, "struct StaticElementNode") ||
      !contains(static_header, "class StaticElementTreeView") ||
      !contains(static_header, "std::span<const StaticElementNode>") ||
      !contains(static_header, "std::span<const ElementId>") ||
      !contains(static_header, "for_each_preorder")) {
    return 4;
  }

  constexpr std::array forbidden{
      "<functional>",
      "std::function",
      "virtual",
      "std::unique_ptr",
      "AnyElement",
      "std::any",
      "dynamic_cast",
      "std::vector",
  };
  if (contains_any(static_header, forbidden) ||
      contains_any(static_source, forbidden)) {
    return 5;
  }

  if (!contains(render_header,
                "#include \"cgpui/ui/static_element_tree.hpp\"") ||
      !contains(render_header, "using IntoElement = AnyElement") ||
      !contains(render_header, "using StaticIntoElement = StaticElementTreeView") ||
      !contains(render_header, "concept Render") ||
      !contains(render_header, "concept StaticRender")) {
    return 6;
  }

  if (!contains(view_header, "supports_static_render()") ||
      !contains(view_header, "render_static(ViewContext& context)") ||
      !contains(runtime_header, "static_element_tree() const") ||
      !contains(diagnostics_header, "enum class RenderTreeKind") ||
      !contains(diagnostics_header, "static_element_tree") ||
      !contains(diagnostics_header, "static_node_count")) {
    return 9;
  }

  constexpr std::array static_runtime_forbidden{
      "<functional>",
      "std::function",
      "AnyElement",
      "std::any",
      "dynamic_cast",
      "std::unique_ptr",
      "std::unordered_map",
  };
  if (!contains(static_runtime_source, "install_static_render_tree(") ||
      !contains(static_runtime_source, "render_static(render_context)") ||
      !contains(static_runtime_source, "set_static_element_tree(") ||
      contains_any(static_runtime_source, static_runtime_forbidden)) {
    return 10;
  }

  if (!contains(static_runtime_elements_source,
                "WindowRuntime::set_static_element_tree(") ||
      !contains(static_runtime_elements_source,
                "WindowRuntime::static_element_tree() const") ||
      !contains(static_runtime_elements_source,
                "WindowRuntime::static_element_node(") ||
      contains_any(static_runtime_elements_source, static_runtime_forbidden)) {
    return 11;
  }

  if (!contains(static_runtime_accessibility_source,
                "WindowRuntime::static_accessibility_snapshot(") ||
      !contains(static_runtime_accessibility_source, "StaticElementKind::text") ||
      !contains(static_runtime_accessibility_source,
                "StaticElementKind::image")) {
    return 12;
  }

  if (!contains(frame_results_source, "install_static_render_tree(") ||
      !contains(frame_results_source,
                "RenderTreeKind::static_element_tree") ||
      !contains(frame_results_source,
                "RenderTreeKind::dynamic_element_tree")) {
    return 13;
  }

  if (!contains(event_input_source, "static_element_tree_") ||
      !contains(event_input_source, "element_enabled(") ||
      !contains(event_input_source, "element_focusable(") ||
      !contains(event_routes_source, "static_element_tree_") ||
      !contains(event_routes_source, "static_element_node(")) {
    return 14;
  }

  if (!contains(ui_header, "#include \"cgpui/ui/static_element_tree.hpp\"")) {
    return 7;
  }

  if (!contains(task_plan, "Zero-cost abstraction is a hard") ||
      !contains(task_plan, "Static widget fast paths") ||
      !contains(task_plan, "dynamic escape hatches") ||
      !contains(findings, "Zero-Cost Static Element Fast Path")) {
    return 8;
  }

  return 0;
}
