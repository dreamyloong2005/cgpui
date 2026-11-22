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
  const std::string header =
      read_source("include/cgpui/ui/style_animation.hpp");
  const std::string geometry_header =
      read_source("src/ui/style_tween_geometry_internal.hpp");
  const std::string geometry =
      read_source("src/ui/style_tween_geometry.cpp");
  const std::string style = read_source("src/ui/style_tween.cpp");
  const std::string behavior =
      read_source("tests/animation/style_interpolation_test.cpp");
  const std::string ui_structure =
      read_source("tests/architecture/ui_source_structure_test.cpp");
  const std::string manifests = read_source("tests/animation/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  const std::string core = read_source("docs/gpui-core-api-parity.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string* required[]{
      &header, &geometry_header, &geometry, &style, &behavior, &ui_structure, &manifests, &vocabulary, &core, &ledger_json};
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
  if (!contains(ui_structure, "src/ui/style_tween.cpp") ||
      !contains(manifests, "\"style_interpolation_test.cpp\"") ||
      !contains(manifests,
                "\"phase_g_style_interpolation_structure_test.cpp\"")) {
    return 6;
  }
  if (line_count(header) > 30 || line_count(geometry_header) > 35 ||
      line_count(geometry) > 110 || line_count(style) > 80 ||
      line_count(behavior) > 180) return 7;

  if (!contains(vocabulary, "broad style interpolation") ||
      !contains(core, "broad production style interpolation") ||
      !contains(ledger_json, "\"phase_g_step_649_sources\"")) return 9;
  return 0;
}
