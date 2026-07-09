#include "vulkan_glyph_atlas_draw_bindings_internal.hpp"

#include <cstdlib>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <string>
#include <type_traits>

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

template <typename Handle>
Handle fake_handle(std::uintptr_t value) {
  if constexpr (std::is_pointer_v<Handle>) {
    return reinterpret_cast<Handle>(value);
  } else {
    return static_cast<Handle>(value);
  }
}

cgpui::TextDraw three_page_text_draw() {
  cgpui::TextDraw draw{
      .bounds = cgpui::Rect{.size = cgpui::Size{512.0F, 512.0F}},
      .color = cgpui::Color{.r = 1.0F, .g = 1.0F, .b = 1.0F, .a = 1.0F},
      .font = cgpui::FontDescriptor{.family = "Inter"},
      .content = "bindings",
      .byte_length = 8,
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
        .advance = 128.0F,
        .device_advance = 128.0F,
    });
  }
  return draw;
}

cgpui::VulkanGlyphAtlasResources three_page_resources() {
  cgpui::VulkanGlyphAtlasResources resources;
  for (std::size_t page_index = 0; page_index < 3; ++page_index) {
    resources.pages.push_back(cgpui::VulkanGlyphAtlasPageResource{
        .page_index = page_index,
        .descriptor_set = fake_handle<VkDescriptorSet>(page_index + 1),
    });
  }
  return resources;
}

int test_page_usage_plans_and_resolves_descriptor_bindings() {
  cgpui::GlyphCache cache;
  const std::vector<cgpui::TextDraw> draws{three_page_text_draw()};
  const std::vector<cgpui::VulkanGlyphAtlasDrawPageUsage> usages =
      cgpui::vulkan_plan_glyph_atlas_draw_page_usages(draws, cache);
  if (usages.size() != 3 || usages[0].text_draw_index != 0 ||
      usages[0].page_index != 0 || usages[0].glyph_quad_count != 4 ||
      usages[1].page_index != 1 || usages[1].glyph_quad_count != 4 ||
      usages[2].page_index != 2 || usages[2].glyph_quad_count != 1 ||
      cache.atlas_pages().size() != 3) {
    return 10;
  }

  const cgpui::VulkanGlyphAtlasResources resources = three_page_resources();
  const auto bindings =
      cgpui::vulkan_resolve_glyph_atlas_draw_bindings(usages, resources);
  if (!bindings || bindings->size() != 3) {
    return 11;
  }
  for (std::size_t page_index = 0; page_index < 3; ++page_index) {
    if ((*bindings)[page_index].text_draw_index != 0 ||
        (*bindings)[page_index].page_index != page_index ||
        (*bindings)[page_index].glyph_quad_count !=
            usages[page_index].glyph_quad_count ||
        (*bindings)[page_index].descriptor_set !=
            resources.pages[page_index].descriptor_set) {
      return 12;
    }
  }
  return cgpui::vulkan_validate_glyph_atlas_draw_bindings(
             *bindings,
             resources)
             ? 0
             : 13;
}

int test_missing_or_stale_page_binding_is_rejected() {
  const std::vector<cgpui::VulkanGlyphAtlasDrawPageUsage> usages{
      cgpui::VulkanGlyphAtlasDrawPageUsage{
          .text_draw_index = 0,
          .page_index = 1,
          .glyph_quad_count = 2,
      },
  };
  cgpui::VulkanGlyphAtlasResources missing_page = three_page_resources();
  missing_page.pages.erase(missing_page.pages.begin() + 1);
  if (cgpui::vulkan_resolve_glyph_atlas_draw_bindings(usages, missing_page)) {
    return 20;
  }

  cgpui::VulkanGlyphAtlasResources resources = three_page_resources();
  const auto bindings =
      cgpui::vulkan_resolve_glyph_atlas_draw_bindings(usages, resources);
  if (!bindings) {
    return 21;
  }
  resources.pages[1].descriptor_set = fake_handle<VkDescriptorSet>(99);
  return cgpui::vulkan_validate_glyph_atlas_draw_bindings(
             *bindings,
             resources)
             ? 22
             : 0;
}

int test_draw_binding_module_structure() {
  const std::string internal = read_source(
      "src/renderer/vulkan/vulkan_glyph_atlas_draw_bindings_internal.hpp");
  const std::string bindings = read_source(
      "src/renderer/vulkan/vulkan_glyph_atlas_draw_bindings.cpp");
  const std::string frame =
      read_source("src/renderer/vulkan/vulkan_glyph_atlas_frame.cpp");
  const std::string state =
      read_source("src/renderer/vulkan/vulkan_state_internal.hpp");
  const std::string command_header = read_source(
      "src/renderer/vulkan/vulkan_command_recording_internal.hpp");
  const std::string command_source =
      read_source("src/renderer/vulkan/vulkan_command_recording.cpp");
  const std::string presentation =
      read_source("src/renderer/vulkan/vulkan_presentation.cpp");
  if (internal.empty() || bindings.empty() || frame.empty() || state.empty() ||
      command_header.empty() || command_source.empty() ||
      presentation.empty()) {
    return 30;
  }
  if (!contains(internal, "struct VulkanGlyphAtlasDrawPageUsage") ||
      !contains(internal, "struct VulkanGlyphAtlasDrawBinding") ||
      !contains(bindings, "vulkan_build_textured_glyph_quads(") ||
      !contains(bindings, "vulkan_resolve_glyph_atlas_draw_bindings(") ||
      !contains(frame, "vulkan_plan_glyph_atlas_draw_page_usages(") ||
      !contains(frame, "vulkan_resolve_glyph_atlas_draw_bindings(") ||
      !contains(state,
                "std::vector<VulkanGlyphAtlasDrawBinding> glyph_atlas_draw_bindings_") ||
      !contains(command_header,
                "std::span<const VulkanGlyphAtlasDrawBinding>") ||
      !contains(command_source,
                "vulkan_validate_glyph_atlas_draw_bindings(") ||
      !contains(presentation, "glyph_atlas_draw_bindings_")) {
    return 31;
  }
  return 0;
}

int test_step_464_documentation() {
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const char* required[] = {
      "Phase E Step 464",
      "VulkanGlyphAtlasDrawBinding",
      "vulkan_resolve_glyph_atlas_draw_bindings",
      "live command buffer",
      "Step 465",
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
  if (const int result =
          test_page_usage_plans_and_resolves_descriptor_bindings();
      result != 0) {
    return result;
  }
  if (const int result = test_missing_or_stale_page_binding_is_rejected();
      result != 0) {
    return result;
  }
  if (const int result = test_draw_binding_module_structure(); result != 0) {
    return result;
  }
  return test_step_464_documentation();
}
