#include "cgpui/ui/element_tree.hpp"
#include "cgpui/ui/paint.hpp"
#include "ui_paint_internal.hpp"
#include "vulkan_frame_draw_order_internal.hpp"

#include <array>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

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

enum class TestPrimitive {
  solid,
  rounded,
  text,
};

class PrimitiveElement final : public cgpui::Element {
 public:
  PrimitiveElement(
      TestPrimitive primitive,
      float marker,
      int layer,
      int z_index = 0)
      : primitive_(primitive), marker_(marker) {
    set_layer(layer);
    set_z_index(z_index);
  }

  void paint(cgpui::PaintList& paint_list) const override {
    const cgpui::Rect bounds{
        .origin = {.x = marker_, .y = marker_},
        .size = {.width = 12.0F, .height = 8.0F},
    };
    const cgpui::Color color{
        .r = marker_,
        .g = 0.25F,
        .b = 0.5F,
        .a = 1.0F,
    };
    if (primitive_ == TestPrimitive::solid) {
      paint_list.fill_rect(bounds, color);
      return;
    }
    if (primitive_ == TestPrimitive::rounded) {
      paint_list.fill_rounded_rect(
          bounds,
          color,
          cgpui::BorderRadii::corners(2.0F, 2.0F, 2.0F, 2.0F));
      return;
    }
    paint_list.fill_text(bounds, color, "z", {}, 8.0F);
  }

 private:
  TestPrimitive primitive_ = TestPrimitive::solid;
  float marker_ = 0.0F;
};

class OrderingFrame final : public cgpui::RenderFrame {
 public:
  void clear(cgpui::Color) override {}

  void draw_rect(const cgpui::SolidRect& rect) override {
    append(
        cgpui::RendererPrimitiveKind::solid_rect,
        solid_count_++,
        rect.color.r);
  }

  void draw_rounded_rect(const cgpui::RoundedRectDraw& rect) override {
    append(
        cgpui::RendererPrimitiveKind::rounded_rect,
        rounded_count_++,
        rect.color.r);
  }

  void draw_text(const cgpui::TextDraw& text) override {
    append(cgpui::RendererPrimitiveKind::text, text_count_++, text.color.r);
  }

  cgpui::Result<void> present() override { return {}; }

  std::vector<cgpui::VulkanFrameDrawOrderEntry> draw_order;
  std::vector<float> markers;

 private:
  void append(
      cgpui::RendererPrimitiveKind kind,
      std::size_t command_index,
      float marker) {
    draw_order.push_back(cgpui::VulkanFrameDrawOrderEntry{
        .primitive_kind = kind,
        .command_index = command_index,
    });
    markers.push_back(marker);
  }

  std::size_t solid_count_ = 0;
  std::size_t rounded_count_ = 0;
  std::size_t text_count_ = 0;
};

