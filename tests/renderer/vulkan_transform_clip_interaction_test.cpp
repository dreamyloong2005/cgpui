#include "cgpui/ui/paint.hpp"
#include "paint_clip.hpp"
#include "paint_clip_transform.hpp"
#include "vulkan_clip_scissor_internal.hpp"

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <limits>
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

cgpui::Rect rect(float x, float y, float width, float height) {
  return cgpui::Rect{
      .origin = {.x = x, .y = y},
      .size = {.width = width, .height = height},
  };
}

bool approximately_equal(float left, float right) {
  return std::fabs(left - right) < 0.0001F;
}

bool same_rect(cgpui::Rect value, cgpui::Rect expected) {
  return approximately_equal(value.origin.x, expected.origin.x) &&
         approximately_equal(value.origin.y, expected.origin.y) &&
         approximately_equal(value.size.width, expected.size.width) &&
         approximately_equal(value.size.height, expected.size.height);
}

int test_clip_transform_resolves_framebuffer_aabb() {
  const cgpui::AffineTransform quarter_turn{
      .scale_x = 0.0F,
      .skew_y = 1.0F,
      .skew_x = -1.0F,
      .scale_y = 0.0F,
      .translate_x = 10.0F,
  };
  const cgpui::Rect transformed =
      cgpui::transform_clip_rect_to_framebuffer_aabb(
          rect(1.0F, 2.0F, 3.0F, 4.0F), quarter_turn);
  if (!same_rect(transformed, rect(4.0F, 1.0F, 4.0F, 3.0F))) {
    return 10;
  }

  cgpui::AffineTransform invalid;
  invalid.translate_y = std::numeric_limits<float>::quiet_NaN();
  const cgpui::Rect original = rect(2.0F, 3.0F, 5.0F, 7.0F);
  return same_rect(
             cgpui::transform_clip_rect_to_framebuffer_aabb(original, invalid),
             original)
             ? 0
             : 11;
}

int test_push_clip_captures_current_precomposed_transform() {
  cgpui::PaintList paint_list;
  paint_list.push_metadata(cgpui::PaintMetadata{
      .transform = cgpui::AffineTransform::translation(10.0F, 20.0F),
  });
  paint_list.push_clip(rect(1.0F, 2.0F, 3.0F, 4.0F));
  paint_list.fill_rect(rect(0.0F, 0.0F, 20.0F, 20.0F),
                       cgpui::Color{.r = 1.0F, .a = 1.0F});
  const auto commands = paint_list.commands();
  if (commands.size() != 1 || !commands[0].clip_rect.has_value() ||
      !same_rect(*commands[0].clip_rect, rect(11.0F, 22.0F, 3.0F, 4.0F)) ||
      commands[0].clip_stack.clips.size() != 1) {
    return 20;
  }
  return same_rect(
             commands[0].clip_stack.clips[0],
             rect(11.0F, 22.0F, 3.0F, 4.0F))
             ? 0
             : 21;
}

int test_nested_clip_transforms_are_captured_at_each_push() {
  cgpui::PaintList paint_list;
  paint_list.push_metadata(cgpui::PaintMetadata{
      .transform = cgpui::AffineTransform::translation(10.0F, 0.0F),
  });
  paint_list.push_clip(rect(0.0F, 0.0F, 20.0F, 20.0F));
  paint_list.push_metadata(cgpui::PaintMetadata{
      .transform = cgpui::AffineTransform::scale(2.0F, 1.0F),
  });
  paint_list.push_clip(rect(5.0F, 0.0F, 10.0F, 10.0F));
  paint_list.fill_rect(rect(0.0F, 0.0F, 20.0F, 20.0F),
                       cgpui::Color{.g = 1.0F, .a = 1.0F});

  const auto commands = paint_list.commands();
  if (commands.size() != 1 || commands[0].clip_stack.full_depth != 2 ||
      commands[0].clip_stack.clips.size() != 2 ||
      !same_rect(commands[0].clip_stack.clips[0],
                 rect(10.0F, 0.0F, 20.0F, 20.0F)) ||
      !same_rect(commands[0].clip_stack.clips[1],
                 rect(20.0F, 0.0F, 10.0F, 10.0F)) ||
      !commands[0].clip_rect.has_value()) {
    return 30;
  }
  return same_rect(*commands[0].clip_rect, rect(20.0F, 0.0F, 10.0F, 10.0F))
             ? 0
             : 31;
}

