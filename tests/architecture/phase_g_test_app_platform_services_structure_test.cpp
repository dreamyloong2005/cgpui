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
  for (const char value : text) lines += value == '\n' ? 1U : 0U;
  return lines + (!text.empty() && text.back() != '\n' ? 1U : 0U);
}

} // namespace

int main() {
  const std::string header = read_source("include/cgpui/ui/test_app.hpp");
  const std::string services_header =
      read_source("include/cgpui/ui/test_app_services.hpp");
  const std::string internal = read_source("src/ui/test_app_internal.hpp");
  const std::string facade = read_source("src/ui/test_app.cpp");
  const std::string platform = read_source("src/ui/test_app_platform.cpp");
  const std::string platform_services =
      read_source("src/ui/test_app_platform_services.cpp");
  const std::string services = read_source("src/ui/test_app_services.cpp");
  const std::string behavior =
      read_source("tests/ui/test_app_platform_services_test.cpp");
  const std::string previous = read_source(
      "tests/architecture/phase_g_test_app_rendering_control_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  const std::string core = read_source("docs/gpui-core-api-parity.md");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &header, &services_header, &internal, &facade, &platform,
      &platform_services, &services, &behavior, &previous, &xmake, &vocabulary,
      &core, &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* item : required) if (item->empty()) return 1;

  if (!contains(header, "#include \"cgpui/ui/test_app_services.hpp\"") ||
      !contains(header, "simulate_path_prompt_response(") ||
      !contains(header, "platform_service_snapshot() const") ||
      !contains(services_header, "struct TestPlatformServiceSnapshot")) {
    return 2;
  }
  if (!contains(internal, "path_responses_") ||
      !contains(internal, "prompt_responses_") ||
      !contains(internal, "MemoryClipboard clipboard") ||
      !contains(facade, "runtime.set_clipboard(&clipboard)")) return 3;
  if (!contains(platform_services, "install_native_menu(") ||
      !contains(platform_services, "no simulated path prompt response") ||
      !contains(platform_services, "no simulated prompt response") ||
      !contains(platform_services, "service_snapshot_.opened_url") ||
      !contains(platform_services, "PlatformReopenResult")) return 4;
  if (!contains(services, "TestApp::write_to_clipboard(") ||
      !contains(services, "TestApp::simulate_prompt_answer(") ||
      !contains(behavior, "alpha.txt") ||
      !contains(behavior, "std::nullopt") ||
      !contains(behavior, "platform_service_snapshot()")) return 5;
  if (!contains(previous, "Step 669 Windows and Linux release") ||
      !contains(xmake, "target(\"phase_g_test_app_platform_services_test\")") ||
      !contains(xmake,
                "target(\"phase_g_test_app_platform_services_structure_test\")")) {
    return 6;
  }
  if (line_count(header) > 125 || line_count(services_header) > 35 ||
      line_count(internal) > 140 || line_count(facade) > 135 ||
      line_count(platform) > 75 || line_count(platform_services) > 120 ||
      line_count(services) > 75 || line_count(behavior) > 155) return 7;

  constexpr const char* completion =
      "Phase G Step 664 adds deterministic TestApp platform service fakes for "
      "isolated clipboard state, FIFO path and prompt responses with "
      "cancellation and fail-closed empty queues, supported menu/open URL/"
      "reopen behavior, opened-URL inspection, and service call snapshots. "
      "Step 665 GPUI-style test runner ergonomics production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 8;
  }
  if (!contains(vocabulary, "`TestPlatformServiceSnapshot`") ||
      !contains(core, "Deterministic `TestApp` platform service fakes") ||
      !contains(ledger_json, "\"phase_g_step_664_sources\"") ||
      !contains(ledger_json, "\"phase_f_current_handoff\": \"Step 669 Windows and Linux release build and packaging coverage\"")) return 9;
  return 0;
}
