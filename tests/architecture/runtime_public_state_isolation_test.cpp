#include "cgpui/ui/window_runtime.hpp"
#include "runtime_public_structure_checks.hpp"
#include "runtime_state_structure_checks.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <regex>
#include <stdexcept>
#include <string>

namespace {
std::string read(const std::filesystem::path& path) {
  std::ifstream input(path);
  if (!input) throw std::runtime_error("cannot read " + path.string());
  return {std::istreambuf_iterator<char>(input), {}};
}

void require(bool value, const std::string& message) {
  if (!value) throw std::runtime_error(message);
}

void check_public_header(const std::filesystem::path& header,
                         const std::filesystem::path& include_root) {
  const auto source = read(header);
  const std::regex quoted_include(R"re(#\s*include\s*"([^"]+)")re");
  for (auto match = std::sregex_iterator(source.begin(), source.end(), quoted_include);
       match != std::sregex_iterator(); ++match) {
    const std::filesystem::path name((*match)[1].str());
    auto target = (header.parent_path() / name).lexically_normal();
    if (!std::filesystem::is_regular_file(target)) {
      target = (include_root / name).lexically_normal();
    }
    const auto relative = target.lexically_relative(include_root);
    require(!relative.empty() && *relative.begin() != ".." &&
                std::filesystem::is_regular_file(target),
            "public header requires files outside installed include/: " + header.string());
  }

  const std::regex complete_state(
      R"(\b(?:class|struct)\s+(?:WindowRuntime::)?(?:RuntimeState|Runtime[A-Za-z]*State|RuntimeTask|RuntimeTimer|RuntimeTaskCompletion|RuntimeTaskDiagnostics|RuntimeAnimation|RegisteredView|WindowElementHost|OwnedRendererFactoryTag|StaticRenderInstallResult|TextPointerSelectionDrag|ActionDispatchTarget|ActionWindowScope)\s*\{)");
  require(!std::regex_search(source, complete_state),
          "complete runtime state leaked into installed headers: " + header.string());
  for (const char* private_storage : {"entity_stores_", "entity_count_", "globals_",
           "view_registry_", "window_runtime_records_", "tasks_mutex_",
           "task_completion_queue_", "native_additional_windows_",
           "cross_thread_entity_queue_", "owned_element_tree_"}) {
    require(source.find(private_storage) == std::string::npos,
            "public declarations/templates depend on runtime storage " +
                std::string(private_storage) + ": " + header.string());
  }
}
} // namespace

int main() {
  try {
    // Moving complete definitions to include/detail is not state isolation.
    require(sizeof(cgpui::WindowRuntime) == sizeof(void*),
            "WindowRuntime must contain only its owning opaque-state pointer");
    const auto root = std::filesystem::path(CGPUI_SOURCE_ROOT);
    runtime_public_structure::check(root);
    runtime_state_structure::check(root);
    const auto include_root = root / "include";
    for (const auto& entry : std::filesystem::recursive_directory_iterator(include_root)) {
      if (entry.is_regular_file() && entry.path().extension() == ".hpp") {
        check_public_header(entry.path(), include_root);
      }
    }
  } catch (const std::exception& error) {
    std::cerr << "public runtime state isolation: " << error.what() << '\n';
    return 1;
  }
  return 0;
}
