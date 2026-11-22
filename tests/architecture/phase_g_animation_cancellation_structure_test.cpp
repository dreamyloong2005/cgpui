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
  const std::string public_header =
      read_source("include/cgpui/ui/animation_cancellation.hpp");
  const std::string handles =
      read_source("include/cgpui/ui/runtime_handles.hpp");
  const std::string transition_header =
      read_source("include/cgpui/ui/animation_transition.hpp");
  const std::string diagnostics_header =
      read_source("include/cgpui/ui/runtime_diagnostics.hpp");
  const std::string cancellation =
      read_source("src/ui/runtime_animation_cancellation.cpp");
  const std::string state = read_source("src/ui/runtime_animation_state.cpp");
  const std::string state_internal =
      read_source("src/ui/runtime_animation_state_internal.hpp");
  const std::string animation_types = read_source("src/ui/runtime_state_animations_types_internal.hpp");
  const std::string tick = read_source("src/ui/runtime_animation_tick.cpp");
  const std::string handle_source = read_source("src/ui/ui.cpp");
  const std::string transition_source =
      read_source("src/ui/animation_transition.cpp");
  const std::string diagnostic_source =
      read_source("src/ui/runtime_diagnostic_snapshot.cpp");
  const std::string runtime_internal =
      read_source("include/cgpui/ui/detail/window_runtime_declarations.hpp");
  const std::string aggregate = read_source("include/cgpui/ui/ui.hpp");
  const std::string behavior =
      read_source("tests/animation/animation_cancellation_test.cpp");
  const std::string ui_structure =
      read_source("tests/architecture/ui_source_structure_test.cpp");
  const std::string header_cleanliness =
      read_source("tests/header_cleanliness/ui_header_cleanliness.cpp");
  const std::string previous = read_source(
      "tests/architecture/phase_g_animation_curve_variant_structure_test.cpp");
  const std::string manifests = read_source("tests/animation/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  const std::string core_parity = read_source("docs/gpui-core-api-parity.md");
  const std::string* required[]{
      &public_header, &handles, &transition_header, &diagnostics_header,
      &cancellation, &state, &state_internal, &animation_types, &tick, &handle_source,
      &transition_source, &diagnostic_source, &runtime_internal, &aggregate,
      &behavior, &ui_structure, &header_cleanliness, &previous, &manifests,
      &vocabulary, &core_parity};
  for (const auto* item : required) if (item->empty()) return 1;

  if (!contains(public_header, "struct AnimationCancellationDiagnostic") ||
      !contains(handles, "bool cancelled = false;") ||
      !contains(handles, "bool cancelled() const;") ||
      !contains(transition_header, "bool cancelled() const;") ||
      !contains(diagnostics_header, "active_animation_count") ||
      !contains(diagnostics_header, "cancelled_animation_count") ||
      !contains(diagnostics_header, "last_animation_cancellation")) return 2;
  if (!contains(animation_types, "cancelled_elapsed_ms") ||
      !contains(animation_types, "bool cancelled = false") ||
      !contains(state_internal, "last_animation_cancellation_") ||
      !contains(cancellation, "animation->callback = {}") ||
      !contains(cancellation, "animation->cancelled_elapsed_ms") ||
      !contains(cancellation, "timer_was_active") ||
      !contains(tick, "animation->cancelled") ||
      contains(state, "bool WindowRuntime::cancel_animation(")) return 3;
  if (!contains(state, "!snapshot->complete && !snapshot->cancelled") ||
      !contains(handle_source, "AnimationHandle::cancelled() const") ||
      !contains(transition_source,
                "AnimationTransitionHandle::cancelled() const") ||
      !contains(diagnostic_source, "active_animation_count") ||
      !contains(diagnostic_source, "last_animation_cancellation_")) return 4;
  if (!contains(behavior, "callback_lifetime.expired()") ||
      !contains(behavior, "after_time->elapsed_ms != before->elapsed_ms") ||
      !contains(behavior, "context.cancel_animation(") ||
      !contains(behavior, "context.async_context().cancel_animation(") ||
      !contains(behavior, "diagnostics.cancelled_animation_count != 4") ||
      !contains(behavior, "last_animation_cancellation->timer_was_active")) {
    return 5;
  }
  if (!contains(aggregate, "cgpui/ui/animation_cancellation.hpp") ||
      !contains(ui_structure, "include/cgpui/ui/animation_cancellation.hpp") ||
      !contains(ui_structure, "src/ui/runtime_animation_cancellation.cpp") ||
      !contains(header_cleanliness, "cgpui/ui/animation_cancellation.hpp") ||
      !contains(manifests, "\"animation_cancellation_test.cpp\"") ||
      !contains(manifests,
                "\"phase_g_animation_cancellation_structure_test.cpp\"")) {
    return 6;
  }
  if (line_count(public_header) > 30 || line_count(handles) > 130 ||
      line_count(transition_header) > 100 ||
      line_count(diagnostics_header) > 140 || line_count(cancellation) > 70 ||
      line_count(state) > 100 || line_count(state_internal) > 25 ||
      line_count(animation_types) > 25 ||
      line_count(tick) > 70 || line_count(handle_source) > 70 ||
      line_count(transition_source) > 120 ||
      line_count(diagnostic_source) > 150 ||
      line_count(runtime_internal) > 260 || line_count(behavior) > 190) {
    return 7;
  }

  if (!contains(vocabulary, "`AnimationCancellationDiagnostic`") ||
      !contains(core_parity, "last-cancellation diagnostics")) return 9;
  return 0;
}