int test_ui_z_order_reaches_vulkan_cursor() {
  cgpui::ElementTree tree;
  const cgpui::ElementId root_id =
      tree.set_root(std::make_unique<cgpui::Element>());
  (void)tree.append_child(
      root_id,
      std::make_unique<PrimitiveElement>(TestPrimitive::rounded, 0.3F, 5));
  (void)tree.append_child(
      root_id,
      std::make_unique<PrimitiveElement>(TestPrimitive::solid, 0.1F, -2));
  (void)tree.append_child(
      root_id,
      std::make_unique<PrimitiveElement>(TestPrimitive::text, 0.2F, -10, 2));
  (void)tree.append_child(
      root_id,
      std::make_unique<PrimitiveElement>(TestPrimitive::solid, 0.4F, 5));
  (void)tree.append_child(
      root_id,
      std::make_unique<PrimitiveElement>(TestPrimitive::text, 0.5F, 5));

  cgpui::PaintList paint_list;
  tree.paint(paint_list);
  const std::array<cgpui::PaintCommandKind, 5> expected_paint_kinds{
      cgpui::PaintCommandKind::solid_rect,
      cgpui::PaintCommandKind::text,
      cgpui::PaintCommandKind::rounded_rect,
      cgpui::PaintCommandKind::solid_rect,
      cgpui::PaintCommandKind::text,
  };
  const std::array<float, 5> expected_markers{0.1F, 0.2F, 0.3F, 0.4F, 0.5F};
  if (paint_list.commands().size() != expected_paint_kinds.size()) {
    return 10;
  }
  for (std::size_t index = 0; index < expected_paint_kinds.size(); ++index) {
    if (paint_list.commands()[index].kind != expected_paint_kinds[index]) {
      return 11;
    }
  }

  OrderingFrame frame;
  for (const cgpui::PaintCommand& command : paint_list.commands()) {
    cgpui::submit_paint_command_to_frame(frame, command, nullptr);
  }
  const std::array<cgpui::VulkanFrameDrawOrderEntry, 5> expected_order{
      cgpui::VulkanFrameDrawOrderEntry{
          .primitive_kind = cgpui::RendererPrimitiveKind::solid_rect,
          .command_index = 0,
      },
      cgpui::VulkanFrameDrawOrderEntry{
          .primitive_kind = cgpui::RendererPrimitiveKind::text,
          .command_index = 0,
      },
      cgpui::VulkanFrameDrawOrderEntry{
          .primitive_kind = cgpui::RendererPrimitiveKind::rounded_rect,
          .command_index = 0,
      },
      cgpui::VulkanFrameDrawOrderEntry{
          .primitive_kind = cgpui::RendererPrimitiveKind::solid_rect,
          .command_index = 1,
      },
      cgpui::VulkanFrameDrawOrderEntry{
          .primitive_kind = cgpui::RendererPrimitiveKind::text,
          .command_index = 1,
      },
  };
  if (frame.draw_order.size() != expected_order.size() ||
      frame.markers.size() != expected_markers.size()) {
    return 12;
  }
  for (std::size_t index = 0; index < expected_order.size(); ++index) {
    if (frame.draw_order[index] != expected_order[index] ||
        frame.markers[index] != expected_markers[index]) {
      return 13;
    }
  }

  const std::array<cgpui::VulkanRoundedRectDrawRange, 2> solid_draws{
      cgpui::VulkanRoundedRectDrawRange{.source_index = 0},
      cgpui::VulkanRoundedRectDrawRange{.source_index = 1},
  };
  const std::array<cgpui::VulkanRoundedRectDrawRange, 1> rounded_draws{
      cgpui::VulkanRoundedRectDrawRange{.source_index = 0},
  };
  const std::array<cgpui::VulkanTextDrawCommand, 2> text_commands{
      cgpui::VulkanTextDrawCommand{.text_draw_index = 0},
      cgpui::VulkanTextDrawCommand{.text_draw_index = 1},
  };
  cgpui::VulkanFrameDrawOrderCursor cursor(
      frame.draw_order, solid_draws, rounded_draws, text_commands);
  const std::array<cgpui::VulkanResolvedFrameDraw, 5> expected_resolved{
      cgpui::VulkanResolvedFrameDraw{
          .resource_kind = cgpui::VulkanFrameDrawResourceKind::solid_rect,
          .resource_index = 0,
      },
      cgpui::VulkanResolvedFrameDraw{
          .resource_kind = cgpui::VulkanFrameDrawResourceKind::text,
          .resource_index = 0,
      },
      cgpui::VulkanResolvedFrameDraw{
          .resource_kind = cgpui::VulkanFrameDrawResourceKind::rounded_rect,
          .resource_index = 0,
      },
      cgpui::VulkanResolvedFrameDraw{
          .resource_kind = cgpui::VulkanFrameDrawResourceKind::solid_rect,
          .resource_index = 1,
      },
      cgpui::VulkanResolvedFrameDraw{
          .resource_kind = cgpui::VulkanFrameDrawResourceKind::text,
          .resource_index = 1,
      },
  };
  for (const cgpui::VulkanResolvedFrameDraw expected : expected_resolved) {
    const auto draw = cursor.next();
    if (!draw || *draw != expected) {
      return 14;
    }
  }
  return cursor.next().has_value() ? 15 : 0;
}

int test_structure_and_documentation() {
  const std::string element_core =
      read_source("include/cgpui/ui/element_core.hpp");
  const std::string tree_paint = read_source("src/ui/element_tree_paint.cpp");
  const std::string child_ordering =
      read_source("src/ui/element_layer_ordering.cpp");
  const std::string render_view = read_source("src/ui/render_view.cpp");
  const std::string submission = read_source("src/ui/render_view_commands.cpp");
  const std::string renderer =
      read_source("src/renderer/vulkan/vulkan_renderer.cpp");
  const std::string cursor =
      read_source("src/renderer/vulkan/vulkan_frame_draw_order.cpp");
  const std::string ui_structure =
      read_source("tests/architecture/ui_source_structure_test.cpp");
  const std::string renderer_structure =
      read_source("tests/architecture/renderer_source_structure_test.cpp");
  if (element_core.empty() || tree_paint.empty() || child_ordering.empty() ||
      render_view.empty() || submission.empty() || renderer.empty() ||
      cursor.empty() || ui_structure.empty() || renderer_structure.empty()) {
    return 30;
  }
  if (!contains(element_core, "explicit_z_index != 0") ||
      !contains(tree_paint, "std::stable_sort(") ||
      !contains(tree_paint, "element->z_order()") ||
      !contains(child_ordering, "return lhs.index < rhs.index;") ||
      !contains(
          render_view,
          "for (const auto& command : paint_list.commands())") ||
      !contains(submission, "submit_paint_command_to_frame(") ||
      !contains(renderer, "draw_order_.push_back(") ||
      !contains(renderer, ".command_index = draws.size()") ||
      !contains(cursor, "VulkanFrameDrawOrderCursor::next(") ||
      !contains(ui_structure, "element_layer_ordering.cpp") ||
      !contains(renderer_structure, "vulkan_frame_draw_order.cpp")) {
    return 31;
  }

  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const char* required[] = {
      "Phase E Step 489",
      "explicit z/layer command ordering",
      "stable UI paint order",
      "Step 490",
  };
  for (const char* value : required) {
    if (!contains(roadmap, value) || !contains(ledger_md, value) ||
        !contains(ledger_json, value) || !contains(task_plan, value) ||
        !contains(findings, value)) {
      return 40;
    }
  }
  return 0;
}

} // namespace

int main() {
  if (const int result = test_ui_z_order_reaches_vulkan_cursor(); result != 0) {
    return result;
  }
  return test_structure_and_documentation();
}
