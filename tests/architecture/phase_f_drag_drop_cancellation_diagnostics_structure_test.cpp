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
  const std::string public_header =
      read_source("include/cgpui/platform/platform_diagnostics.hpp");
  const std::string runtime = read_source("src/ui/runtime_event_input.cpp");
  const std::string behavior = read_source(
      "tests/ui/window_runtime_drag_drop_cancellation_diagnostics_test.cpp");
  const std::string manifests = read_source("tests/ui/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{&public_header, &runtime, &behavior, &manifests};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(public_header, "bool cancelled = false;")) return 2;
  if (!contains(runtime,
                "const bool cancelled = std::holds_alternative<DragExited>(event)") ||
      !contains(runtime, ".succeeded = !cancelled") ||
      !contains(runtime, ".cancelled = cancelled")) return 3;
  if (!contains(behavior, "index < 64") ||
      !contains(behavior, "platform_diagnostics.size() != 32") ||
      !contains(behavior, "97 + index") ||
      !contains(behavior, "event.cancelled != cancellation") ||
      !contains(behavior, "event.succeeded == cancellation") ||
      !contains(behavior, "event.value_count != 1")) return 4;
  if (line_count(public_header) > 45 || line_count(runtime) > 170 ||
      line_count(behavior) > 75) return 5;
  if (!contains(manifests,
                "\"window_runtime_drag_drop_cancellation_diagnostics_test.cpp\"") ||
      !contains(manifests,
                "\"phase_f_drag_drop_cancellation_diagnostics_structure_test.cpp\"")) return 6;
  return 0;
}
