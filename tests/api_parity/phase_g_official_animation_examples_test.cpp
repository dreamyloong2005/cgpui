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

} // namespace

int main() {
  const std::string animation = read_source(
      "examples/api_parity/public_animation_example/main.cpp");
  const std::string opacity = read_source(
      "examples/api_parity/public_opacity_example/main.cpp");
  const std::string pin = read_source("docs/gpui-upstream-pinned-revision.md");
  const std::string xmake = read_source("xmake.lua");
  if (animation.empty() || opacity.empty() || pin.empty() || xmake.empty()) {
    return 1;
  }

  if (!contains(animation, "class PublicAnimationExampleView") ||
      !contains(animation, "cgpui::with_animations(") ||
      !contains(animation, ".repeat()") ||
      !contains(animation, "duration_ms = 2'000") ||
      !contains(animation, "AnimationCurve::bounce(") ||
      !contains(animation, "AnimationEasing::ease_in_out") ||
      !contains(animation, "set_inline_style(") ||
      !contains(animation, "rotation_transform(")) return 2;

  if (!contains(opacity, "class PublicOpacityExampleView") ||
      !contains(opacity, "RestartOpacityAnimationAction") ||
      !contains(opacity, "cgpui::start_animation_transition(") ||
      !contains(opacity, "cgpui::AnimationTransition{.from = 0.0F, .to = 1.0F}") ||
      !contains(opacity, "transition_.cancel()") ||
      !contains(opacity, "opacity_ = snapshot.value") ||
      !contains(opacity, ".opacity(opacity_)") ||
      !contains(opacity, "event.dispatch_action(")) return 3;

  if (!contains(pin, "examples/animation.rs") ||
      !contains(pin, "examples/opacity.rs") ||
      !contains(pin, "5a823cf70ebb1d7a158c6a7ca455860cd9f6aed0") ||
      !contains(xmake, "target(\"api_parity_public_animation_example\")") ||
      !contains(xmake, "target(\"api_parity_public_opacity_example\")") ||
      !contains(xmake, "target(\"phase_g_official_animation_examples_test\")")) {
    return 4;
  }

  const std::string forbidden[]{
      "#include \"cgpui/ui/", "#include \"cgpui/platform/",
      "#include \"cgpui/renderer/", "#include \"src/", "WindowRuntime",
      "runtime.", ".runtime"};
  for (const std::string& value : forbidden) {
    if (contains(animation, value.c_str()) ||
        contains(opacity, value.c_str())) return 5;
  }
  return 0;
}
