#include "cgpui/renderer/svg_raster_cache.hpp"

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

constexpr std::string_view red_svg =
    "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"4\" height=\"2\">"
    "<rect width=\"4\" height=\"2\" fill=\"#ff0000\"/></svg>";
constexpr std::string_view green_svg =
    "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"4\" height=\"2\">"
    "<rect width=\"4\" height=\"2\" fill=\"#00ff00\"/></svg>";

cgpui::SvgRasterizationRequest request_for(
    std::string_view source = red_svg,
    cgpui::Size size = {.width = 4.0F, .height = 2.0F},
    float scale = 2.0F) {
  return cgpui::SvgRasterizationRequest{
      .asset_id = cgpui::ImageAssetId{61},
      .logical_size = size,
      .svg_source = source,
      .scale = cgpui::DpiScale{scale},
  };
}

int test_identical_request_reuses_cache_owned_result() {
  cgpui::SvgRasterCache cache;
  const cgpui::SvgRasterizationRequest request = request_for();
  const cgpui::SvgRasterCacheLookup first = cache.rasterize(request);
  if (!first.ready() || first.cache_hit || first.result == nullptr ||
      cache.entry_count() != 1 || cache.lookup_count() != 1 ||
      cache.hit_count() != 0 || cache.miss_count() != 1) {
    return 10;
  }

  const cgpui::SvgRasterizationResult* stored = first.result;
  const cgpui::SvgRasterCacheLookup second = cache.rasterize(request);
  if (!second.ready() || !second.cache_hit || second.result != stored) {
    return 11;
  }
  const cgpui::SvgRasterCacheLookup other =
      cache.rasterize(request_for(green_svg));
  const cgpui::SvgRasterCacheLookup third = cache.rasterize(request);
  return other.ready() && !other.cache_hit && third.ready() &&
                 third.cache_hit && third.result == stored &&
                 stored->image.bitmap.width == 8 &&
                 stored->image.bitmap.height == 4 &&
                 stored->image.bitmap.pixels[0] == 255 &&
                 stored->image.bitmap.pixels[1] == 0 &&
                 cache.entry_count() == 2 && cache.lookup_count() == 4 &&
                 cache.hit_count() == 2 && cache.miss_count() == 2
             ? 0
             : 12;
}

int test_cache_key_preserves_request_inputs() {
  cgpui::SvgRasterCache cache;
  const cgpui::SvgRasterCacheLookup base = cache.rasterize(request_for());
  const cgpui::SvgRasterCacheLookup source =
      cache.rasterize(request_for(green_svg));
  const cgpui::SvgRasterCacheLookup size = cache.rasterize(
      request_for(red_svg, {.width = 5.0F, .height = 2.0F}));
  const cgpui::SvgRasterCacheLookup scale =
      cache.rasterize(
          request_for(red_svg, {.width = 4.0F, .height = 2.0F}, 1.0F));
  return base.ready() && source.ready() && size.ready() && scale.ready() &&
                 !base.cache_hit && !source.cache_hit && !size.cache_hit &&
                 !scale.cache_hit && cache.entry_count() == 4 &&
                 cache.lookup_count() == 4 && cache.hit_count() == 0 &&
                 cache.miss_count() == 4
             ? 0
             : 20;
}

int test_failed_rasterizations_are_not_cached() {
  cgpui::SvgRasterCache cache;
  const cgpui::SvgRasterizationRequest invalid_svg = request_for("<svg><");
  const cgpui::SvgRasterCacheLookup first = cache.rasterize(invalid_svg);
  const cgpui::SvgRasterCacheLookup second = cache.rasterize(invalid_svg);
  const cgpui::SvgRasterCacheLookup invalid_request =
      cache.rasterize(cgpui::SvgRasterizationRequest{});
  return !first.ready() && !second.ready() && !invalid_request.ready() &&
                 !first.cache_hit && !second.cache_hit &&
                 !invalid_request.cache_hit && first.result == nullptr &&
                 second.result == nullptr && invalid_request.result == nullptr &&
                 first.status == cgpui::SvgRasterizationStatus::invalid_svg &&
                 second.status == cgpui::SvgRasterizationStatus::invalid_svg &&
                 invalid_request.status ==
                     cgpui::SvgRasterizationStatus::invalid_request &&
                 cache.entry_count() == 0 && cache.lookup_count() == 3 &&
                 cache.hit_count() == 0 && cache.miss_count() == 3
             ? 0
             : 30;
}

int test_clear_removes_entries_without_resetting_statistics() {
  cgpui::SvgRasterCache cache;
  if (!cache.rasterize(request_for()).ready()) {
    return 40;
  }
  cache.clear();
  if (cache.entry_count() != 0 || cache.lookup_count() != 1 ||
      cache.hit_count() != 0 || cache.miss_count() != 1) {
    return 41;
  }
  const cgpui::SvgRasterCacheLookup after_clear =
      cache.rasterize(request_for());
  return after_clear.ready() && !after_clear.cache_hit &&
                 cache.entry_count() == 1 && cache.lookup_count() == 2 &&
                 cache.hit_count() == 0 && cache.miss_count() == 2
             ? 0
             : 42;
}

int test_structure_and_documentation() {
  const std::string header =
      read_source("include/cgpui/renderer/svg_raster_cache.hpp");
  const std::string source =
      read_source("src/renderer/svg_raster_cache.cpp");
  const std::string aggregate =
      read_source("include/cgpui/renderer/renderer.hpp");
  const std::string structure =
      read_source("tests/architecture/renderer_source_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  if (header.empty() || source.empty() || aggregate.empty() ||
      structure.empty() || xmake.empty()) {
    return 50;
  }
  if (!contains(header, "struct SvgRasterCacheKey") ||
      !contains(header, "struct SvgRasterCacheLookup") ||
      !contains(header, "class SvgRasterCache") ||
      !contains(source, "SvgRasterCache::rasterize(") ||
      !contains(aggregate, "cgpui/renderer/svg_raster_cache.hpp") ||
      contains(aggregate, "class SvgRasterCache") ||
      !contains(structure, "src/renderer/svg_raster_cache.cpp") ||
      !contains(xmake, "target(\"svg_raster_cache_test\")")) {
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
      "Phase E Step 501",
      "SvgRasterCache",
      "cache-owned raster result",
      "asset id, logical size, scale, and SVG source",
      "failed rasterizations are not cached",
      "Step 502 SVG viewport scaling",
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
  if (const int result = test_identical_request_reuses_cache_owned_result();
      result != 0) {
    return result;
  }
  if (const int result = test_cache_key_preserves_request_inputs(); result != 0) {
    return result;
  }
  if (const int result = test_failed_rasterizations_are_not_cached();
      result != 0) {
    return result;
  }
  if (const int result = test_clear_removes_entries_without_resetting_statistics();
      result != 0) {
    return result;
  }
  return test_structure_and_documentation();
}
