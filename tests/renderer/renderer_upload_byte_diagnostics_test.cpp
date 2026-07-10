#include "cgpui/renderer/renderer_frame_diagnostics.hpp"

#include <array>
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

int test_counts_glyph_and_image_payload_bytes() {
  std::array<cgpui::GlyphAtlasUploadBatch, 2> glyph_batches;
  glyph_batches[0].uploads = {
      cgpui::GlyphAtlasUploadRegion{.byte_size = 7},
      cgpui::GlyphAtlasUploadRegion{.byte_size = 9},
  };
  glyph_batches[1].uploads = {
      cgpui::GlyphAtlasUploadRegion{.byte_size = 5},
  };
  std::array<cgpui::ImageUploadBatch, 1> image_batches;
  image_batches[0].uploads = {
      cgpui::ImageUploadRegion{.byte_size = 40},
      cgpui::ImageUploadRegion{.byte_size = 24},
  };

  const cgpui::RendererUploadByteCounts counts =
      cgpui::renderer_upload_byte_counts(glyph_batches, image_batches);
  return counts.glyph_atlas_byte_count == 21 &&
                 counts.image_byte_count == 64 &&
                 counts.total_byte_count == 85 && !counts.saturated
             ? 0
             : 10;
}

int test_empty_uploads_report_zero() {
  const cgpui::RendererUploadByteCounts counts =
      cgpui::renderer_upload_byte_counts({}, {});
  return counts.glyph_atlas_byte_count == 0 &&
                 counts.image_byte_count == 0 &&
                 counts.total_byte_count == 0 && !counts.saturated
             ? 0
             : 20;
}

int test_upload_byte_count_saturates() {
  std::array<cgpui::GlyphAtlasUploadBatch, 1> glyph_batches;
  glyph_batches[0].uploads = {
      cgpui::GlyphAtlasUploadRegion{
          .byte_size = std::numeric_limits<std::size_t>::max(),
      },
      cgpui::GlyphAtlasUploadRegion{.byte_size = 1},
  };
  const cgpui::RendererUploadByteCounts counts =
      cgpui::renderer_upload_byte_counts(glyph_batches, {});
  return counts.glyph_atlas_byte_count ==
                     std::numeric_limits<std::size_t>::max() &&
                 counts.total_byte_count ==
                     std::numeric_limits<std::size_t>::max() &&
                 counts.saturated
             ? 0
             : 30;
}

int test_upload_byte_comparison() {
  const cgpui::RendererFrameDiagnostics pending =
      cgpui::compare_renderer_frame_work(
          {.upload_byte_count = 100},
          {.upload_byte_count = 60});
  if (pending.exact_match() || pending.pending_upload_byte_count != 40 ||
      pending.unexpected_upload_byte_count != 0) {
    return 40;
  }
  const cgpui::RendererFrameDiagnostics unexpected =
      cgpui::compare_renderer_frame_work(
          {.upload_byte_count = 25},
          {.upload_byte_count = 70});
  return !unexpected.exact_match() &&
                 unexpected.pending_upload_byte_count == 0 &&
                 unexpected.unexpected_upload_byte_count == 45
             ? 0
             : 41;
}

int test_step_516_structure_and_documentation() {
  const std::string header = read_source(
      "include/cgpui/renderer/renderer_frame_diagnostics.hpp");
  const std::string source = read_source(
      "src/renderer/renderer_frame_upload_diagnostics.cpp");
  const std::string structure =
      read_source("tests/architecture/renderer_source_structure_test.cpp");
  if (!contains(header, "struct RendererUploadByteCounts") ||
      !contains(header, "renderer_upload_byte_counts(") ||
      !contains(source, "renderer_upload_byte_counts(") ||
      !contains(structure, "renderer_frame_upload_diagnostics.cpp")) {
    return 50;
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
      "Phase E Step 516",
      "RendererUploadByteCounts",
      "glyph and image upload payload bytes",
      "saturation-safe upload-byte accounting",
      "Step 517 draw-count accounting",
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
  if (const int result = test_counts_glyph_and_image_payload_bytes();
      result != 0) {
    return result;
  }
  if (const int result = test_empty_uploads_report_zero(); result != 0) {
    return result;
  }
  if (const int result = test_upload_byte_count_saturates(); result != 0) {
    return result;
  }
  if (const int result = test_upload_byte_comparison(); result != 0) {
    return result;
  }
  return test_step_516_structure_and_documentation();
}
