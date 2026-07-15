#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string read_source(const char* path) {
  const char* root = std::getenv("CGPUI_SOURCE_ROOT");
  std::ifstream source(
      (root == nullptr ? std::string{"."} : root) + "/" + path);
  return source ? std::string{std::istreambuf_iterator<char>(source), {}}
                : std::string{};
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

std::size_t line_count(const std::string& text) {
  std::size_t lines = 0;
  for (char character : text) lines += character == '\n';
  return lines + (!text.empty() && text.back() != '\n');
}

}  // namespace

int main() {
  constexpr const char* files[] = {
      "src/renderer/metal/metal_pipeline_library.mm",
      "src/renderer/metal/metal_geometry.mm",
      "src/renderer/metal/metal_text.mm",
      "src/renderer/metal/metal_image.mm",
      "src/renderer/metal/metal_clip_transform.mm",
      "src/renderer/metal/metal_command_batches.mm",
      "src/renderer/metal/shaders.metal"};
  for (const char* file : files) {
    if (read_source(file).empty()) return 1;
  }
  const std::string internal =
      read_source("src/renderer/metal/metal_renderer_internal.hpp");
  if (line_count(internal) > 240 ||
      !contains(internal, "struct MetalFrameDrawOrderEntry") ||
      !contains(internal, "RendererPrimitiveKind primitive_kind") ||
      !contains(internal, "std::size_t command_index")) return 2;
  const std::string frame = read_source("src/renderer/metal/metal_frame.mm");
  if (line_count(frame) > 320 ||
      contains(frame, "newRenderPipelineStateWithDescriptor") ||
      contains(frame, "newTextureWithDescriptor")) return 3;
  const std::string aggregate = read_source("include/cgpui/renderer/renderer.hpp");
  if (contains(aggregate, "metal_")) return 4;
  const std::string xmake = read_source("xmake.lua");
  const std::string module = read_source("build/xmake/phase_h_metal.lua");
  const std::string targets = read_source("build/xmake/phase_h_structure_targets.lua");
  return contains(xmake, "includes(\"build/xmake/phase_h_metal.lua\")") &&
                 contains(xmake, "includes(\"build/xmake/phase_h_structure_targets.lua\")") &&
                 !module.empty() && !targets.empty() && contains(targets, "target(\"phase_h_metal_primitives_structure_test\")") &&
                 contains(module, "xcrun") && contains(module, "metallib") &&
                 contains(module, "for _, metal_pixel_test in ipairs") &&
                 contains(module, "\"metal_primitive_pixel_test\"") &&
                 contains(module, "\"metal_text_image_pixel_test\"") &&
                 contains(module, "\"metal_clip_transform_pixel_test\"") &&
                 contains(xmake, "includes(\"build/xmake/phase_h_metal.lua\")") &&
                 contains(module, "for _, metal_pixel_test in ipairs")
             ? 0
             : 5;
}
