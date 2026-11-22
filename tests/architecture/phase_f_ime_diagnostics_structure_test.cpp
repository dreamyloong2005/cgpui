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
  const std::string runtime = read_source("src/ui/runtime_event_text.cpp");
  const std::string behavior =
      read_source("tests/ui/window_runtime_ime_diagnostics_test.cpp");
  const std::string manifests = read_source("tests/ui/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{&public_header, &runtime, &behavior, &manifests};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(public_header, "bool cancelled = false;")) return 2;
  const char* operations[]{"composition-update", "composition-commit",
      "composition-cancel", "delete-surrounding"};
  for (const char* operation : operations) {
    if (!contains(runtime, operation) || !contains(behavior, operation)) return 3;
  }
  if (!contains(runtime, ".succeeded = !cancelled") ||
      !contains(runtime, ".cancelled = cancelled") ||
      !contains(runtime, "const bool deleted = model->delete_surrounding_text(") ||
      !contains(runtime, ".succeeded = deleted")) return 4;
  if (!contains(behavior, "index < 64") ||
      !contains(behavior, "platform_diagnostics.size() != 32") ||
      !contains(behavior, "97 + index") ||
      !contains(behavior, "event.cancelled != cancellation")) return 5;
  if (line_count(public_header) > 45 || line_count(runtime) > 90 ||
      line_count(behavior) > 135) return 6;
  if (!contains(manifests, "\"window_runtime_ime_diagnostics_test.cpp\"") ||
      !contains(manifests,
                "\"phase_f_ime_diagnostics_structure_test.cpp\"")) return 7;
  return 0;
}
