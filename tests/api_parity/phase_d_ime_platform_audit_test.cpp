#include <array>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string source_root() {
  if (const char* root = std::getenv("CGPUI_SOURCE_ROOT"); root != nullptr) {
    return root;
  }
  return ".";
}

std::string read_source(const char* path) {
  const std::string root = source_root();
  std::ifstream source(root + "/" + path);
  if (!source) {
    source.open(path);
  }
  if (!source) {
    return {};
  }

  return std::string{
      std::istreambuf_iterator<char>(source),
      std::istreambuf_iterator<char>()};
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

} // namespace

int main() {
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string window_header =
      read_source("include/cgpui/core/window.hpp");
  const std::string event_text = read_source("include/cgpui/core/event_text.hpp");
  const std::string event_text_source = read_source("src/core/event_text.cpp");
  const std::string wayland_requests =
      read_source("src/platform/linux/wayland_text_input_requests.cpp");
  const std::string wayland_events =
      read_source("src/platform/linux/wayland_text_input_events.cpp");
  const std::string wayland_window_text =
      read_source("src/platform/linux/wayland_window_text_events.cpp");
  const std::string win32_ime =
      read_source("src/platform/win32/win32_window_ime.cpp");
  const std::string win32_ime_placement =
      read_source("src/platform/win32/win32_window_ime_placement.cpp");
  const std::string wayland_keyboard_test =
      read_source("tests/platform/wayland_keyboard_test.cpp");
  const std::string win32_text_test =
      read_source("tests/platform/win32_text_input_test.cpp");
  const std::string runtime_text_test =
      read_source("tests/ui/window_runtime_text_test.cpp");
  const std::string structure_test =
      read_source("tests/architecture/platform_source_structure_test.cpp");

  if (xmake.empty() || roadmap.empty() || ledger.empty() ||
      task_plan.empty() || findings.empty() || window_header.empty() ||
      event_text.empty() || event_text_source.empty() ||
      wayland_requests.empty() || wayland_events.empty() ||
      wayland_window_text.empty() || win32_ime.empty() ||
      win32_ime_placement.empty() ||
      wayland_keyboard_test.empty() || win32_text_test.empty() ||
      runtime_text_test.empty() || structure_test.empty()) {
    return 1;
  }

  if (!contains(xmake, "target(\"phase_d_ime_platform_audit_test\")") ||
      !contains(
          xmake,
          "tests/api_parity/phase_d_ime_platform_audit_test.cpp")) {
    return 2;
  }

  if (!contains(roadmap,
                "- [x] Steps 427-434: Complete IME on active targets") ||
      contains(roadmap, "- [ ] Steps 427-434") ||
      !contains(roadmap,
                "Step 434 closes the active-target IME platform band") ||
      !contains(roadmap, "Steps 435-442: Add rich text runs")) {
    return 3;
  }

  if (!contains(task_plan, "Step 434 closes the active-target IME band") ||
      !contains(findings, "Step 434 is an audit-only closeout")) {
    return 4;
  }

  constexpr std::array ledger_evidence{
      "Step 434 closes the active-target IME platform band",
      "phase_d_ime_platform_audit_test",
      "ImeTextInputPlacement::candidate_rect",
      "GCS_COMPSTR",
      "GCS_RESULTSTR",
      "last accepted `done(serial)`",
      "production candidate UI policy",
      "deeper Win32 TSF integration",
  };
  for (std::size_t index = 0; index < ledger_evidence.size(); ++index) {
    if (!contains(ledger, ledger_evidence[index])) {
      return 10 + static_cast<int>(index);
    }
  }

  constexpr std::array public_evidence{
      "std::optional<Rect> candidate_rect",
      "std::string surrounding_text",
      "std::size_t selection_anchor",
      "std::uint32_t content_hint",
      "std::uint32_t content_purpose",
      "std::uint32_t serial = 0",
      "preedit_cursor_begin",
      "preedit_styles",
      "struct ImeDeleteSurroundingText",
  };
  const std::array public_sources{
      window_header,
      window_header,
      window_header,
      window_header,
      window_header,
      event_text,
      event_text,
      event_text,
      event_text,
  };
  for (std::size_t index = 0; index < public_evidence.size(); ++index) {
    if (!contains(public_sources[index], public_evidence[index])) {
      return 30 + static_cast<int>(index);
    }
  }

  constexpr std::array source_evidence{
      "append_ime_default_preedit_style",
      "placement->surrounding_text",
      "placement->content_hint",
      "placement->candidate_rect",
      "preedit.cursor_begin",
      "text_input_done_serial_is_stale",
      "last_done_serial_ = serial",
      "append_ime_default_preedit_style",
      "ImeDeleteSurroundingText",
      "ImmSetCandidateWindow",
      "GCS_COMPSTR",
      "GCS_RESULTSTR",
      "ImeCompositionPhase::cancel",
  };
  const std::array source_files{
      event_text_source,
      wayland_requests,
      wayland_requests,
      wayland_requests,
      wayland_events,
      wayland_events,
      wayland_events,
      wayland_window_text,
      wayland_window_text,
      win32_ime_placement,
      win32_ime,
      win32_ime,
      win32_ime,
  };
  for (std::size_t index = 0; index < source_evidence.size(); ++index) {
    if (!contains(source_files[index], source_evidence[index]) ||
        !contains(structure_test, source_evidence[index])) {
      return 50 + static_cast<int>(index);
    }
  }

  constexpr std::array behavior_evidence{
      "request_text_input_preedit_with_serial",
      "request_text_input_commit_with_serial",
      "ime_stale_commit_received",
      "candidate_rect",
      "WM_IME_ENDCOMPOSITION",
      "test_runtime_applies_focused_text_ime_rect_to_platform_window",
      "test_runtime_routes_ime_delete_surrounding_to_focused_text_model",
  };
  const std::array behavior_sources{
      wayland_keyboard_test,
      wayland_keyboard_test,
      wayland_keyboard_test,
      wayland_keyboard_test,
      win32_text_test,
      runtime_text_test,
      runtime_text_test,
  };
  for (std::size_t index = 0; index < behavior_evidence.size(); ++index) {
    if (!contains(behavior_sources[index], behavior_evidence[index])) {
      return 80 + static_cast<int>(index);
    }
  }

  return 0;
}
