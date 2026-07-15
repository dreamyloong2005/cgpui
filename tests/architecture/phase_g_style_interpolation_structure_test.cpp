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
      read_source("include/cgpui/ui/style_animation.hpp");
  const std::string geometry_header =
      read_source("src/ui/style_tween_geometry_internal.hpp");
  const std::string geometry =
      read_source("src/ui/style_tween_geometry.cpp");
  const std::string style = read_source("src/ui/style_tween.cpp");
  const std::string behavior =
      read_source("tests/animation/style_interpolation_test.cpp");
  const std::string previous = read_source(
      "tests/architecture/phase_g_animation_frame_pacing_structure_test.cpp");
  const std::string ui_structure =
      read_source("tests/architecture/ui_source_structure_test.cpp");
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
      &header, &geometry_header, &geometry, &style, &behavior, &previous,
      &ui_structure, &xmake, &vocabulary, &core, &roadmap, &ledger_md,
      &ledger_json, &task_plan, &findings};
  for (const auto* item : required) if (item->empty()) return 1;

  if (!contains(header, "Style tween(") ||
      !contains(header, "struct StyleTween") ||
      contains(header, "tween_style_size") ||
      contains(header, "std::function") || contains(header, "std::vector")) {
    return 2;
  }
  if (!contains(geometry_header, "tween_style_float") ||
      !contains(geometry_header, "tween_style_size") ||
      !contains(geometry_header, "tween_style_shadow") ||
      !contains(geometry_header, "tween_style_percentage_size") ||
      !contains(geometry, "!std::isfinite(from)") ||
      !contains(geometry, "from == to") ||
      !contains(geometry, "tween_style_rect") ||
      !contains(geometry, "tween_style_edges") ||
      !contains(geometry, "tween_style_radii")) return 3;
  if (!contains(style, "style.preferred_size") ||
      !contains(style, "style.percentage_size") ||
      !contains(style, "style.padding") ||
      !contains(style, "style.margin") ||
      !contains(style, "style.border_width") ||
      !contains(style, "style.border_radius") ||
      !contains(style, "style.box_shadow") ||
      !contains(style, "style.clip_rect") ||
      !contains(style, "style.flex_grow") ||
      !contains(style, "style.inset") ||
      !contains(style, "style.font_size") ||
      !contains(style, "style.opacity") ||
      !contains(style, "style.transform")) return 4;
  if (!contains(behavior, "test_layout_and_box_interpolation") ||
      !contains(behavior, "test_paint_geometry_interpolation") ||
      !contains(behavior, "test_discrete_optional_and_non_finite_policy") ||
      !contains(behavior, "std::isinf(middle.max_size.width)") ||
      !contains(behavior, "end.background_color.has_value()")) return 5;
  if (!contains(previous, "Step 649 style ") ||
      !contains(previous, "interpolation production behavior") ||
      !contains(ui_structure, "src/ui/style_tween.cpp") ||
      !contains(xmake, "target(\"phase_g_style_interpolation_test\")") ||
      !contains(xmake,
                "target(\"phase_g_style_interpolation_structure_test\")")) {
    return 6;
  }
  if (line_count(header) > 30 || line_count(geometry_header) > 35 ||
      line_count(geometry) > 110 || line_count(style) > 80 ||
      line_count(behavior) > 180) return 7;

  constexpr const char* completion =
      "Phase G Step 649 broadens production style interpolation across "
      "layout geometry, percentage sizing, spacing, borders, shadows, "
      "clipping, flex, inset, typography, colors, opacity, and transforms, "
      "preserves discrete and one-sided optional values until the endpoint, "
      "and avoids NaNs for unchanged non-finite dimensions. Step 650 "
      "official animation and opacity examples is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 8;
  }
  if (!contains(vocabulary, "broad style interpolation") ||
      !contains(core, "broad production style interpolation") ||
      !contains(ledger_json, "\"phase_g_step_649_sources\"") ||
      !contains(ledger_json,
                "\"phase_f_current_handoff\": \"Phase H Step 679 Cocoa application and NSWindow lifecycle\"")) return 9;
  return 0;
}
