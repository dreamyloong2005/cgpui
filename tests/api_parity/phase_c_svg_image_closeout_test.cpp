#include <array>
#include <cstddef>
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

} // namespace

int main() {
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  const std::string xmake = read_source("xmake.lua");

  if (roadmap.empty() || ledger_md.empty() || ledger_json.empty() ||
      vocabulary.empty() || xmake.empty()) {
    return 1;
  }

  if (!contains(xmake, "target(\"phase_c_svg_image_closeout_test\")") ||
      !contains(xmake,
                "tests/api_parity/phase_c_svg_image_closeout_test.cpp")) {
    return 2;
  }

  if (!contains(roadmap,
                "- [x] Steps 361-366: Add SVG/image element front-end APIs") ||
      !contains(roadmap,
                "Step 364 closes the SVG/image band through") ||
      !contains(roadmap,
                "tests/api_parity/phase_c_svg_image_closeout_test.cpp") ||
      !contains(roadmap,
                "Step 367 starts widget family structure tests")) {
    return 3;
  }
  if (contains(roadmap, "- [ ] Steps 361-366") ||
      contains(roadmap, "Step 364 SVG/image band closeout is next")) {
    return 4;
  }

  constexpr std::array roadmap_evidence{
      "Phase C Step 361 SVG/image front-end source APIs adds",
      "Phase C Step 362 SVG/image asset registration adds",
      "Phase C Step 363 SVG/image public example coverage adds",
  };
  for (std::size_t index = 0; index < roadmap_evidence.size(); ++index) {
    if (!contains(roadmap, roadmap_evidence[index])) {
      return 10 + static_cast<int>(index);
    }
  }

  constexpr std::array docs_evidence{
      "Phase C Step 361 SVG/image front-end source APIs",
      "ImageSource",
      "ImageSourceKind",
      "svg_image_source(...)",
      "phase_c_svg_image_front_end_test.cpp",
      "Phase C Step 362 SVG/image asset registration",
      "ImageAssetRegistry",
      "RegisteredImageAsset",
      "phase_c_svg_image_asset_registration_test.cpp",
      "Phase C Step 363 SVG/image public example coverage",
      "examples/api_parity/public_svg_image_sources/main.cpp",
      "api_parity_public_svg_image_sources",
      "phase_c_svg_image_public_examples_test.cpp",
      "Phase C Step 364 SVG/image band closeout",
      "phase_c_svg_image_closeout_test.cpp",
  };
  for (std::size_t index = 0; index < docs_evidence.size(); ++index) {
    if (!contains(ledger_md, docs_evidence[index]) ||
        !contains(ledger_json, docs_evidence[index]) ||
        !contains(vocabulary, docs_evidence[index])) {
      return 30 + static_cast<int>(index);
    }
  }

  if (!contains(ledger_json,
                "\"image_front_end_status\": \"phase_c_closed\"") ||
      !contains(ledger_json,
                "\"image_front_end_next\": \"Phase C Step 367 widget "
                "family structure tests\"")) {
    return 60;
  }
  if (contains(ledger_md, "Step 364 SVG/image band closeout is next") ||
      contains(ledger_json,
               "\"next_step\": \"Phase C Step 364 SVG/image band closeout\"") ||
      contains(ledger_json,
               "\"image_front_end_next\": \"Phase C Step 364 SVG/image "
               "band closeout\"") ||
      contains(vocabulary, "Step 364 SVG/image band closeout is the next")) {
    return 61;
  }

  constexpr std::array closeout_exclusions{
      "SVG decoding",
      "PNG/JPEG loading",
      "renderer upload",
      "GPU texture lifetime",
      "private runtime headers",
      "WindowRuntime",
  };
  for (std::size_t index = 0; index < closeout_exclusions.size(); ++index) {
    if (!contains(roadmap, closeout_exclusions[index]) ||
        !contains(vocabulary, closeout_exclusions[index])) {
      return 80 + static_cast<int>(index);
    }
  }

  return 0;
}
