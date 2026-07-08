#include <array>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string read_source(const char* path) {
  std::ifstream source(path);
  if (!source) {
    source.open((std::string("../../../../") + path).c_str());
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
  const std::string example =
      read_source("examples/api_parity/public_svg_image_sources/main.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");

  if (example.empty() || xmake.empty() || vocabulary.empty() ||
      roadmap.empty() || ledger_md.empty() || ledger_json.empty()) {
    return 1;
  }

  constexpr std::array xmake_fragments{
      "target(\"api_parity_public_svg_image_sources\")",
      "examples/api_parity/public_svg_image_sources/main.cpp",
      "target(\"phase_c_svg_image_public_examples_test\")",
      "tests/api_parity/phase_c_svg_image_public_examples_test.cpp",
  };
  for (std::size_t index = 0; index < xmake_fragments.size(); ++index) {
    if (!contains(xmake, xmake_fragments[index])) {
      return 10 + static_cast<int>(index);
    }
  }

  constexpr std::array required_example_fragments{
      "#include \"cgpui/prelude.hpp\"",
      "class PublicSvgImageSourcesView",
      "cgpui::ImageAssetRegistry registry",
      "registry.register_image(",
      "registry.register_svg(",
      "cgpui::RegisteredImageAsset",
      "cgpui::ImageSourceKind::image",
      "cgpui::ImageSourceKind::svg",
      "registry.raster_assets()",
      "cgpui::image(raster_registration.source())",
      "cgpui::svg(svg_registration.source())",
      ".alt(\"Registered raster source\")",
      ".alt(\"Registered SVG source\")",
      "CGPUI_RUN_PUBLIC_SVG_IMAGE_SOURCES",
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
      "ClipboardItem",
      "gpui::test",
      "TaskPriority",
      "StructuredTaskGroup",
      ".runtime",
      "runtime.",
      "window_runtime",
  };
  for (std::size_t index = 0; index < forbidden_example_fragments.size();
       ++index) {
    if (contains(example, forbidden_example_fragments[index])) {
      return 70 + static_cast<int>(index);
    }
  }

  constexpr std::array docs_fragments{
      "Step 363 SVG/image public example",
      "public_svg_image_sources",
      "`api_parity_public_svg_image_sources`",
      "\"step_363\"",
      "\"public_svg_image_sources\"",
      "\"image_front_end_next\": \"Phase C Step 367 widget family structure tests\"",
      "Phase C Step 364 SVG/image band closeout",
  };
  const std::array docs_sources{
      roadmap,
      vocabulary,
      ledger_md,
      ledger_json,
      ledger_json,
      ledger_json,
      roadmap,
  };
  for (std::size_t index = 0; index < docs_fragments.size(); ++index) {
    if (!contains(docs_sources[index], docs_fragments[index])) {
      return 90 + static_cast<int>(index);
    }
  }

  return 0;
}
