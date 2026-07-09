#include "cgpui/renderer/renderer.hpp"

#include <array>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>
#include <vector>

namespace {

std::string source_root() {
  if (const char* root = std::getenv("CGPUI_SOURCE_ROOT"); root != nullptr) {
    return root;
  }
  return ".";
}

std::string read_source(const char* path) {
  const std::string root = source_root();
  std::ifstream source(root + "/" + path);
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

cgpui::GlyphCache seeded_cache(std::string_view text) {
  const cgpui::TextShapeRun run =
      cgpui::shape_text(text, cgpui::FontDescriptor{.family = "Inter"}, 20.0F);
  const std::vector<cgpui::TextGlyphPaint> glyphs =
      cgpui::text_glyph_paint_metadata(run);

  cgpui::GlyphCache cache;
  for (const cgpui::TextGlyphPaint& glyph : glyphs) {
    (void)cache.allocate(cgpui::rasterize_fallback_glyph(glyph));
  }
  return cache;
}

int test_phase_e_glyph_atlas_production_resources() {
  cgpui::GlyphCache cache = seeded_cache("ab");
  std::vector<cgpui::GlyphAtlasUploadBatch> batches =
      cgpui::vulkan_plan_glyph_atlas_uploads(
          cache.upload_records(),
          cache.atlas_pages());

  cgpui::GlyphAtlasProductionResourceState state;
  cgpui::GlyphAtlasProductionPlan first_plan =
      cgpui::vulkan_plan_glyph_atlas_production_resources(state, batches);
  if (first_plan.created_count != 1 || first_plan.reused_count != 0 ||
      first_plan.dropped_count != 0 || first_plan.live_resources.size() != 1 ||
      first_plan.upload_commands.size() != 1 ||
      state.live_resources().size() != 1) {
    return 10;
  }

  const cgpui::GlyphAtlasProductionResourceRecord& created =
      first_plan.live_resources[0];
  if (created.status != cgpui::GlyphAtlasProductionResourceStatus::created ||
      created.page_index != 0 || created.generation != 1 ||
      !created.image_create_ready || !created.memory_allocation_ready ||
      !created.memory_bind_ready || !created.image_view_ready ||
      !created.sampler_ready || !created.descriptor_slot_reserved ||
      !created.upload_command_path_ready || created.descriptor_set_bound ||
      created.uploaded_count != 2) {
    return 11;
  }

  const cgpui::GlyphAtlasProductionUploadCommand& first_upload =
      first_plan.upload_commands[0];
  if (first_upload.page_index != 0 || first_upload.first_upload_index != 0 ||
      first_upload.upload_count != 2 || first_upload.byte_offset != 0 ||
      first_upload.byte_size != batches[0].alpha.size() ||
      !first_upload.stages_alpha_payload ||
      !first_upload.transitions_to_transfer_dst ||
      !first_upload.copies_buffer_to_image ||
      !first_upload.transitions_to_shader_read_only) {
    return 12;
  }

  const cgpui::GlyphAtlasProductionPlan repeated_plan =
      cgpui::vulkan_plan_glyph_atlas_production_resources(state, batches);
  if (repeated_plan.created_count != 0 || repeated_plan.reused_count != 1 ||
      repeated_plan.dropped_count != 0 ||
      !repeated_plan.upload_commands.empty() ||
      repeated_plan.live_resources[0].generation != 1 ||
      repeated_plan.live_resources[0].uploaded_count != 2) {
    return 13;
  }

  cache = seeded_cache("abc");
  batches = cgpui::vulkan_plan_glyph_atlas_uploads(
      cache.upload_records(),
      cache.atlas_pages());
  const cgpui::GlyphAtlasProductionPlan dirty_plan =
      cgpui::vulkan_plan_glyph_atlas_production_resources(state, batches);
  if (dirty_plan.reused_count != 1 || dirty_plan.upload_commands.size() != 1 ||
      dirty_plan.live_resources[0].uploaded_count != 3) {
    return 14;
  }

  const cgpui::GlyphAtlasProductionUploadCommand& dirty_upload =
      dirty_plan.upload_commands[0];
  if (dirty_upload.first_upload_index != 2 || dirty_upload.upload_count != 1 ||
      dirty_upload.byte_offset != batches[0].uploads[2].byte_offset ||
      dirty_upload.byte_size != batches[0].uploads[2].byte_size) {
    return 15;
  }

  const std::vector<cgpui::GlyphAtlasUploadBatch> empty_batches;
  const cgpui::GlyphAtlasProductionPlan dropped_plan =
      cgpui::vulkan_plan_glyph_atlas_production_resources(
          state,
          empty_batches);
  return dropped_plan.dropped_count == 1 &&
                 dropped_plan.dropped_resources.size() == 1 &&
                 dropped_plan.dropped_resources[0].status ==
                     cgpui::GlyphAtlasProductionResourceStatus::dropped &&
                 state.live_resources().empty()
             ? 0
             : 16;
}

int test_phase_e_glyph_atlas_production_structure_docs() {
  const std::string xmake = read_source("xmake.lua");
  const std::string glyph_atlas =
      read_source("include/cgpui/renderer/glyph_atlas.hpp");
  const std::string production_header =
      read_source("include/cgpui/renderer/glyph_atlas_production.hpp");
  const std::string production_source =
      read_source("src/renderer/vulkan/vulkan_glyph_atlas_production.cpp");
  const std::string structure =
      read_source("tests/architecture/renderer_source_structure_test.cpp");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");

  if (xmake.empty() || glyph_atlas.empty() || production_header.empty() ||
      production_source.empty() || structure.empty() || roadmap.empty() ||
      ledger_md.empty() || ledger_json.empty() || task_plan.empty() ||
      findings.empty()) {
    return 30;
  }

  if (!contains(xmake, "target(\"phase_e_glyph_atlas_production_test\")") ||
      !contains(xmake,
                "tests/api_parity/phase_e_glyph_atlas_production_test.cpp")) {
    return 31;
  }

  if (!contains(glyph_atlas,
                "#include \"cgpui/renderer/glyph_atlas_production.hpp\"") ||
      !contains(production_header, "struct GlyphAtlasProductionPlan") ||
      !contains(production_header,
                "class GlyphAtlasProductionResourceState") ||
      !contains(production_source,
                "vulkan_plan_glyph_atlas_production_resources(") ||
      !contains(structure, "glyph_atlas_production.hpp") ||
      !contains(structure, "vulkan_glyph_atlas_production.cpp")) {
    return 32;
  }

  constexpr std::array required_docs{
      "Phase E Step 459 Vulkan glyph atlas production",
      "glyph_atlas_production.hpp",
      "vulkan_glyph_atlas_production.cpp",
      "GlyphAtlasProductionResourceState",
      "vulkan_plan_glyph_atlas_production_resources",
      "alpha8 atlas page image readiness",
      "memory allocation and bind readiness",
      "image-view and sampler readiness",
      "dirty upload command path readiness",
      "descriptor set binding remains Step 460",
  };
  for (std::size_t index = 0; index < required_docs.size(); ++index) {
    if (!contains(roadmap, required_docs[index]) ||
        !contains(ledger_md, required_docs[index]) ||
        !contains(ledger_json, required_docs[index]) ||
        !contains(task_plan, required_docs[index]) ||
        !contains(findings, required_docs[index])) {
      return 40 + static_cast<int>(index);
    }
  }

  return 0;
}

} // namespace

int main() {
  if (const int result = test_phase_e_glyph_atlas_production_resources();
      result != 0) {
    return result;
  }
  return test_phase_e_glyph_atlas_production_structure_docs();
}
