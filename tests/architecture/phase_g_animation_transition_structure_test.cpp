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

std::size_t count_occurrences(const std::string& text, const char* value) {
  std::size_t count = 0;
  std::size_t offset = 0;
  while ((offset = text.find(value, offset)) != std::string::npos) {
    ++count;
    offset += std::string(value).size();
  }
  return count;
}

std::size_t line_count(const std::string& text) {
  std::size_t lines = 0;
  for (const char value : text) lines += value == '\n' ? 1U : 0U;
  return lines + (!text.empty() && text.back() != '\n' ? 1U : 0U);
}

} // namespace

int main() {
  const std::string public_header =
      read_source("include/cgpui/ui/animation_transition.hpp");
  const std::string aggregate = read_source("include/cgpui/ui/ui.hpp");
  const std::string source = read_source("src/ui/animation_transition.cpp");
  const std::string animation_start =
      read_source("src/ui/runtime_animation_start.cpp");
  const std::string runtime_header =
      read_source("include/cgpui/ui/window_runtime.hpp");
  const std::string runtime_internal =
      read_source("src/ui/window_runtime_internal.hpp");
  const std::string behavior =
      read_source("tests/animation/runtime_animation_transition_test.cpp");
  const std::string ui_structure =
      read_source("tests/architecture/ui_source_structure_test.cpp");
  const std::string header_cleanliness =
      read_source("tests/header_cleanliness/ui_header_cleanliness.cpp");
  const std::string previous =
      read_source("tests/api_parity/phase_g_async_runtime_closeout_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  const std::string core_parity = read_source("docs/gpui-core-api-parity.md");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &public_header, &aggregate, &source, &animation_start, &runtime_header,
      &runtime_internal, &behavior, &ui_structure, &header_cleanliness,
      &previous, &xmake, &vocabulary, &core_parity, &roadmap, &ledger_md,
      &ledger_json, &task_plan, &findings};
  for (const auto* item : required) if (item->empty()) return 1;

  if (!contains(public_header, "struct AnimationTransition") ||
      !contains(public_header, "struct AnimationTransitionSnapshot") ||
      !contains(public_header, "class AnimationTransitionHandle") ||
      !contains(public_header, "AnimationTransitionCallback") ||
      !contains(public_header, "std::optional<AnimationTransitionSnapshot>") ||
      count_occurrences(public_header, "start_animation_transition(") != 6) {
    return 2;
  }
  if (!contains(source, "wrap_transition_callback(") ||
      !contains(source, "transition.value_at(animation.eased_progress)") ||
      count_occurrences(source, "start_animation_transition(") != 3 ||
      contains(source, "RuntimeAnimation") ||
      contains(source, "animations_")) return 3;
  if (!contains(animation_start, "options.duration_ms == 0") ||
      !contains(animation_start, "immediate_callback(context(), *snapshot)") ||
      !contains(animation_start, "schedule_animation_frame_wakeup(true)") ||
      contains(animation_start, "schedule_repeating_timer(")) return 4;
  if (!contains(behavior, "cgpui/prelude.hpp") ||
      !contains(behavior, "quarter->animation.eased_progress") ||
      !contains(behavior, "near(quarter->value, 14.375F)") ||
      !contains(behavior, "context.async_context()") ||
      !contains(behavior, "immediate_transitions_match") ||
      !contains(behavior, "immediate_callback_released") ||
      !contains(behavior, "invalid_callback_rejected") ||
      !contains(behavior, "callbacks_on_runtime_thread")) return 5;
  if (!contains(aggregate, "cgpui/ui/animation_transition.hpp") ||
      !contains(ui_structure, "include/cgpui/ui/animation_transition.hpp") ||
      !contains(ui_structure, "src/ui/animation_transition.cpp") ||
      !contains(header_cleanliness, "cgpui/ui/animation_transition.hpp") ||
      !contains(xmake, "target(\"phase_g_animation_transition_test\")") ||
      !contains(xmake,
                "target(\"phase_g_animation_transition_structure_test\")") ||
      !contains(previous, "Step 643 animation transition \"")) return 6;
  if (line_count(public_header) > 100 || line_count(source) > 120 ||
      line_count(animation_start) > 80 || line_count(behavior) > 190 ||
      line_count(runtime_header) > 260 || line_count(runtime_internal) > 260) {
    return 7;
  }

  constexpr const char* completion =
      "Phase G Step 643 adds typed scalar animation transitions over existing "
      "runtime, runtime-context, and async-context scheduling, with eased "
      "from/to value snapshots, runtime-thread callbacks, zero-duration final "
      "delivery, invalid-callback rejection, and handle observability. Step "
      "644 element lifecycle animation production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 8;
  }
  if (!contains(vocabulary, "`AnimationTransition`") ||
      !contains(core_parity, "typed scalar animation transitions") ||
      !contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 659 GPUI-style app and window test setup production behavior\"")) return 9;
  return 0;
}
