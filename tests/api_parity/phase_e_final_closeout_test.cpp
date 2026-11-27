#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string source_root() {
  if (const char* root = CGPUI_SOURCE_ROOT; root != nullptr) {
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
  std::string text{
      std::istreambuf_iterator<char>(source),
      std::istreambuf_iterator<char>()};
  text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());
  return text;
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

std::size_t line_count(const std::string& text) {
  std::size_t lines = 0;
  for (const char character : text) {
    if (character == '\n') {
      ++lines;
    }
  }
  return lines + (!text.empty() && text.back() != '\n' ? 1U : 0U);
}

} // namespace

int main() {
  const std::string manifests = read_source("tests/api_parity/mcpp.toml");
  const std::string presentation =
      read_source("src/renderer/vulkan/vulkan_presentation.cpp");
  const std::string command_recording =
      read_source("src/renderer/vulkan/vulkan_command_recording.cpp");
  const std::string image_staging =
      read_source("src/renderer/vulkan/vulkan_image_texture_staging.cpp");

  if (manifests.empty() ||
      presentation.empty() ||
      command_recording.empty() ||
      image_staging.empty()) {
    return 1;
  }

  if (!contains(manifests, "\"phase_e_final_closeout_test.cpp\"") ||
      !contains(manifests,
                "\"phase_e_final_closeout_test.cpp\"")) {
    return 2;
  }

  constexpr std::array closeout_targets{
      "phase_e_glyph_atlas_integration_closeout_test",
      "phase_e_text_pipeline_integration_closeout_test",
      "phase_e_clip_composition_integration_closeout_test",
      "phase_e_image_integration_closeout_test",
      "phase_e_svg_integration_closeout_test",
      "phase_e_batching_scheduling_integration_closeout_test",
      "phase_e_renderer_diagnostics_integration_closeout_test",
      "phase_e_pixel_output_integration_closeout_test",
  };
  for (std::size_t index = 0; index < closeout_targets.size(); ++index) {
    if (!contains(manifests, closeout_targets[index])) {
      return 10 + static_cast<int>(index);
    }
  }

  if (line_count(presentation) > 180 ||
      line_count(command_recording) > 220 ||
      !contains(presentation, "VulkanRendererState::present_frame(") ||
      !contains(command_recording, "record_vulkan_frame_command_buffer(") ||
      !contains(image_staging, "static_cast<std::uint32_t>(")) {
    return 20;
  }

  return 0;
}
