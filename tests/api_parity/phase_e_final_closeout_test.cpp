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
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string presentation =
      read_source("src/renderer/vulkan/vulkan_presentation.cpp");
  const std::string command_recording =
      read_source("src/renderer/vulkan/vulkan_command_recording.cpp");
  const std::string image_staging =
      read_source("src/renderer/vulkan/vulkan_image_texture_staging.cpp");

  if (xmake.empty() || roadmap.empty() || ledger_md.empty() ||
      ledger_json.empty() || task_plan.empty() || findings.empty() ||
      presentation.empty() || command_recording.empty() ||
      image_staging.empty()) {
    return 1;
  }

  if (!contains(xmake, "target(\"phase_e_final_closeout_test\")") ||
      !contains(xmake,
                "tests/api_parity/phase_e_final_closeout_test.cpp")) {
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
    if (!contains(xmake, closeout_targets[index])) {
      return 10 + static_cast<int>(index);
    }
  }

  if (line_count(presentation) > 165 ||
      line_count(command_recording) > 180 ||
      !contains(presentation, "VulkanRendererState::present_frame(") ||
      !contains(command_recording, "record_vulkan_frame_command_buffer(") ||
      !contains(image_staging, "static_cast<std::uint32_t>(")) {
    return 20;
  }

  if (!contains(roadmap,
                "- [x] Steps 531-538: Run full Windows/WSL verification") ||
      contains(roadmap,
               "- [ ] Steps 531-538: Run full Windows/WSL verification")) {
    return 30;
  }

  constexpr const char* final_closeout =
      "Phase E final closeout: Steps 531-538 close with "
      "`tests/api_parity/phase_e_final_closeout_test.cpp`; Windows full "
      "debug suite passes 214/214 and WSL Arch Linux full debug suite "
      "passes 204/204, including active-display Wayland pixel capture on "
      "`WAYLAND_DISPLAY=wayland-0`, using D-drive WSL build/cache output "
      "plus `/dev/shm/cgpui` transient temp. The required Windows/Linux "
      "Vulkan renderer production path is complete for Phase E primitives, "
      "and Phase F Step 539 window lifecycle production depth is next.";
  const std::array documents{
      &roadmap,
      &ledger_md,
      &ledger_json,
      &task_plan,
      &findings,
  };
  for (std::size_t index = 0; index < documents.size(); ++index) {
    if (!contains(*documents[index], final_closeout)) {
      return 40 + static_cast<int>(index);
    }
  }

  if (!contains(ledger_json,
                "\"phase_e_current_handoff\": \"Phase F Step 539") ||
      !contains(task_plan,
                "- Status: complete\n- Authoritative scope: Phase E") ||
      contains(task_plan, "- In progress: Step 533 WSL full verification.")) {
    return 50;
  }

  return 0;
}
