#include "cgpui/ui/element.hpp"
#include "cgpui/ui/paint.hpp"
#include "cgpui/ui/style.hpp"
#include "vulkan_rounded_rect_geometry_internal.hpp"

#include <array>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <memory>
#include <optional>
#include <span>
#include <string>

namespace {

std::string source_root() {
  if (const char* root = std::getenv("CGPUI_SOURCE_ROOT"); root != nullptr) {
    return root;
  }
  return ".";
}

std::string read_source(const char* path) {
  std::ifstream source(source_root() + "/" + path);
  if (!source) {
    source.open(path);
  }
  if (!source) {
    return {};
  }
  return std::string{
      std::istreambuf_iterator<char>(source),
      std::istreambuf_iterator<char>()};
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

constexpr cgpui::Rect test_rect() {
  return cgpui::Rect{
      .origin = {.x = 10.0F, .y = 20.0F},
      .size = {.width = 40.0F, .height = 20.0F},
  };
}

int test_paint_list_exposes_fill_and_stroke_variants() {
  const cgpui::Color fill{.r = 0.2F, .g = 0.4F, .b = 0.6F, .a = 1.0F};
  const cgpui::Color border{.r = 0.9F, .g = 0.1F, .b = 0.2F, .a = 1.0F};
  const cgpui::BorderRadii radius = cgpui::BorderRadii::all(6.0F);
  cgpui::PaintList paint_list;
  paint_list.fill_rounded_rect(test_rect(), fill, radius);
  paint_list.fill_stroked_rounded_rect(
      test_rect(), fill, radius, border, 2.0F);
  paint_list.stroke_rounded_rect(test_rect(), border, radius, 3.0F);

  const std::span<const cgpui::PaintCommand> commands = paint_list.commands();
  if (commands.size() != 3 || !commands[0].rounded_rect.fill_enabled ||
      commands[0].rounded_rect.border_color.has_value() ||
      !commands[1].rounded_rect.fill_enabled ||
      !commands[1].rounded_rect.border_color.has_value() ||
      commands[1].rounded_rect.border_width != 2.0F ||
      commands[2].rounded_rect.fill_enabled ||
      !commands[2].rounded_rect.border_color.has_value() ||
      commands[2].rounded_rect.border_width != 3.0F) {
    return 10;
  }
  return 0;
}

std::unique_ptr<cgpui::Element> styled_box(
    std::optional<cgpui::Color> background,
    cgpui::EdgeSizes border_width) {
  cgpui::Style style = cgpui::Style{}
                           .with_border_color(
                               cgpui::Color{.r = 0.8F, .a = 1.0F})
                           .with_border_width(border_width)
                           .with_border_radius(cgpui::BorderRadii::all(6.0F))
                           .with_preferred_size(
                               cgpui::Size{.width = 40.0F, .height = 20.0F});
  if (background.has_value()) {
    style = style.with_background_color(*background);
  }
  return cgpui::ElementBuilder::box().style(style).build();
}

int test_styled_box_coalesces_uniform_fill_and_stroke_variants() {
  const cgpui::Color fill{.g = 0.4F, .a = 1.0F};
  std::unique_ptr<cgpui::Element> filled =
      styled_box(fill, cgpui::EdgeSizes::all(2.0F));
  (void)filled->layout(cgpui::LayoutInput{});
  cgpui::PaintList filled_paint;
  filled->paint(filled_paint);
  const auto filled_commands = filled_paint.commands();
  if (filled_commands.size() != 1 ||
      filled_commands[0].kind != cgpui::PaintCommandKind::rounded_rect ||
      !filled_commands[0].rounded_rect.fill_enabled ||
      !filled_commands[0].rounded_rect.border_color.has_value() ||
      filled_commands[0].rounded_rect.border_width != 2.0F) {
    return 20;
  }

  std::unique_ptr<cgpui::Element> stroke_only =
      styled_box(std::nullopt, cgpui::EdgeSizes::all(3.0F));
  (void)stroke_only->layout(cgpui::LayoutInput{});
  cgpui::PaintList stroke_paint;
  stroke_only->paint(stroke_paint);
  const auto stroke_commands = stroke_paint.commands();
  if (stroke_commands.size() != 1 ||
      stroke_commands[0].kind != cgpui::PaintCommandKind::rounded_rect ||
      stroke_commands[0].rounded_rect.fill_enabled ||
      !stroke_commands[0].rounded_rect.border_color.has_value() ||
      stroke_commands[0].rounded_rect.border_width != 3.0F) {
    return 21;
  }
  return 0;
}

int test_nonuniform_styled_border_keeps_edge_fallback() {
  std::unique_ptr<cgpui::Element> element = styled_box(
      cgpui::Color{.b = 0.5F, .a = 1.0F},
      cgpui::EdgeSizes::trbl(1.0F, 2.0F, 3.0F, 4.0F));
  (void)element->layout(cgpui::LayoutInput{});
  cgpui::PaintList paint_list;
  element->paint(paint_list);
  const auto commands = paint_list.commands();
  return commands.size() == 5 &&
                 commands[0].kind == cgpui::PaintCommandKind::rounded_rect &&
                 commands[1].kind == cgpui::PaintCommandKind::solid_rect &&
                 commands[4].kind == cgpui::PaintCommandKind::solid_rect
             ? 0
             : 30;
}

int test_vulkan_stroke_only_geometry_skips_fill_vertices() {
  const std::array<cgpui::RoundedRectDraw, 2> draws{
      cgpui::RoundedRectDraw{
          .rect = test_rect(),
          .color = {.a = 0.0F},
          .radius = cgpui::BorderRadii::all(6.0F),
          .fill_enabled = false,
          .border_color = cgpui::Color{.r = 1.0F, .a = 1.0F},
          .border_width = 2.0F,
      },
      cgpui::RoundedRectDraw{
          .rect = test_rect(),
          .color = {.a = 0.0F},
          .radius = cgpui::BorderRadii::all(6.0F),
          .fill_enabled = false,
      },
  };
  const cgpui::VulkanRoundedRectGeometry geometry =
      cgpui::vulkan_build_rounded_rect_geometry(draws, 2);
  return geometry.draws.size() == 1 && geometry.draws[0].source_index == 0 &&
                 geometry.vertices.size() == 36 &&
                 geometry.indices.size() == 144
             ? 0
             : 40;
}

int test_step_482_documentation() {
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const char* required[] = {
      "Phase E Step 482",
      "rounded rectangle fill variants",
      "stroke-only geometry",
      "Step 483",
  };
  for (const char* value : required) {
    if (!contains(roadmap, value) || !contains(ledger_md, value) ||
        !contains(ledger_json, value) || !contains(task_plan, value) ||
        !contains(findings, value)) {
      return 50;
    }
  }
  return 0;
}

} // namespace

int main() {
  if (const int result = test_paint_list_exposes_fill_and_stroke_variants();
      result != 0) {
    return result;
  }
  if (const int result =
          test_styled_box_coalesces_uniform_fill_and_stroke_variants();
      result != 0) {
    return result;
  }
  if (const int result = test_nonuniform_styled_border_keeps_edge_fallback();
      result != 0) {
    return result;
  }
  if (const int result =
          test_vulkan_stroke_only_geometry_skips_fill_vertices();
      result != 0) {
    return result;
  }
  return test_step_482_documentation();
}
