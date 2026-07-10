#include <array>
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

} // namespace

int main() {
  const std::string example = read_source(
      "examples/api_parity/public_svg_raster_upload/main.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  if (example.empty() || xmake.empty() || roadmap.empty() ||
      ledger_md.empty() || ledger_json.empty() || vocabulary.empty() ||
      task_plan.empty() || findings.empty()) {
    return 1;
  }

  constexpr std::array xmake_fragments{
      "target(\"api_parity_public_svg_raster_upload\")",
      "examples/api_parity/public_svg_raster_upload/main.cpp",
      "target(\"phase_e_svg_public_example_test\")",
      "tests/api_parity/phase_e_svg_public_example_test.cpp",
  };
  for (std::size_t index = 0; index < xmake_fragments.size(); ++index) {
    if (!contains(xmake, xmake_fragments[index])) {
      return 10 + static_cast<int>(index);
    }
  }

  constexpr std::array required_example_fragments{
      "#include \"cgpui/prelude.hpp\"",
      "class PublicSvgUploadFrame",
      "public cgpui::RenderFrame",
      "cgpui::ImageAssetRegistry registry",
      "registry.register_svg(",
      "cgpui::RegisteredImageAsset",
      "registration.svg_source()",
      "cgpui::SvgRasterizationRequest",
      ".viewport_size =",
      ".current_color =",
      "cgpui::SvgRasterCache cache",
      "cgpui::upload_svg_image(",
      "cgpui::SvgImageUploadResult first",
      "cgpui::SvgImageUploadResult repeated",
      "repeated.cache_hit",
      "frame.draw_image(cgpui::ImageDraw{",
      ".asset = repeated.image",
      "static_assert(std::is_same_v<",
  };
  for (std::size_t index = 0; index < required_example_fragments.size();
       ++index) {
    if (!contains(example, required_example_fragments[index])) {
      return 30 + static_cast<int>(index);
    }
  }

  constexpr std::array forbidden_example_fragments{
      "#include \"cgpui/ui/",
      "#include \"cgpui/platform/",
      "#include \"cgpui/renderer/",
      "#include \"src/",
      "#include \"../",
      "WindowRuntime",
      "vulkan_plan_image_uploads(",
      "VulkanFrame",
      "runtime.",
      ".runtime",
  };
  for (std::size_t index = 0; index < forbidden_example_fragments.size();
       ++index) {
    if (contains(example, forbidden_example_fragments[index])) {
      return 60 + static_cast<int>(index);
    }
  }

  constexpr std::array vocabulary_fragments{
      "`SvgRasterizationRequest`",
      "`SvgRasterCache`",
      "`SvgImageUploadResult`",
      "`upload_svg_image(...)`",
  };
  for (std::size_t index = 0; index < vocabulary_fragments.size(); ++index) {
    if (!contains(vocabulary, vocabulary_fragments[index])) {
      return 70 + static_cast<int>(index);
    }
  }

  constexpr std::array required_docs{
      "Phase E Step 505",
      "public_svg_raster_upload",
      "registered SVG source",
      "viewport-aware raster request",
      "cached upload and image draw",
      "Step 506 SVG integration closeout",
  };
  for (const char* value : required_docs) {
    if (!contains(roadmap, value) || !contains(ledger_md, value) ||
        !contains(ledger_json, value) || !contains(task_plan, value) ||
        !contains(findings, value)) {
      return 80;
    }
  }
  return 0;
}
