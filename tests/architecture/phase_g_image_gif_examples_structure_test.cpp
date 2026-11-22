#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string read_source(const char* path) {
  const char* root = CGPUI_SOURCE_ROOT;
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
  const std::string header = read_source("include/cgpui/ui/paint.hpp");
  const std::string transport = read_source("src/ui/paint_image_assets.cpp");
  const std::string paint = read_source("src/ui/paint.cpp");
  const std::string render_view = read_source("src/ui/render_view.cpp");
  const std::string transport_test =
      read_source("tests/ui/render_view_image_asset_updates_test.cpp");
  const std::string image = read_source(
      "examples/api_parity/public_image_example/main.cpp");
  const std::string gif = read_source(
      "examples/api_parity/public_gif_viewer_example/main.cpp");
  const std::string behavior = read_source(
      "tests/api_parity/phase_g_official_image_gif_examples_test.cpp");
  const std::string manifests = read_source("tests/ui/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  const std::string core = read_source("docs/gpui-core-api-parity.md");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string* required[]{
      &header, &transport, &paint, &render_view, &transport_test, &image, &gif,
      &behavior, &manifests, &vocabulary, &core, &roadmap, &ledger_json};
  for (const auto* item : required) if (item->empty()) return 1;

  if (!contains(header, "void upload_image(const ImageAsset& image)") ||
      !contains(header, "void upload_image(ImageAsset&& image) = delete") ||
      !contains(header, "void invalidate_image(ImageAssetId asset_id)") ||
      !contains(header, "std::vector<const ImageAsset*> image_uploads_") ||
      contains(header, "std::vector<ImageAsset> image_uploads_")) return 2;
  if (!contains(transport, "PaintList::upload_image(") ||
      !contains(transport, "PaintList::invalidate_image(") ||
      !contains(transport, "image_uploads_.push_back(&image)") ||
      !contains(transport, "image_invalidations_.push_back(asset_id)") ||
      contains(transport, "std::thread") || contains(transport, "std::async")) {
    return 3;
  }
  if (!contains(paint, "image_uploads_.clear()") ||
      !contains(paint, "image_invalidations_.clear()") ||
      !contains(transport_test, "reusable.clear()") ||
      !contains(transport_test, "image_invalidations().empty()") ||
      !contains(transport_test, "image_uploads().empty()")) return 3;
  const std::size_t invalidations =
      render_view.find("paint_list.image_invalidations()");
  const std::size_t uploads = render_view.find("paint_list.image_uploads()");
  const std::size_t commands = render_view.find(
      "for (const auto& command : paint_list.commands())");
  if (invalidations == std::string::npos || uploads == std::string::npos ||
      commands == std::string::npos || invalidations >= uploads ||
      uploads >= commands ||
      !contains(transport_test, "invalidated_id != cgpui::ImageAssetId{41}") ||
      !contains(transport_test, "std::vector<int>({1, 2, 3, 4, 5})")) return 4;
  if (!contains(image, "#include \"cgpui/prelude.hpp\"") ||
      !contains(gif, "#include \"cgpui/prelude.hpp\"") ||
      !contains(image, "CGPUI_RUN_PUBLIC_IMAGE_EXAMPLE") ||
      !contains(gif, "CGPUI_RUN_PUBLIC_GIF_VIEWER_EXAMPLE") ||
      !contains(manifests, "\"render_view_image_asset_updates_test.cpp\"") ||
      !contains(manifests,
                "\"phase_g_image_gif_examples_structure_test.cpp\"")) {
    return 5;
  }
  if (line_count(transport) > 55 || line_count(render_view) > 113 ||
      line_count(transport_test) > 135 || line_count(image) > 190 ||
      line_count(gif) > 210 || line_count(behavior) > 100) return 6;
  if (!contains(roadmap, "[x] Steps 651-658: Complete assets") ||
      !contains(vocabulary, "`PaintList::upload_image(...)`") ||
      !contains(vocabulary, "public_gif_viewer_example") ||
      !contains(core, "Frame-local `PaintList` invalidation/upload records") ||
      !contains(ledger_json, "\"phase_g_step_658_sources\"")) return 8;
  return 0;
}
