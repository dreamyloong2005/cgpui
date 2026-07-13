#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string read_source(const char* path) {
  const char* root = std::getenv("CGPUI_SOURCE_ROOT");
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
  if (!source) source.open(path);
  return source ? std::string{std::istreambuf_iterator<char>(source), {}}
                : std::string{};
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

std::size_t line_count(const std::string& text) {
  std::size_t lines = 0;
  for (const char value : text) lines += value == '\n' ? 1U : 0U;
  return lines + (!text.empty() && text.back() != '\n' ? 1U : 0U);
}

} // namespace

int main() {
  const std::string header =
      read_source("include/cgpui/renderer/svg_asset_decode.hpp");
  const std::string decoder =
      read_source("src/renderer/svg_asset_decode_lunasvg.cpp");
  const std::string bridge =
      read_source("src/renderer/svg_asset_rasterization.cpp");
  const std::string raster_header =
      read_source("include/cgpui/renderer/svg_rasterization.hpp");
  const std::string raster =
      read_source("src/renderer/svg_rasterization.cpp");
  const std::string raster_backend =
      read_source("src/renderer/svg_rasterization_lunasvg.cpp");
  const std::string behavior =
      read_source("tests/assets/svg_asset_decode_test.cpp");
  const std::string prelude = read_source("include/cgpui/prelude.hpp");
  const std::string cleanliness =
      read_source("tests/header_cleanliness/prelude_header_cleanliness.cpp");
  const std::string inventory =
      read_source("tests/architecture/renderer_source_structure_test.cpp");
  const std::string previous = read_source(
      "tests/architecture/phase_g_gif_decode_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  const std::string core = read_source("docs/gpui-core-api-parity.md");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &header, &decoder, &bridge, &raster_header, &raster, &raster_backend,
      &behavior, &prelude, &cleanliness, &inventory, &previous, &xmake,
      &vocabulary, &core, &roadmap, &ledger_md, &ledger_json, &task_plan,
      &findings};
  for (const auto* item : required) if (item->empty()) return 1;

  if (!contains(header, "enum class SvgAssetDecodeStatus") ||
      !contains(header, "struct SvgAssetDecodeLimits") ||
      !contains(header, "struct DecodedSvgAsset") ||
      !contains(header, "struct SvgAssetDecodeResult") ||
      !contains(header, "struct SvgAssetRasterizationOptions") ||
      !contains(header, "decode_svg_asset(") ||
      !contains(header, "rasterize_svg_asset(") ||
      contains(header, "lunasvg")) return 2;
  if (!contains(decoder, "lunasvg::Document::loadFromData(") ||
      !contains(decoder, "document->width()") ||
      !contains(decoder, "document->height()") ||
      !contains(decoder, "encoded.size() > limits.max_encoded_bytes") ||
      !contains(decoder, "limits.max_intrinsic_pixels") ||
      decoder.find("encoded.size() > limits.max_encoded_bytes") >
          decoder.find("std::string source(")) return 3;
  if (!contains(bridge, "return rasterize_svg(SvgRasterizationRequest{") ||
      !contains(bridge, ".svg_source = asset.source") ||
      !contains(bridge, ".logical_size = asset.intrinsic_size") ||
      !contains(raster_header, "struct SvgRasterizationResult") ||
      !contains(raster, "plan_svg_rasterization(") ||
      !contains(raster_backend, "renderToBitmap(")) return 4;
  if (!contains(behavior, "test_decodes_asset_bytes_and_rasterizes") ||
      !contains(behavior, "test_rejects_invalid_and_oversized_assets") ||
      !contains(behavior, "AssetBytes") ||
      !contains(behavior, "max_encoded_bytes = 8") ||
      !contains(behavior, "max_intrinsic_dimension = 3.0F") ||
      !contains(cleanliness, "SvgAssetDecodeStatus::empty_input")) return 5;
  if (!contains(prelude, "cgpui/renderer/svg_asset_decode.hpp") ||
      !contains(inventory, "include/cgpui/renderer/svg_asset_decode.hpp") ||
      !contains(inventory, "src/renderer/svg_asset_decode_lunasvg.cpp") ||
      !contains(inventory, "src/renderer/svg_asset_rasterization.cpp") ||
      !contains(previous, "Step 654 SVG asset decode boundary production ") ||
      !contains(previous, "behavior is next.") ||
      !contains(xmake, "target(\"phase_g_svg_asset_decode_test\")") ||
      !contains(xmake,
                "target(\"phase_g_svg_asset_decode_structure_test\")")) {
    return 6;
  }
  if (line_count(header) > 65 || line_count(decoder) > 75 ||
      line_count(bridge) > 35 || line_count(behavior) > 100) return 7;

  constexpr const char* completion =
      "Phase G Step 654 adds bounded SVG asset decoding from AssetBytes with "
      "LunaSVG intrinsic-size validation and a lifetime-safe bridge into the "
      "existing viewport-aware, recolorable RGBA8 rasterization path. Step "
      "655 asset cache key production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 8;
  }
  if (!contains(vocabulary, "`decode_svg_asset(...)`") ||
      !contains(vocabulary, "`rasterize_svg_asset(...)`") ||
      !contains(core, "Bounded SVG asset decoding") ||
      !contains(ledger_json, "\"phase_g_step_654_sources\"") ||
      !contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 669 Windows and Linux release build and packaging coverage\"")) return 9;
  return 0;
}
