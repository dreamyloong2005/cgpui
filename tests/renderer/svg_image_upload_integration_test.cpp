#include "cgpui/renderer/renderer.hpp"
#include "cgpui/renderer/svg_image_upload.hpp"

#include <cstdlib>
#include <fstream>
#include <iterator>
#include <limits>
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

class CapturingFrame final : public cgpui::RenderFrame {
 public:
  void clear(cgpui::Color) override {}
  void draw_rect(const cgpui::SolidRect&) override {}
  void upload_image(const cgpui::ImageAsset& image) override {
    uploads.push_back(image);
  }
  cgpui::Result<void> present() override { return {}; }

  std::vector<cgpui::ImageAsset> uploads;
};

constexpr std::string_view svg_source =
    "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"3\" height=\"2\">"
    "<rect width=\"3\" height=\"2\" fill=\"currentColor\"/></svg>";

cgpui::SvgRasterizationRequest request_for(cgpui::ImageAssetId asset_id) {
  return cgpui::SvgRasterizationRequest{
      .asset_id = asset_id,
      .logical_size = {.width = 3.0F, .height = 2.0F},
      .svg_source = svg_source,
      .scale = cgpui::DpiScale{2.0F},
      .current_color =
          cgpui::Color{.r = 1.0F, .g = 0.25F, .b = 0.0F, .a = 1.0F},
  };
}

int test_rasterized_svg_is_submitted_to_render_frame() {
  cgpui::SvgRasterCache cache;
  CapturingFrame frame;
  const cgpui::SvgImageUploadResult result =
      cgpui::upload_svg_image(frame, cache, request_for({91}));
  if (!result.ready() || result.cache_hit || !result.uploaded ||
      frame.uploads.size() != 1 || result.image.id.value != 91 ||
      result.image.pixel_width != 6 || result.image.pixel_height != 4 ||
      result.image.byte_size != 96) {
    return 10;
  }

  const cgpui::ImageAsset& uploaded = frame.uploads[0];
  return uploaded.id.value == 91 && uploaded.bitmap.width == 6 &&
                 uploaded.bitmap.height == 4 && uploaded.bitmap.stride == 24 &&
                 uploaded.bitmap.pixels.size() == 96 &&
                 uploaded.bitmap.pixels[0] == 255 &&
                 uploaded.bitmap.pixels[1] == 64 &&
                 uploaded.bitmap.pixels[2] == 0 &&
                 uploaded.bitmap.pixels[3] == 255
             ? 0
             : 11;
}

int test_cache_hit_reuses_raster_and_resubmits_upload() {
  cgpui::SvgRasterCache cache;
  CapturingFrame frame;
  const cgpui::SvgImageUploadResult first =
      cgpui::upload_svg_image(frame, cache, request_for({92}));
  const cgpui::SvgImageUploadResult repeated =
      cgpui::upload_svg_image(frame, cache, request_for({92}));
  return first.ready() && repeated.ready() && !first.cache_hit &&
                 repeated.cache_hit && frame.uploads.size() == 2 &&
                 cache.entry_count() == 1 && cache.lookup_count() == 2 &&
                 cache.hit_count() == 1 && cache.miss_count() == 1 &&
                 frame.uploads[0].bitmap.pixels == frame.uploads[1].bitmap.pixels
             ? 0
             : 20;
}

int test_failed_rasterization_does_not_upload() {
  cgpui::SvgRasterCache cache;
  CapturingFrame frame;
  cgpui::SvgRasterizationRequest request = request_for({93});
  request.current_color = cgpui::Color{
      .r = std::numeric_limits<float>::infinity(),
      .g = 0.0F,
      .b = 0.0F,
      .a = 1.0F,
  };
  const cgpui::SvgImageUploadResult result =
      cgpui::upload_svg_image(frame, cache, request);
  return !result.ready() && !result.uploaded && !result.cache_hit &&
                 result.status == cgpui::SvgRasterizationStatus::invalid_request &&
                 frame.uploads.empty() && cache.entry_count() == 0
             ? 0
             : 30;
}

int test_structure_and_documentation() {
  const std::string header =
      read_source("include/cgpui/renderer/svg_image_upload.hpp");
  const std::string source =
      read_source("src/renderer/svg_image_upload.cpp");
  const std::string aggregate =
      read_source("include/cgpui/renderer/renderer.hpp");
  const std::string structure =
      read_source("tests/architecture/renderer_source_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  if (header.empty() || source.empty() || aggregate.empty() ||
      structure.empty() || xmake.empty()) {
    return 40;
  }
  if (!contains(header, "struct SvgImageUploadResult") ||
      !contains(source, "upload_svg_image(") ||
      !contains(source, "frame.upload_image(") ||
      !contains(aggregate, "cgpui/renderer/svg_image_upload.hpp") ||
      !contains(structure, "src/renderer/svg_image_upload.cpp") ||
      !contains(xmake, "target(\"svg_image_upload_integration_test\")")) {
    return 41;
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
      "Phase E Step 504",
      "SvgImageUploadResult",
      "cache-owned raster ImageAsset",
      "RenderFrame::upload_image(...) integration",
      "failed rasterization skips upload",
      "Step 505 SVG public example",
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
  if (const int result = test_rasterized_svg_is_submitted_to_render_frame();
      result != 0) {
    return result;
  }
  if (const int result = test_cache_hit_reuses_raster_and_resubmits_upload();
      result != 0) {
    return result;
  }
  if (const int result = test_failed_rasterization_does_not_upload();
      result != 0) {
    return result;
  }
  return test_structure_and_documentation();
}
