#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string read_source(const char* path) {
  const char* root = CGPUI_SOURCE_ROOT;
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
  const std::string manifests = read_source("examples/api_parity/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string pin = read_source("docs/gpui-upstream-pinned-revision.md");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  const std::string core = read_source("docs/gpui-core-api-parity.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string* required[]{
      &animation, &opacity, &behavior, &manifests, &pin, &vocabulary,
      &core, &ledger_json};
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
      !contains(pin, "5a823cf70ebb1d7a158c6a7ca455860cd9f6aed0") ||
      !contains(manifests, "[targets.api_parity_public_animation_example]") ||
      !contains(manifests, "[targets.api_parity_public_opacity_example]") ||
      !contains(manifests,
                "\"phase_g_animation_examples_structure_test.cpp\"")) {
    return 5;
  }
  if (line_count(animation) > 140 || line_count(opacity) > 145 ||
      line_count(behavior) > 90) return 6;

  if (!contains(vocabulary, "public_animation_example") ||
      !contains(vocabulary, "public_opacity_example") ||
      !contains(core, "official animation and opacity examples") ||
      !contains(ledger_json, "\"phase_g_step_650_sources\"")) return 8;
  return 0;
}
