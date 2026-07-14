#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {
std::string read_source(const char* path) {
  const char* root = std::getenv("CGPUI_SOURCE_ROOT");
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
  const std::string public_header = read_source("include/cgpui/platform/platform_open_url.hpp");
  const std::string application = read_source("include/cgpui/platform/platform_application.hpp");
  const std::string empty = read_source("src/platform/empty.cpp");
  const std::string wayland = read_source("src/platform/linux/wayland_application_services.cpp");
  const std::string header = read_source("src/platform/win32/win32_open_url_internal.hpp");
  const std::string source = read_source("src/platform/win32/win32_open_url.cpp");
  const std::string win32_application = read_source("src/platform/win32/win32_application.cpp");
  const std::string behavior = read_source("tests/platform/win32_open_url_plan_test.cpp");
  const std::string previous = read_source("tests/architecture/phase_f_native_message_dialog_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source("docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md = read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{&public_header, &application, &empty, &wayland,
      &header, &source, &win32_application, &behavior, &previous, &xmake,
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(public_header, "struct PlatformOpenUrlResult") ||
      !contains(public_header, "bool opened = false") ||
      !contains(application, "PlatformOpenUrlResult open_url(std::string url)")) return 2;
  if (!contains(empty, ".backend = \"unsupported\"") ||
      !contains(wayland, ".backend = \"wayland\"") ||
      !contains(wayland, "open URL unsupported by wayland")) return 3;
  if (!contains(header, "win32_open_url_plan(") ||
      !contains(source, "url.find('\\0')") ||
      !contains(source, "ShellExecuteW(") ||
      !contains(source, "result > 32") ||
      !contains(win32_application, "return win32_open_url(url)")) return 4;
  if (!contains(behavior, "https://example.com/a?b=c#d") ||
      !contains(behavior, "std::string_view{\"a\\0b\", 3}") ||
      !contains(behavior, "win32_shell_execute_succeeded(33)")) return 5;
  if (line_count(public_header) > 20 || line_count(header) > 30 ||
      line_count(source) > 55 || line_count(behavior) > 25 ||
      line_count(wayland) > 50 || line_count(win32_application) > 220) return 6;
  if (!contains(previous, "Phase F Step 590") ||
      !contains(xmake, "target(\"win32_open_url_plan_test\")") ||
      !contains(xmake, "target(\"phase_f_native_open_url_structure_test\")")) return 7;
  constexpr const char* completion =
      "Phase F Step 591 adds a result-bearing native open-URL platform service, launches valid Win32 URLs through focused ShellExecuteW handling, rejects empty/NUL URLs, classifies native return codes, and preserves explicit unsupported Wayland/default results. Step 592 quit/reopen lifecycle production behavior is next.";
  const std::string* documents[]{&roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) if (!contains(*document, completion)) return 8;
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 674 WSL full-debug verification")) return 9;
  return 0;
}
