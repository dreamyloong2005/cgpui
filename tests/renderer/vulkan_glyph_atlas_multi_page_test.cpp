#include "cgpui/renderer/renderer.hpp"
#include "vulkan_glyph_atlas_resources_internal.hpp"

#include <cstdlib>
#include <fstream>
#include <iterator>
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

cgpui::TextDraw three_page_text_draw() {
  cgpui::TextDraw draw{
      .bounds = cgpui::Rect{.size = cgpui::Size{512.0F, 512.0F}},
      .color = cgpui::Color{.r = 1.0F, .g = 1.0F, .b = 1.0F, .a = 1.0F},
      .font = cgpui::FontDescriptor{.family = "Inter"},
      .content = "multi-page",
      .byte_length = 10,
      .font_size = 128.0F,
      .device_font_size = 128.0F,
  };
  draw.glyphs.reserve(9);
  for (std::size_t index = 0; index < 9; ++index) {
    draw.glyphs.push_back(cgpui::TextGlyphPaint{
        .key =
            cgpui::GlyphAtlasKey{
                .font_family = "Inter",
                .font_size = 128.0F,
                .scale = 1.0F,
                .device_font_size = 128.0F,
                .glyph_index = index,
                .glyph_id = static_cast<std::uint32_t>(index + 1),
                .byte_offset = index,
                .byte_length = 1,
            },
        .origin = cgpui::Point{.x = static_cast<float>(index) * 128.0F},
        .advance = 128.0F,
        .device_origin =
            cgpui::Point{.x = static_cast<float>(index) * 128.0F},
        .device_advance = 128.0F,
    });
  }
  return draw;
}

int test_three_page_allocation_and_upload_planning() {
  cgpui::GlyphCache cache;
  cgpui::vulkan_consume_text_draw(three_page_text_draw(), cache);
  if (cache.atlas_pages().size() != 3 ||
      cache.upload_records().size() != 9 ||
      cache.atlas_pages()[0].entries.size() != 4 ||
      cache.atlas_pages()[1].entries.size() != 4 ||
      cache.atlas_pages()[2].entries.size() != 1) {
    return 10;
  }

  const std::vector<cgpui::GlyphAtlasUploadBatch> batches =
      cgpui::vulkan_plan_glyph_atlas_uploads(
          cache.upload_records(),
          cache.atlas_pages());
  if (batches.size() != 3 || batches[0].image.page_index != 0 ||
      batches[1].image.page_index != 1 || batches[2].image.page_index != 2 ||
      batches[0].uploads.size() != 4 || batches[1].uploads.size() != 4 ||
      batches[2].uploads.size() != 1) {
    return 11;
  }

  cgpui::GlyphAtlasProductionResourceState state;
  const cgpui::GlyphAtlasProductionPlan plan =
      cgpui::vulkan_plan_glyph_atlas_production_resources(state, batches);
  if (plan.live_resources.size() != 3 || plan.created_count != 3 ||
      plan.upload_commands.size() != 3 || plan.dirty_upload_count != 9 ||
      plan.dirty_upload_byte_count != 9U * 128U * 128U) {
    return 12;
  }
  for (std::size_t page_index = 0; page_index < 3; ++page_index) {
    if (plan.live_resources[page_index].page_index != page_index ||
        plan.upload_commands[page_index].page_index != page_index ||
        plan.upload_commands[page_index].upload_count !=
            batches[page_index].uploads.size()) {
      return 13;
    }
  }
  return 0;
}

int test_descriptor_capacity_rejection_is_preflight() {
  cgpui::GlyphAtlasProductionPlan plan;
  plan.live_resources.reserve(
      cgpui::vulkan_glyph_atlas_descriptor_capacity + 1);
  for (std::size_t page_index = 0;
       page_index <= cgpui::vulkan_glyph_atlas_descriptor_capacity;
       ++page_index) {
    plan.live_resources.push_back(cgpui::GlyphAtlasProductionResourceRecord{
        .page_index = page_index,
    });
  }
  if (cgpui::vulkan_glyph_atlas_plan_fits_descriptor_capacity(plan)) {
    return 20;
  }

  cgpui::VulkanGlyphAtlasResources resources;
  resources.pages.push_back(cgpui::VulkanGlyphAtlasPageResource{
      .page_index = 999,
      .generation = 7,
      .extent = VkExtent2D{.width = 32, .height = 32},
  });
  const cgpui::Result<void> result = cgpui::vulkan_update_glyph_atlas_resources(
      VK_NULL_HANDLE,
      VK_NULL_HANDLE,
      plan,
      resources);
  return !result && resources.pages.size() == 1 &&
                 resources.pages[0].page_index == 999 &&
                 resources.pages[0].generation == 7
             ? 0
             : 21;
}

int test_multi_page_module_structure() {
  const std::string internal = read_source(
      "src/renderer/vulkan/vulkan_glyph_atlas_resources_internal.hpp");
  const std::string descriptors = read_source(
      "src/renderer/vulkan/vulkan_glyph_atlas_descriptors.cpp");
  const std::string images =
      read_source("src/renderer/vulkan/vulkan_glyph_atlas_images.cpp");
  const std::string resources =
      read_source("src/renderer/vulkan/vulkan_glyph_atlas_resources.cpp");
  const std::string staging =
      read_source("src/renderer/vulkan/vulkan_glyph_atlas_staging.cpp");
  const std::string lifetime =
      read_source("tests/renderer/vulkan_frame_lifetime_test.cpp");
  if (internal.empty() || descriptors.empty() || images.empty() ||
      resources.empty() || staging.empty() || lifetime.empty()) {
    return 30;
  }
  if (!contains(internal, "vulkan_glyph_atlas_descriptor_capacity") ||
      !contains(internal,
                "vulkan_glyph_atlas_plan_fits_descriptor_capacity(") ||
      !contains(descriptors, "vulkan_glyph_atlas_descriptor_capacity") ||
      contains(descriptors, "max_glyph_atlas_pages = 256")) {
    return 31;
  }

  const std::size_t capacity_check = resources.find(
      "vulkan_glyph_atlas_plan_fits_descriptor_capacity(plan)");
  const std::size_t destructive_loop = resources.find(
      "for (auto page = resources.pages.begin()");
  if (capacity_check == std::string::npos ||
      destructive_loop == std::string::npos ||
      capacity_check > destructive_loop ||
      !contains(images, "vulkan_allocate_glyph_atlas_descriptor_set(") ||
      !contains(resources, "resources.pages.push_back(*created)") ||
      !contains(staging, "candidate.image.page_index == command.page_index") ||
      !contains(staging, "resources.uploads.push_back(std::move(*staged))")) {
    return 32;
  }
  return contains(lifetime, "multi_page_text_draw(") &&
                 contains(lifetime, "test_multi_page_glyph_atlas_frame(") &&
                 contains(lifetime, "index < 9") &&
                 contains(lifetime, "device_font_size = 128.0F")
             ? 0
             : 33;
}

int test_step_463_documentation() {
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const char* required[] = {
      "Phase E Step 463",
      "three atlas pages",
      "vulkan_glyph_atlas_descriptor_capacity",
      "cross-page uploads",
      "Step 464",
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
  if (const int result = test_three_page_allocation_and_upload_planning();
      result != 0) {
    return result;
  }
  if (const int result = test_descriptor_capacity_rejection_is_preflight();
      result != 0) {
    return result;
  }
  if (const int result = test_multi_page_module_structure(); result != 0) {
    return result;
  }
  return test_step_463_documentation();
}
