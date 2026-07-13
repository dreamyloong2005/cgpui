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

} // namespace

int main() {
  const std::string image = read_source(
      "examples/api_parity/public_image_example/main.cpp");
  const std::string gif = read_source(
      "examples/api_parity/public_gif_viewer_example/main.cpp");
  const std::string pin = read_source("docs/gpui-upstream-pinned-revision.md");
  const std::string xmake = read_source("xmake.lua");
  if (image.empty() || gif.empty() || pin.empty() || xmake.empty()) return 1;

  if (!contains(image, "class PublicImageExampleView") ||
      !contains(image, "FileAssetSource") ||
      !contains(image, "load_asset_async(") ||
      !contains(image, "AssetReloadState") ||
      !contains(image, "AssetCacheKind::raster_image") ||
      !contains(image, "decode_image(") ||
      !contains(image, "ImageAssetRegistry") ||
      !contains(image, "paint.invalidate_image(image.id)") ||
      !contains(image, "paint.upload_image(image)") ||
      !contains(image, "paint.draw_image(")) return 2;
  if (!contains(gif, "class PublicGifViewerExampleView") ||
      !contains(gif, "load_asset_async(") ||
      !contains(gif, "AssetCacheKind::animated_gif") ||
      !contains(gif, "decode_gif(") ||
      !contains(gif, "decoded.gif.frames") ||
      !contains(gif, "decoded_frame.duration_ms") ||
      !contains(gif, "loop_.infinite") ||
      !contains(gif, "loop_.repeat_count") ||
      !contains(gif, "schedule_timer(") ||
      !contains(gif, "paint.invalidate_image(frame.id)") ||
      !contains(gif, "paint.upload_image(frame)")) return 3;
  if (!contains(pin, "examples/image/image.rs") ||
      !contains(pin, "examples/gif_viewer.rs") ||
      !contains(pin, "5a823cf70ebb1d7a158c6a7ca455860cd9f6aed0") ||
      !contains(xmake, "target(\"api_parity_public_image_example\")") ||
      !contains(xmake, "target(\"api_parity_public_gif_viewer_example\")") ||
      !contains(xmake,
                "target(\"phase_g_official_image_gif_examples_test\")")) {
    return 4;
  }
  const char* forbidden[]{
      "#include \"cgpui/ui/", "#include \"cgpui/platform/",
      "#include \"cgpui/renderer/", "#include \"src/", "WindowRuntime",
      "std::thread", "std::jthread", "std::async", "http://", "https://"};
  for (const char* value : forbidden) {
    if (contains(image, value) || contains(gif, value)) return 5;
  }
  return 0;
}
