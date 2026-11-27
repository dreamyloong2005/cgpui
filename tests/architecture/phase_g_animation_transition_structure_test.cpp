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
      read_source("include/cgpui/ui/detail/window_runtime_declarations.hpp");
  const std::string behavior =
      read_source("tests/animation/runtime_animation_transition_test.cpp");
  const std::string ui_structure =
      read_source("tests/architecture/ui_source_structure_test.cpp");
  const std::string header_cleanliness =
      read_source("tests/header_cleanliness/ui_header_cleanliness.cpp");
  const std::string previous =
      read_source("tests/api_parity/phase_g_async_runtime_closeout_test.cpp");
  const std::string manifests = read_source("tests/animation/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  const std::string core_parity = read_source("docs/gpui-core-api-parity.md");
  const std::string* required[]{
      &public_header, &aggregate, &source, &animation_start, &runtime_header,
      &runtime_internal, &behavior, &ui_structure, &header_cleanliness,
      &previous, &manifests, &vocabulary, &core_parity};
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
      !contains(manifests, "\"runtime_animation_transition_test.cpp\"") ||
      !contains(manifests,
                "\"phase_g_animation_transition_structure_test.cpp\"")) return 6;
  if (line_count(public_header) > 100 || line_count(source) > 120 ||
      line_count(animation_start) > 80 || line_count(behavior) > 190 ||
      line_count(runtime_header) > 263 || line_count(runtime_internal) > 260) {
    return 7;
  }

  if (!contains(vocabulary, "`AnimationTransition`") ||
      !contains(core_parity, "typed scalar animation transitions")) return 9;
  return 0;
}
