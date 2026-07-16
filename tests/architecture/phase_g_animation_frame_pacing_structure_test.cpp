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
  const std::string header =
      read_source("include/cgpui/ui/animation_frame_pacing.hpp");
  const std::string diagnostics =
      read_source("include/cgpui/ui/runtime_diagnostics.hpp");
  const std::string private_header =
      read_source("src/ui/runtime_animation_frame_pacing_internal.hpp");
  const std::string pacing =
      read_source("src/ui/runtime_animation_frame_pacing.cpp");
  const std::string state =
      read_source("src/ui/runtime_animation_state_internal.hpp");
  const std::string start = read_source("src/ui/runtime_animation_start.cpp");
  const std::string cancel =
      read_source("src/ui/runtime_animation_cancellation.cpp");
  const std::string tick = read_source("src/ui/runtime_animation_tick.cpp");
  const std::string element =
      read_source("src/ui/runtime_element_animation.cpp");
  const std::string run = read_source("src/ui/runtime_run.cpp");
  const std::string diagnostic_source =
      read_source("src/ui/runtime_diagnostic_snapshot.cpp");
  const std::string aggregate = read_source("include/cgpui/ui/ui.hpp");
  const std::string behavior =
      read_source("tests/animation/animation_frame_pacing_test.cpp");
  const std::string lifecycle =
      read_source("tests/animation/element_lifecycle_animation_test.cpp");
  const std::string sequence =
      read_source("tests/animation/element_animation_sequence_test.cpp");
  const std::string ui_structure =
      read_source("tests/architecture/ui_source_structure_test.cpp");
  const std::string cleanliness =
      read_source("tests/header_cleanliness/ui_header_cleanliness.cpp");
  const std::string previous = read_source(
      "tests/architecture/phase_g_animation_cancellation_structure_test.cpp");
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
      &header, &diagnostics, &private_header, &pacing, &state, &start,
      &cancel, &tick, &element, &run, &diagnostic_source, &aggregate,
      &behavior, &lifecycle, &sequence, &ui_structure, &cleanliness, &previous,
      &xmake, &vocabulary, &core, &roadmap, &ledger_md, &ledger_json,
      &task_plan, &findings};
  for (const auto* item : required) if (item->empty()) return 1;

  if (!contains(header, "struct AnimationFramePacingSnapshot") ||
      !contains(header, "pending_animation_count") ||
      !contains(header, "coalesced_request_count") ||
      !contains(header, "late_frame_count") ||
      !contains(diagnostics, "animation_frame_pacing")) return 2;
  if (!contains(private_header, "schedule_animation_frame_wakeup") ||
      !contains(private_header, "animation_frame_timer_id_") ||
      !contains(private_header, "element_animation_frame_deadline_ms_") ||
      !contains(state, "next_tick_ms") ||
      !contains(state, "frame_pending") ||
      !contains(state, "due_for_frame") || contains(state, "TimerId timer_id")) {
    return 3;
  }
  if (!contains(pacing, "add_interval(") ||
      !contains(pacing, "schedule_timer(") ||
      !contains(pacing, "due_for_frame = true") ||
      !contains(pacing, "while (true)") ||
      !contains(pacing, "request_render()") ||
      contains(pacing, "schedule_repeating_timer(") ||
      contains(pacing, "std::vector<")) return 4;
  if (!contains(start, "schedule_animation_frame_wakeup(true)") ||
      !contains(cancel, "animation->frame_pending = false") ||
      !contains(tick, "completed->frame_pending = false") ||
      !contains(element, "request_element_animation_frame(") ||
      !contains(run, "schedule_animation_frame_wakeup(false)") ||
      !contains(diagnostic_source, "animation_frame_pacing_snapshot()")) {
    return 5;
  }
  if (!contains(behavior, "next_frame_deadline_ms != 32") ||
      !contains(behavior, "last_scheduled_delay_ms != 12") ||
      !contains(behavior, "coalesced_request_count != 2") ||
      !contains(behavior, "state.mutation_second.cancel()") ||
      !contains(behavior, "state.spawned = runtime.start_animation(") ||
      !contains(lifecycle, "std::vector<std::uint64_t>({16, 14})") ||
      !contains(sequence, "std::vector<std::uint64_t>({16, 13, 4})")) {
    return 6;
  }
  if (!contains(aggregate, "cgpui/ui/animation_frame_pacing.hpp") ||
      !contains(ui_structure, "runtime_animation_frame_pacing.cpp") ||
      !contains(cleanliness, "cgpui/ui/animation_frame_pacing.hpp") ||
      !contains(xmake, "target(\"phase_g_animation_frame_pacing_test\")") ||
      !contains(xmake,
                "target(\"phase_g_animation_frame_pacing_structure_test\")") ||
      !contains(previous, "Step 648 animation frame pacing") ||
      !contains(previous, "production behavior")) {
    return 7;
  }
  if (line_count(header) > 30 || line_count(private_header) > 20 ||
      line_count(pacing) > 150 || line_count(state) > 25 ||
      line_count(start) > 60 || line_count(cancel) > 60 ||
      line_count(tick) > 60 || line_count(element) > 50 ||
      line_count(run) > 140 || line_count(behavior) > 190) return 8;

  constexpr const char* completion =
      "Phase G Step 648 coalesces ordinary and element animations onto one "
      "deadline-driven frame timer, preserves cadence across late frames, "
      "delivers mutation-safe due callbacks without a per-frame allocation, "
      "tears down cancelled participation, and reports pending, scheduled, "
      "delivered, coalesced, and late-frame diagnostics. Step 649 style "
      "interpolation production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 9;
  }
  if (!contains(vocabulary, "`AnimationFramePacingSnapshot`") ||
      !contains(core, "deadline-driven frame timer") ||
      !contains(ledger_json,
                "\"phase_f_current_handoff\": \"Phase J Step 799 re-run upstream extractor against the pinned revision\"")) return 10;
  return 0;
}
