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
  return lines;
}
} // namespace

int main() {
  const std::string internal =
      read_source("src/platform/clipboard_wayland_internal.hpp");
  const std::string read =
      read_source("src/platform/clipboard_wayland_read.cpp");
  const std::string payload =
      read_source("src/platform/clipboard_wayland_read_payload.cpp");
  const std::string behavior =
      read_source("tests/platform/wayland_clipboard_read_lifecycle_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &internal, &read, &payload, &behavior, &xmake, &roadmap,
      &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* value : required) if (value->empty()) return 1;
  if (!contains(internal, "std::optional<std::string> read_text()") ||
      !contains(internal, "owned_selection_mutex_")) return 2;
  const auto stop = read.find("stop_dispatch_thread();");
  const auto lock = read.find("std::lock_guard display_lock", stop);
  const auto roundtrip = read.find("wl_display_roundtrip(", lock);
  const auto mime = read.find("preferred_text_mime_type()", roundtrip);
  const auto receive = read.find("read_offer_payload(*mime_type)", mime);
  const auto ownership = read.find("owned_source_ != nullptr", receive);
  const auto restart = read.find("start_dispatch_thread()", ownership);
  if (stop == std::string::npos || lock < stop || roundtrip < lock ||
      mime < roundtrip || receive < mime || ownership < receive ||
      restart < ownership) return 3;
  if (!contains(payload, "wl_data_offer_receive") ||
      !contains(payload, "std::chrono::seconds(3)") ||
      !contains(payload, "std::optional<std::string>{std::move(payload)}")) {
    return 4;
  }
  if (!contains(behavior, "owned before read") ||
      !contains(behavior, "external replacement") ||
      !contains(behavior, "wait_for_clipboard_selection_sent")) return 5;
  if (!contains(xmake,
                "target(\"wayland_clipboard_read_lifecycle_test\")") ||
      !contains(xmake,
                "target(\"phase_f_wayland_clipboard_read_structure_test\")")) {
    return 6;
  }
  if (line_count(internal) > 150 || line_count(read) > 70 ||
      line_count(payload) > 80 || line_count(behavior) > 80) return 7;
  constexpr const char* completion =
      "Phase F Step 566 makes Wayland selection reads dispatch-safe, "
      "processes replacement offers and payload transfer in one display "
      "transaction, and resumes source dispatch only when ownership remains. "
      "Step 567 Wayland clipboard MIME negotiation production behavior is "
      "next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* document : documents) {
    if (!contains(*document, completion)) return 8;
  }
  if (!contains(ledger_json,
                "\"phase_f_step_566_remaining_gap\": \"Step 567 Wayland")) {
    return 9;
  }
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 576 Win32")) {
    return 10;
  }
  return 0;
}