int test_clip_before_transform_stays_in_framebuffer_space() {
  cgpui::PaintList paint_list;
  paint_list.push_clip(rect(3.0F, 4.0F, 5.0F, 6.0F));
  paint_list.push_metadata(cgpui::PaintMetadata{
      .transform = cgpui::AffineTransform::translation(100.0F, 100.0F),
  });
  paint_list.fill_rect(rect(0.0F, 0.0F, 20.0F, 20.0F),
                       cgpui::Color{.b = 1.0F, .a = 1.0F});
  const auto commands = paint_list.commands();
  if (commands.size() != 1 || !commands[0].clip_rect.has_value() ||
      !same_rect(*commands[0].clip_rect, rect(3.0F, 4.0F, 5.0F, 6.0F))) {
    return 40;
  }
  const cgpui::VulkanClipScissorResolution scissor =
      cgpui::vulkan_resolve_clip_stack_scissor(
          VkExtent2D{.width = 200, .height = 200},
          commands[0].clip_rect,
          commands[0].clip_stack);
  return scissor.visible && scissor.clipped &&
                 scissor.scissor.offset.x == 3 &&
                 scissor.scissor.offset.y == 4 &&
                 scissor.scissor.extent.width == 5 &&
                 scissor.scissor.extent.height == 6
             ? 0
             : 41;
}

int test_structure_and_documentation() {
  const std::string clip_transform_header =
      read_source("src/ui/paint_clip_transform.hpp");
  const std::string clip_transform_source =
      read_source("src/ui/paint_clip_transform.cpp");
  const std::string clip_source = read_source("src/ui/paint_clip.cpp");
  const std::string paint_source = read_source("src/ui/paint.cpp");
  const std::string ui_structure =
      read_source("tests/architecture/ui_source_structure_test.cpp");
  const std::string vulkan_clip =
      read_source("src/renderer/vulkan/vulkan_clip_scissor.cpp");
  if (clip_transform_header.empty() || clip_transform_source.empty() ||
      clip_source.empty() || paint_source.empty() || ui_structure.empty() ||
      vulkan_clip.empty()) {
    return 50;
  }
  if (!contains(clip_transform_header,
                "transform_clip_rect_to_framebuffer_aabb(") ||
      !contains(clip_transform_source,
                "transform_clip_rect_to_framebuffer_aabb(") ||
      !contains(paint_source, "transform_clip_rect_to_framebuffer_aabb(") ||
      !contains(ui_structure, "transform_clip_rect_to_framebuffer_aabb(") ||
      contains(clip_source, "transform_clip_rect_to_framebuffer_aabb(") ||
      contains(vulkan_clip, "transform_clip_rect_to_framebuffer_aabb(") ||
      contains(vulkan_clip, "vulkan_apply_composed_transform(")) {
    return 51;
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
      "Phase E Step 487",
      "transform_clip_rect_to_framebuffer_aabb",
      "push-time framebuffer AABB",
      "Step 488",
  };
  for (const char* value : required) {
    if (!contains(roadmap, value) || !contains(ledger_md, value) ||
        !contains(ledger_json, value) || !contains(task_plan, value) ||
        !contains(findings, value)) {
      return 60;
    }
  }
  return 0;
}

} // namespace

int main() {
  if (const int result = test_clip_transform_resolves_framebuffer_aabb();
      result != 0) {
    return result;
  }
  if (const int result =
          test_push_clip_captures_current_precomposed_transform();
      result != 0) {
    return result;
  }
  if (const int result = test_nested_clip_transforms_are_captured_at_each_push();
      result != 0) {
    return result;
  }
  if (const int result = test_clip_before_transform_stays_in_framebuffer_space();
      result != 0) {
    return result;
  }
  return test_structure_and_documentation();
}
