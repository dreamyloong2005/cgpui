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
  const std::string base_header =
      read_source("include/cgpui/ui/element_animation.hpp");
  const std::string public_header =
      read_source("include/cgpui/ui/element_animation_sequence.hpp");
  const std::string aggregate = read_source("include/cgpui/ui/ui.hpp");
  const std::string store =
      read_source("src/ui/element_animation_state_store.cpp");
  const std::string state =
      read_source("src/ui/element_animation_state_internal.hpp");
  const std::string element = read_source("src/ui/element_animation.cpp");
  const std::string sequence =
      read_source("src/ui/element_animation_sequence.cpp");
  const std::string sequence_state =
      read_source("src/ui/element_animation_sequence_state.cpp");
  const std::string behavior =
      read_source("tests/animation/element_animation_sequence_test.cpp");
  const std::string ui_structure =
      read_source("tests/architecture/ui_source_structure_test.cpp");
  const std::string header_cleanliness =
      read_source("tests/header_cleanliness/ui_header_cleanliness.cpp");
  const std::string previous = read_source(
      "tests/architecture/phase_g_element_lifecycle_animation_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string pin = read_source("docs/gpui-upstream-pinned-revision.md");
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
      &base_header, &public_header, &aggregate, &store, &state, &element,
      &sequence, &sequence_state, &behavior, &ui_structure,
      &header_cleanliness, &previous, &xmake, &pin, &vocabulary, &core_parity,
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* item : required) if (item->empty()) return 1;

  if (!contains(public_header, "struct ElementAnimationStage") ||
      !contains(public_header, "ElementAnimationStage repeat() const") ||
      !contains(public_header, "AnyElement with_animations(") ||
      !contains(base_header, "std::size_t stage_index") ||
      !contains(base_header, "std::uint64_t iteration") ||
      !contains(base_header, "bool repeating")) return 2;
  if (!contains(state, "struct ElementAnimationRecord") ||
      !contains(state, "std::size_t stage_index") ||
      !contains(state, "bool complete") ||
      !contains(store, "normalized_animation_tick_interval") ||
      contains(store, "completed_iterations")) return 3;
  if (!contains(sequence, "ElementAnimationStage::repeat() const") ||
      !contains(sequence, "invalid_repeating_stage") ||
      !contains(sequence, "std::make_unique<AnimationElement>") ||
      !contains(sequence_state, "completed_iterations") ||
      !contains(sequence_state, "elapsed_ms %= duration_ms") ||
      !contains(sequence_state, "record->second.stage_index += 1") ||
      !contains(sequence_state, "sequence_complete")) return 4;
  if (!contains(element, "sequence_") ||
      !contains(element, "resolve(animation_key_, stages_)") ||
      !contains(behavior, "test_repeat_wraps_progress_and_never_advances") ||
      !contains(behavior, "test_chain_delivers_stage_final_before_advancing") ||
      !contains(behavior, "test_invalid_sequences_preserve_the_child") ||
      !contains(behavior, "std::vector<std::uint64_t>({16, 13, 4})")) {
    return 5;
  }
  if (!contains(aggregate, "cgpui/ui/element_animation_sequence.hpp") ||
      !contains(ui_structure, "include/cgpui/ui/element_animation_sequence.hpp") ||
      !contains(ui_structure, "src/ui/element_animation_sequence_state.cpp") ||
      !contains(header_cleanliness, "cgpui/ui/element_animation_sequence.hpp") ||
      !contains(xmake, "target(\"phase_g_element_animation_sequence_test\")") ||
      !contains(xmake, "target(\"phase_g_element_animation_sequence_structure_test\")") ||
      !contains(previous,
                "animation repeat and chaining production behavior is next.")) {
    return 6;
  }
  if (!contains(pin, "5a823cf70ebb1d7a158c6a7ca455860cd9f6aed0") ||
      line_count(base_header) > 140 || line_count(public_header) > 60 ||
      line_count(store) > 180 || line_count(state) > 80 ||
      line_count(element) > 120 || line_count(sequence) > 80 ||
      line_count(sequence_state) > 160 || line_count(behavior) > 230) return 7;

  constexpr const char* completion =
      "Phase G Step 645 adds pinned-upstream-compatible infinite element "
      "animation repetition and indexed one-shot chains, with iteration "
      "observability, final-stage value delivery before advancement, "
      "reconstructed-wrapper persistence, invalid-sequence rejection, and "
      "shared runtime frame wakeups. Step 646 spring and tween variant "
      "production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 8;
  }
  if (!contains(vocabulary, "`ElementAnimationStage`") ||
      !contains(core_parity, "indexed one-shot animation chains") ||
      !contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 654 SVG asset decode boundary production behavior\"")) return 9;
  return 0;
}
