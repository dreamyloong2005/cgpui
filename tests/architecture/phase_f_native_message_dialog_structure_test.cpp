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
  const std::string public_header = read_source("include/cgpui/platform/platform_message_dialog.hpp");
  const std::string application = read_source("include/cgpui/platform/platform_application.hpp");
  const std::string empty = read_source("src/platform/empty.cpp");
  const std::string wayland = read_source("src/platform/linux/wayland_application_services.cpp");
  const std::string win32_header = read_source("src/platform/win32/win32_message_dialog_internal.hpp");
  const std::string win32_source = read_source("src/platform/win32/win32_message_dialog.cpp");
  const std::string win32_application = read_source("src/platform/win32/win32_application.cpp");
  const std::string behavior = read_source("tests/platform/win32_message_dialog_plan_test.cpp");
  const std::string previous = read_source("tests/architecture/phase_f_native_directory_picker_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source("docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md = read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{&public_header, &application, &empty, &wayland,
      &win32_header, &win32_source, &win32_application, &behavior, &previous,
      &xmake, &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(public_header, "enum class NativeMessageDialogKind") ||
      !contains(public_header, "enum class NativeMessageDialogButtons") ||
      !contains(public_header, "enum class NativeMessageDialogResponse") ||
      !contains(public_header, "struct NativeMessageDialogResult")) return 2;
  if (!contains(application, "show_native_message_dialog(") ||
      !contains(empty, ".backend = \"unsupported\"") ||
      !contains(wayland, ".backend = \"wayland\"") ||
      !contains(wayland, "message dialog unsupported by wayland")) return 3;
  if (!contains(win32_header, "struct Win32MessageDialogPlan") ||
      !contains(win32_source, "MB_ICONQUESTION") ||
      !contains(win32_source, "MB_YESNO") ||
      !contains(win32_source, "MessageBoxW(") ||
      !contains(win32_source, "NativeMessageDialogResponse::cancel") ||
      !contains(win32_application, "show_win32_native_message_dialog(options)")) return 4;
  if (!contains(behavior, "MB_ICONQUESTION") ||
      !contains(behavior, "NativeMessageDialogResponse::yes") ||
      !contains(behavior, "NativeMessageDialogResponse::none")) return 5;
  if (line_count(public_header) > 40 || line_count(win32_header) > 35 ||
      line_count(win32_source) > 75 || line_count(behavior) > 35 ||
      line_count(wayland) > 40 || line_count(win32_application) > 220) return 6;
  if (!contains(previous, "Phase F Step 589") ||
      !contains(xmake, "target(\"win32_message_dialog_plan_test\")") ||
      !contains(xmake, "target(\"phase_f_native_message_dialog_structure_test\")")) return 7;
  constexpr const char* completion =
      "Phase F Step 590 adds a public native message-dialog platform service, maps Win32 kinds/buttons/responses through a focused MessageBoxW leaf, and reports explicit unsupported results on Wayland and default backends. Step 591 native open-URL production behavior is next.";
  const std::string* documents[]{&roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) if (!contains(*document, completion)) return 8;
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 658 official image/GIF examples and asset closeout")) return 9;
  return 0;
}
