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

int test_draw_data_preserves_contiguous_page_runs() {
  cgpui::GlyphCache cache;
  const cgpui::TextDraw warmup = three_page_text_draw();
  (void)cgpui::vulkan_build_textured_glyph_quads(warmup, cache);

  cgpui::TextDraw alternating;
  alternating.glyphs = {
      warmup.glyphs[8],
      warmup.glyphs[0],
      warmup.glyphs[8],
  };
  const std::vector<cgpui::TextDraw> draws{alternating};
  cgpui::VulkanGlyphAtlasDrawData draw_data =
      cgpui::vulkan_plan_glyph_atlas_draw_data(draws, cache);
  if (draw_data.quads.size() != 3 || draw_data.page_usages.size() != 3 ||
      draw_data.quads[0].page_index != 2 ||
      draw_data.quads[1].page_index != 0 ||
      draw_data.quads[2].page_index != 2) {
    return 10;
  }
  for (std::size_t index = 0; index < 3; ++index) {
    if (draw_data.page_usages[index].text_draw_index != 0 ||
        draw_data.page_usages[index].page_index !=
            draw_data.quads[index].page_index ||
        draw_data.page_usages[index].first_quad_index != index ||
        draw_data.page_usages[index].glyph_quad_count != 1) {
      return 11;
    }
  }

  const cgpui::VulkanGlyphAtlasResources resources = three_page_resources();
  const auto bindings = cgpui::vulkan_resolve_glyph_atlas_draw_bindings(
      draw_data.page_usages,
      resources);
  if (!bindings || bindings->size() != 3 ||
      (*bindings)[0].first_quad_index != 0 ||
      (*bindings)[1].first_quad_index != 1 ||
      (*bindings)[2].first_quad_index != 2) {
    return 12;
  }
  return cgpui::vulkan_validate_glyph_atlas_draw_bindings(
             *bindings,
             draw_data.quads,
             resources)
             ? 0
             : 13;
}

int test_invalid_quad_range_or_page_is_rejected() {
  cgpui::GlyphCache cache;
  const std::vector<cgpui::TextDraw> draws{three_page_text_draw()};
  cgpui::VulkanGlyphAtlasDrawData draw_data =
      cgpui::vulkan_plan_glyph_atlas_draw_data(draws, cache);
  const cgpui::VulkanGlyphAtlasResources resources = three_page_resources();
  auto bindings = cgpui::vulkan_resolve_glyph_atlas_draw_bindings(
      draw_data.page_usages,
      resources);
  if (!bindings) {
    return 20;
  }

  bindings->back().glyph_quad_count += 1;
  if (cgpui::vulkan_validate_glyph_atlas_draw_bindings(
          *bindings,
          draw_data.quads,
          resources)) {
    return 21;
  }
  bindings->back().glyph_quad_count -= 1;
  draw_data.quads[bindings->front().first_quad_index].page_index = 1;
  return cgpui::vulkan_validate_glyph_atlas_draw_bindings(
             *bindings,
             draw_data.quads,
             resources)
             ? 22
             : 0;
}

int test_draw_data_module_structure() {
  const std::string internal = read_source(
      "src/renderer/vulkan/vulkan_glyph_atlas_draw_bindings_internal.hpp");
  const std::string draw_data =
      read_source("src/renderer/vulkan/vulkan_glyph_atlas_draw_data.cpp");
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
  if (internal.empty() || draw_data.empty() || bindings.empty() ||
      frame.empty() || state.empty() || command_header.empty() ||
      command_source.empty()) {
    return 30;
  }
  if (!contains(internal, "struct VulkanGlyphAtlasDrawData") ||
      !contains(internal, "first_quad_index") ||
      !contains(draw_data, "vulkan_plan_glyph_atlas_draw_data(") ||
      !contains(draw_data, "vulkan_build_textured_glyph_quads(") ||
      !contains(bindings, "first_quad_index = usage.first_quad_index") ||
      !contains(frame, "VulkanGlyphAtlasDrawData draw_data") ||
      !contains(state,
                "std::vector<TexturedGlyphQuad> glyph_atlas_draw_quads_") ||
      !contains(command_header, "std::span<const TexturedGlyphQuad>") ||
      !contains(command_source, "glyph_atlas_draw_quads")) {
    return 31;
  }
  return 0;
}

int test_step_465_documentation() {
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const char* required[] = {
      "Phase E Step 465",
      "VulkanGlyphAtlasDrawData",
      "first_quad_index",
      "contiguous page runs",
      "Step 466",
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
  if (const int result = test_draw_data_preserves_contiguous_page_runs();
      result != 0) {
    return result;
  }
  if (const int result = test_invalid_quad_range_or_page_is_rejected();
      result != 0) {
    return result;
  }
  if (const int result = test_draw_data_module_structure(); result != 0) {
    return result;
  }
  return test_step_465_documentation();
}
