#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
void require(bool value, const char* message) {
  if (!value) throw std::runtime_error(message);
}
std::string read(const char* path, std::size_t limit) {
  std::ifstream stream(std::filesystem::path(CGPUI_SOURCE_ROOT) / path);
  if (!stream) throw std::runtime_error(std::string("missing static-window owner: ") + path);
  std::string source{std::istreambuf_iterator<char>(stream), {}};
  require(static_cast<std::size_t>(std::count(source.begin(), source.end(), '\n')) <= limit,
          "static-window owner exceeds its line budget");
  return source;
}
bool has(const std::string& source, std::string_view token) {
  return source.find(token) != source.npos;
}
} // namespace

int main() {
  try {
    const auto declarations = read(
        "include/cgpui/ui/detail/window_runtime_window_element_host_declarations.hpp", 45);
    for (const auto name : {"static_tree_for_record(", "install_static_tree_for_record(",
                            "dispatch_record_event_through_static_elements("}) {
      const auto start = declarations.find(name);
      require(start != declarations.npos, "static-window bridge declaration missing");
      const auto terminator = declarations.find_first_of(";{", start);
      require(terminator != declarations.npos && declarations[terminator] == ';',
              "static-window implementation leaked into declaration fragment");
    }
    const auto state = read("src/ui/runtime_state_elements_types_internal.hpp", 45);
    require(has(state, "StaticElementTreeView static_tree;") &&
                has(state, "std::unique_ptr<ElementTree> tree;"),
            "both tree representations belong to the private window host");
    const auto install = read("src/ui/runtime_window_static_elements.cpp", 60);
    require(has(install, "WindowRuntime::install_static_tree_for_record(") &&
                has(install, "WindowRuntime::static_tree_for_record(") &&
                has(install, "window_element_hosts_") &&
                has(install, "retire_tree(std::move(host.tree))") &&
                !has(install, "host.tree.reset()"),
            "focused static installer owns the per-window transition and deferred retirement");
    const auto routing = read("src/ui/runtime_window_static_element_routing.cpp", 90);
    require(has(routing, "WindowRuntime::dispatch_record_event_through_static_elements(") &&
                has(routing, "static_tree_for_record(record)") &&
                has(routing, "synthesize_pointer_gesture_state(") &&
                has(routing, "view.handle_event("),
            "static routing must remain in its focused implementation owner");
    const auto events = read("src/ui/runtime_window_element_routing.cpp", 110);
    require(has(events, "dispatch_record_event_through_static_elements(record, view, event)") &&
                !has(events, "StaticElementNode"),
            "dynamic routing delegates rather than absorbing static traversal");
    require(events.find("update_record_hover_cursor(record, event)") <
                events.find("static_tree_for_record(record)") &&
                !has(routing, "update_record_hover_cursor(") &&
                !has(routing, "dispatch_record_event_through_elements("),
            "cursor synchronization precedes tree dispatch without static/dynamic recursion");
    const auto cursor = read("src/ui/runtime_window_element_cursor.cpp", 100);
    require(has(cursor, "struct CursorAttempt") && has(cursor, "attempted->contains(cursor)") &&
                has(cursor, "if (!record.active) return {};") && !has(cursor, "std::vector<"),
            "focused cursor owner bounds reentrant submissions without heap state");
    const auto frame = read("src/ui/runtime_window_frame_results.cpp", 120);
    require(has(frame, "install_static_tree_for_record(record, rendered)") &&
                !has(frame, "host.static_tree") && !has(frame, "tree.hit_test("),
            "frame orchestration delegates static installation");
    const auto trees = read("src/ui/runtime_window_element_trees.cpp", 120);
    require(has(trees, "host.static_tree = {}") && has(trees, "host->static_tree = {}"),
            "explicit and rendered dynamic trees clear stale static state");
    require(has(read("packages/ui/mcpp.toml", 40), "../../src/ui/*.cpp"),
            "focused static owners must be included by their package");
    const auto manifest = read("tests/ui/mcpp.toml", 220);
    require(has(manifest, "window_record_static_tree_test.cpp") &&
                has(manifest, "window_record_static_lifetime_test.cpp") &&
                has(manifest, "window_record_static_cursor_lifetime_test.cpp"),
            "static routing and lifetime regressions must be registered");
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
