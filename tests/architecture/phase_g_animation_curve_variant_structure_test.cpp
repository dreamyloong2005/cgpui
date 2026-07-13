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
  const std::string public_header =
      read_source("include/cgpui/ui/animation_curve.hpp");
  const std::string source = read_source("src/ui/animation_curve.cpp");
  const std::string runtime_handles =
      read_source("include/cgpui/ui/runtime_handles.hpp");
  const std::string runtime_state =
      read_source("src/ui/runtime_animation_state.cpp");
  const std::string element_header =
      read_source("include/cgpui/ui/element_animation.hpp");
  const std::string element_state =
      read_source("src/ui/element_animation_state_store.cpp");
  const std::string sequence_state =
      read_source("src/ui/element_animation_sequence_state.cpp");
  const std::string style_header =
      read_source("include/cgpui/ui/style_animation.hpp");
  const std::string style_source = read_source("src/ui/style_tween.cpp");
  const std::string aggregate = read_source("include/cgpui/ui/ui.hpp");
  const std::string behavior =
      read_source("tests/animation/animation_curve_variant_test.cpp");
  const std::string ui_structure =
      read_source("tests/architecture/ui_source_structure_test.cpp");
  const std::string header_cleanliness =
      read_source("tests/header_cleanliness/ui_header_cleanliness.cpp");
  const std::string previous = read_source(
      "tests/architecture/phase_g_element_animation_sequence_structure_test.cpp");
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
      &public_header, &source, &runtime_handles, &runtime_state,
      &element_header, &element_state, &sequence_state, &style_header,
      &style_source, &aggregate, &behavior, &ui_structure,
      &header_cleanliness, &previous, &xmake, &pin, &vocabulary,
      &core_parity, &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* item : required) if (item->empty()) return 1;

  if (!contains(public_header, "struct AnimationSpring") ||
      !contains(public_header, "enum class AnimationCurveKind") ||
      !contains(public_header, "class AnimationCurve") ||
      !contains(public_header, "AnimationCurve bounce(") ||
      !contains(public_header, "AnimationCurve pulsating_between(") ||
      !contains(public_header, "AnimationCurve spring(") ||
      contains(public_header, "std::function") ||
      contains(public_header, "std::vector")) return 2;
  if (!contains(source, "spring_value(") ||
      !contains(source, "damping_ratio < 1.0F") ||
      !contains(source, "std::fabs(damping_ratio - 1.0F)") ||
      !contains(source, "AnimationCurveKind::ease_out_quint") ||
      !contains(source, "AnimationCurveKind::bounce") ||
      !contains(source, "AnimationCurveKind::pulsating")) return 3;
  if (!contains(runtime_handles, "AnimationCurve curve;") ||
      !contains(runtime_state, "animation->options.curve.value_at(") ||
      !contains(element_header, "AnimationCurve curve;") ||
      !contains(element_state, "options.curve.value_at(") ||
      !contains(sequence_state, "stage.animation.curve.value_at(") ||
      !contains(style_header, "AnimationCurve curve;") ||
      !contains(style_source, "curve.value_at(progress, easing)")) return 4;
  if (!contains(behavior, "test_legacy_and_pinned_tween_curves") ||
      !contains(behavior, "test_parameterized_spring_curve") ||
      !contains(behavior, "test_curve_integration_is_shared") ||
      !contains(behavior, "test_runtime_uses_curve_variant") ||
      !contains(behavior, "std::is_trivially_copyable_v") ||
      !contains(behavior, "sizeof(cgpui::AnimationCurve) <= 40")) return 5;
  if (!contains(aggregate, "cgpui/ui/animation_curve.hpp") ||
      !contains(ui_structure, "include/cgpui/ui/animation_curve.hpp") ||
      !contains(ui_structure, "src/ui/animation_curve.cpp") ||
      !contains(header_cleanliness, "cgpui/ui/animation_curve.hpp") ||
      !contains(xmake, "target(\"phase_g_animation_curve_variant_test\")") ||
      !contains(xmake,
                "target(\"phase_g_animation_curve_variant_structure_test\")") ||
      !contains(previous,
                "Step 646 spring and tween variant ")) {
    return 6;
  }
  if (!contains(pin, "5a823cf70ebb1d7a158c6a7ca455860cd9f6aed0") ||
      line_count(public_header) > 115 || line_count(source) > 110 ||
      line_count(runtime_handles) > 125 || line_count(runtime_state) > 100 ||
      line_count(element_header) > 140 || line_count(element_state) > 160 ||
      line_count(sequence_state) > 140 || line_count(style_header) > 30 ||
      line_count(style_source) > 80 || line_count(behavior) > 190) return 7;

  constexpr const char* completion =
      "Phase G Step 646 adds zero-allocation animation curve variants with "
      "pinned-upstream quadratic, ease-out-quint, bounce, and pulsating "
      "tween behavior plus parameterized under-, critical-, and over-damped "
      "springs shared by runtime, element, sequence, and style animation "
      "paths. Step 647 animation cancellation production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 8;
  }
  if (!contains(vocabulary, "`AnimationCurve`") ||
      !contains(core_parity, "parameterized spring curves") ||
      !contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 672 reproducible dependency setup\"")) return 9;
  return 0;
}
