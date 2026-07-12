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
  const std::string public_header = read_source("include/cgpui/platform/platform_lifecycle.hpp");
  const std::string application_header = read_source("include/cgpui/platform/platform_application.hpp");
  const std::string base = read_source("src/platform/empty.cpp");
  const std::string win32 = read_source("src/platform/win32/win32_application.cpp");
  const std::string wayland = read_source("src/platform/linux/wayland_application_lifecycle.cpp");
  const std::string behavior = read_source("tests/platform/platform_reopen_lifecycle_test.cpp");
  const std::string previous = read_source("tests/architecture/phase_f_native_open_url_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source("docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md = read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{&public_header, &application_header, &base,
      &win32, &wayland, &behavior, &previous, &xmake, &roadmap, &ledger_md,
      &ledger_json, &task_plan, &findings};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(public_header, "using PlatformReopenCallback") ||
      !contains(public_header, "struct PlatformReopenResult") ||
      !contains(application_header, "set_reopen_callback(") ||
      !contains(application_header, "dispatch_reopen(std::string backend)") ||
      !contains(application_header, "PlatformReopenCallback reopen_callback_")) return 2;
  if (!contains(base, "reopen callback not registered") ||
      !contains(base, "reopen_callback_();") ||
      !contains(win32, "return dispatch_reopen(\"win32\")") ||
      !contains(wayland, "return dispatch_reopen(\"wayland\")")) return 3;
  if (!contains(win32, "void quit() override") ||
      !contains(win32, "PostQuitMessage(0)") ||
      !contains(wayland, "void WaylandApplication::quit()") ||
      !contains(wayland, "running_.store(false)") ||
      !contains(wayland, "request_wakeup()")) return 4;
  if (!contains(behavior, "reopen callback not registered") ||
      !contains(behavior, "set_reopen_callback([&]") ||
      !contains(behavior, "app.set_reopen_callback({})") ||
      !contains(behavior, "app.quit_count != 2")) return 5;
  if (line_count(public_header) > 25 || line_count(application_header) > 70 ||
      line_count(behavior) > 50 || line_count(wayland) > 45 ||
      line_count(win32) > 220) return 6;
  if (!contains(previous, "Phase F Step 591") ||
      !contains(xmake, "target(\"platform_reopen_lifecycle_test\")") ||
      !contains(xmake, "target(\"phase_f_quit_reopen_lifecycle_structure_test\")")) return 7;
  constexpr const char* completion =
      "Phase F Step 592 adds explicit reopen callback/result lifecycle behavior shared by Win32 and Wayland backends, reports missing callbacks without dispatch, supports callback replacement/clearing, and preserves existing platform quit paths. Step 593 platform-service result and unsupported-policy production behavior is next.";
  const std::string* documents[]{&roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) if (!contains(*document, completion)) return 8;
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 647 animation cancellation production behavior")) return 9;
  return 0;
}
