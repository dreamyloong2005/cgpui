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
} // namespace

int main() {
  const std::string open = read_source("tests/architecture/phase_f_win32_file_open_dialog_structure_test.cpp");
  const std::string save = read_source("tests/architecture/phase_f_win32_file_save_dialog_structure_test.cpp");
  const std::string directory = read_source("tests/architecture/phase_f_native_directory_picker_structure_test.cpp");
  const std::string message = read_source("tests/architecture/phase_f_native_message_dialog_structure_test.cpp");
  const std::string url = read_source("tests/architecture/phase_f_native_open_url_structure_test.cpp");
  const std::string reopen = read_source("tests/architecture/phase_f_quit_reopen_lifecycle_structure_test.cpp");
  const std::string policy = read_source("tests/architecture/phase_f_platform_service_result_policy_structure_test.cpp");
  const std::string wayland = read_source("src/platform/linux/wayland_application_services.cpp");
  const std::string empty = read_source("src/platform/empty.cpp");
  const std::string result_source = read_source("src/ui/runtime_platform_service_system_results.cpp");
  const std::string diagnostics = read_source("src/ui/runtime_platform_services.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source("docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md = read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{&open, &save, &directory, &message, &url,
      &reopen, &policy, &wayland, &empty, &result_source, &diagnostics, &xmake,
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(open, "Phase F Step 587") ||
      !contains(save, "Phase F Step 588") ||
      !contains(directory, "Phase F Step 589") ||
      !contains(message, "Phase F Step 590") ||
      !contains(url, "Phase F Step 591") ||
      !contains(reopen, "Phase F Step 592") ||
      !contains(policy, "Phase F Step 593")) return 2;
  if (!contains(wayland, "message dialog unsupported by wayland") ||
      !contains(wayland, "open URL unsupported by wayland") ||
      !contains(empty, "NativeFileDialogResult{") ||
      !contains(empty, ".backend = \"unsupported\"") ||
      !contains(empty, "message dialog unsupported") ||
      !contains(empty, "open URL unsupported") ||
      !contains(empty, "reopen unsupported")) return 3;
  if (!contains(result_source, "ErrorCode::unsupported_platform") ||
      !contains(result_source, "native_message_dialog_result_") ||
      !contains(result_source, "open_url_result_") ||
      !contains(result_source, "reopen_result_") ||
      !contains(diagnostics, "platform_diagnostic_limit = 32")) return 4;
  const char* targets[]{"phase_f_win32_file_open_dialog_structure_test",
      "phase_f_win32_file_save_dialog_structure_test",
      "phase_f_native_directory_picker_structure_test",
      "phase_f_native_message_dialog_structure_test",
      "phase_f_native_open_url_structure_test",
      "phase_f_quit_reopen_lifecycle_structure_test",
      "phase_f_platform_service_result_policy_structure_test",
      "phase_f_platform_services_closeout_structure_test"};
  for (const char* target : targets) if (!contains(xmake, target)) return 5;
  constexpr const char* completion =
      "Phase F Step 594 audits and closes the Steps 587-593 dialogs and platform-services band, freezing Win32 file/directory/message/URL behavior, shared reopen lifecycle, Runtime/AppContext Result policy, bounded diagnostics, and explicit Wayland/default unsupported behavior. Step 595 multi-window event-loop ownership production behavior is next.";
  const std::string* documents[]{&roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) if (!contains(*document, completion)) return 6;
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 641 cross-thread entity access production behavior")) return 7;
  return 0;
}
