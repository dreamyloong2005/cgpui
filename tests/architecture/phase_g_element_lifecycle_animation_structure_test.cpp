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
      read_source("include/cgpui/ui/element_animation.hpp");
  const std::string aggregate = read_source("include/cgpui/ui/ui.hpp");
  const std::string store =
      read_source("src/ui/element_animation_state_store.cpp");
  const std::string element = read_source("src/ui/element_animation.cpp");
  const std::string forwarding =
      read_source("src/ui/element_animation_forwarding.cpp");
  const std::string runtime =
      read_source("src/ui/runtime_element_animation.cpp");
  const std::string runtime_state =
      read_source("src/ui/runtime_animation_state_internal.hpp");
  const std::string runtime_internal =
      read_source("src/ui/window_runtime_internal.hpp");
  const std::string flex = read_source("src/ui/element_flex_layout.cpp");
  const std::string stack =
      read_source("src/ui/element_vertical_stack_node.cpp");
  const std::string scroll_header =
      read_source("include/cgpui/ui/element_scroll_nodes.hpp");
  const std::string scroll = read_source("src/ui/element_scroll_layout.cpp");
  const std::string behavior =
      read_source("tests/animation/element_lifecycle_animation_test.cpp");
  const std::string ui_structure =
      read_source("tests/architecture/ui_source_structure_test.cpp");
  const std::string header_cleanliness =
      read_source("tests/header_cleanliness/ui_header_cleanliness.cpp");
  const std::string previous =
      read_source("tests/architecture/phase_g_animation_transition_structure_test.cpp");
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
      &public_header, &aggregate, &store, &element, &forwarding, &runtime,
      &runtime_state, &runtime_internal, &flex, &stack, &scroll_header,
      &scroll, &behavior, &ui_structure, &header_cleanliness, &previous,
      &xmake, &vocabulary, &core_parity, &roadmap, &ledger_md, &ledger_json,
      &task_plan, &findings};
  for (const auto* item : required) if (item->empty()) return 1;

  if (!contains(public_header, "struct ElementAnimationSnapshot") ||
      !contains(public_header, "struct ElementAnimationFrameResult") ||
      !contains(public_header, "class ElementAnimationStateStore") ||
      !contains(public_header, "class AnimationElement final") ||
      !contains(public_header, "ElementAnimationCallback") ||
      !contains(public_header, "AnyElement with_animation(")) return 2;
  if (!contains(store, "ScopedElementAnimationKey") ||
      !contains(store, "begin_frame(") || !contains(store, "resolve(") ||
      !contains(store, "finish_frame(") ||
      !contains(store, "seen_generation") ||
      !contains(store, "impl_->records.erase(record)")) return 3;
  if (!contains(element, "callback_(*child_, snapshot)") ||
      !contains(element, "set_key(animation_key_)") ||
      !contains(forwarding, "AnimationElement::handle_event(") ||
      !contains(forwarding, "AnimationElement::accessibility_patterns()")) {
    return 4;
  }
  if (!contains(runtime, "element_animation_state_store_.begin_frame") ||
      !contains(runtime, "element_animation_state_store_.finish_frame") ||
      !contains(runtime, "request_element_animation_frame(") ||
      !contains(runtime, "clear_element_animation_frame()") ||
      !contains(runtime_state, "ElementAnimationStateStore") ||
      !contains(runtime_internal, "runtime_animation_state_internal.hpp")) {
    return 5;
  }
  const std::string* propagation[]{&flex, &stack, &scroll_header, &scroll};
  for (const auto* source : propagation) {
    if (!contains(*source, "LayoutInput") ||
        !contains(*source, "= input;") ||
        !contains(*source, ".constraints = {};")) return 6;
  }
  if (!contains(behavior, "cgpui/prelude.hpp") ||
      !contains(behavior, "test_scoped_store_mount_update_unmount_and_isolation") ||
      !contains(behavior, "test_runtime_preserves_animation_across_rendered_wrapper_instances") ||
      !contains(behavior, "std::vector<std::uint64_t>({16, 14})") ||
      !contains(behavior, "view.render_count != 4")) return 7;
  if (!contains(aggregate, "cgpui/ui/element_animation.hpp") ||
      !contains(ui_structure, "include/cgpui/ui/element_animation.hpp") ||
      !contains(ui_structure, "src/ui/runtime_element_animation.cpp") ||
      !contains(header_cleanliness, "cgpui/ui/element_animation.hpp") ||
      !contains(xmake, "target(\"phase_g_element_lifecycle_animation_test\")") ||
      !contains(xmake, "target(\"phase_g_element_lifecycle_animation_structure_test\")") ||
      !contains(previous, "644 element lifecycle animation")) return 8;
  if (line_count(public_header) > 140 || line_count(store) > 220 ||
      line_count(element) > 100 || line_count(forwarding) > 120 ||
      line_count(runtime) > 80 || line_count(runtime_state) > 40 ||
      line_count(behavior) > 180 || line_count(runtime_internal) > 260) {
    return 9;
  }

  constexpr const char* completion =
      "Phase G Step 644 adds runtime-owned keyed element lifecycle animations "
      "with mount/update/unmount tracking, cross-window scope isolation, "
      "per-frame eased snapshots across reconstructed wrappers, transparent "
      "element forwarding, and one shared delayed frame wakeup. Step 645 "
      "animation repeat and chaining production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 10;
  }
  if (!contains(vocabulary, "`ElementAnimationStateStore`") ||
      !contains(core_parity, "runtime-owned keyed element lifecycle") ||
      !contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 676 candidate-ledger closeout audit\"")) return 11;
  return 0;
}
