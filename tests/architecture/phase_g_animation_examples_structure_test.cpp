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
  const std::string animation = read_source(
      "examples/api_parity/public_animation_example/main.cpp");
  const std::string opacity = read_source(
      "examples/api_parity/public_opacity_example/main.cpp");
  const std::string behavior = read_source(
      "tests/api_parity/phase_g_official_animation_examples_test.cpp");
  const std::string previous = read_source(
      "tests/architecture/phase_g_style_interpolation_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string pin = read_source("docs/gpui-upstream-pinned-revision.md");
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
      &animation, &opacity, &behavior, &previous, &xmake, &pin, &vocabulary,
      &core, &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* item : required) if (item->empty()) return 1;

  if (!contains(animation, "#include \"cgpui/prelude.hpp\"") ||
      !contains(animation, "std::numbers::pi_v<float>") ||
      !contains(animation, "cgpui::with_animations(") ||
      !contains(animation, "AnimationCurve::bounce(") ||
      !contains(animation, ".repeat()") ||
      !contains(animation, "CGPUI_RUN_PUBLIC_ANIMATION_EXAMPLE") ||
      !contains(animation, "static_assert(cgpui::Render<")) return 2;
  if (!contains(opacity, "#include \"cgpui/prelude.hpp\"") ||
      !contains(opacity, "context.register_action(") ||
      !contains(opacity, "transition_.active()") ||
      !contains(opacity, "transition_.cancel()") ||
      !contains(opacity, "start_animation_transition(") ||
      !contains(opacity, "animation_context.request_render()") ||
      !contains(opacity, "CGPUI_RUN_PUBLIC_OPACITY_EXAMPLE") ||
      !contains(opacity, "static_assert(cgpui::Action<")) return 3;

  const char* forbidden[]{
      "#include \"cgpui/ui/", "#include \"cgpui/platform/",
      "#include \"cgpui/renderer/", "#include \"src/", "WindowRuntime"};
  for (const char* value : forbidden) {
    if (contains(animation, value) || contains(opacity, value)) return 4;
  }
  if (!contains(behavior, "examples/animation.rs") ||
      !contains(behavior, "examples/opacity.rs") ||
      !contains(previous, "Step 650 ") ||
      !contains(previous, "official animation and opacity examples") ||
      !contains(pin, "5a823cf70ebb1d7a158c6a7ca455860cd9f6aed0") ||
      !contains(xmake, "target(\"api_parity_public_animation_example\")") ||
      !contains(xmake, "target(\"api_parity_public_opacity_example\")") ||
      !contains(xmake,
                "target(\"phase_g_animation_examples_structure_test\")")) {
    return 5;
  }
  if (line_count(animation) > 140 || line_count(opacity) > 145 ||
      line_count(behavior) > 90) return 6;

  constexpr const char* completion =
      "Phase G Step 650 ports the pinned official animation and opacity "
      "examples to public C++ authoring, demonstrating two-second repeated "
      "bounce rotation, click-restarted opacity transitions, cancellation-"
      "safe restarts, shared frame pacing, and compile/run smoke coverage "
      "without direct runtime internals. Step 651 file-backed asset loading "
      "production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 7;
  }
  if (!contains(roadmap, "[x] Steps 643-650: Complete animation") ||
      !contains(vocabulary, "public_animation_example") ||
      !contains(vocabulary, "public_opacity_example") ||
      !contains(core, "official animation and opacity examples") ||
      !contains(ledger_json, "\"phase_g_step_650_sources\"") ||
      !contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 670 examples and smoke test matrix coverage\"")) return 8;
  return 0;
}
