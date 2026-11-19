#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string read_source(const char* path) {
  const char* root = CGPUI_SOURCE_ROOT;
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
  const std::string root = read_source("src/renderer/metal/metal_renderer.mm");
  if (root.empty() || line_count(root) > 60 ||
      contains(root, "class MetalRenderer") ||
      contains(root, "class MetalFrame") ||
      contains(root, " retain]") || contains(root, " release]")) return 1;

  constexpr const char* files[] = {
      "src/renderer/metal/metal_renderer_internal.hpp",
      "src/renderer/metal/metal_device.mm",
      "src/renderer/metal/metal_surface.mm",
      "src/renderer/metal/metal_frame.mm",
      "src/renderer/metal/metal_frame_pacing.mm",
      "src/renderer/metal/metal_diagnostics.mm",
  };
  for (const char* file : files) {
    const std::string contents = read_source(file);
    if (contents.empty() || contains(contents, " retain]") ||
        contains(contents, " release]")) return 2;
  }

  const std::string internal = read_source(
      "src/renderer/metal/metal_renderer_internal.hpp");
  if (!contains(internal, "struct MetalRendererState") ||
      !contains(internal, "class MetalFramePacingState") ||
      !contains(internal, "__strong CAMetalLayer* layer") ||
      !contains(internal, "next_frame_id") ||
      !contains(internal, "RendererFrameDiagnosticSnapshot")) return 3;

  const std::string frame = read_source("src/renderer/metal/metal_frame.mm");
  if (!contains(frame, "renderCommandEncoderWithDescriptor") ||
      !contains(frame, "presentDrawable") || !contains(frame, "commit")) {
    return 4;
  }

  const std::string manifests = read_source("mcpp.toml");
  const std::string module = read_source("tests/renderer/mcpp.toml") +
      read_source("packages/renderer-metal/mcpp.toml");
  const std::string targets = read_source("tests/architecture/mcpp.toml");
  if (!contains(manifests, "\"packages/renderer-metal\",") ||
      !contains(manifests, "\"tests/architecture\",") ||
      module.empty() ||
      !contains(module, "\"metal/metal_*.mm\"") ||
      !contains(module, "\"../../src/renderer/metal/*.mm\"") ||
      !contains(module, "\"metal/metal_*.mm\"") ||
      !contains(module, "include_dirs = [\"../../include\"]") ||
      targets.empty() || !contains(targets, "\"phase_h_metal_bootstrap_structure_test.cpp\"")) return 6;
  return 0;
}
