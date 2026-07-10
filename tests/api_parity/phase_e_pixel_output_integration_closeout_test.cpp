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
  std::ifstream source(source_root() + "/" + path);
  if (!source) {
    source.open(path);
  }
  return source ? std::string{
                      std::istreambuf_iterator<char>(source),
                      std::istreambuf_iterator<char>()}
                : std::string{};
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

struct SourceEvidence {
  const char* path;
  const char* needle;
};

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
  if (xmake.empty() || roadmap.empty() || ledger_md.empty() ||
      ledger_json.empty() || task_plan.empty() || findings.empty()) {
    return 1;
  }

  if (!contains(xmake,
                "target(\"phase_e_pixel_output_integration_closeout_test\")") ||
      !contains(
          xmake,
          "tests/api_parity/phase_e_pixel_output_integration_closeout_test.cpp")) {
    return 2;
  }

  constexpr std::array target_evidence{
      "target(\"renderer_frame_pixels_test\")",
      "target(\"vulkan_frame_pixel_capture_test\")",
      "for _, pixel_case in ipairs({",
      "target(\"vulkan_\" .. pixel_case .. \"_pixel_test\")",
      "target(\"wayland_frame_pixel_capture_test\")",
      "target(\"renderer_source_structure_test\")",
  };
  for (std::size_t index = 0; index < target_evidence.size(); ++index) {
    if (!contains(xmake, target_evidence[index])) {
      return 10 + static_cast<int>(index);
    }
  }

  constexpr std::array module_evidence{
      SourceEvidence{
          "src/renderer/renderer_frame_pixels.cpp",
          "RendererFramePixels::pixel_rgba8("},
      SourceEvidence{
          "src/renderer/vulkan/vulkan_frame_pixel_capture_recording.cpp",
          "vkCmdCopyImageToBuffer"},
      SourceEvidence{
          "src/renderer/vulkan/vulkan_frame_pixel_capture_readback.cpp",
          "vkMapMemory"},
      SourceEvidence{
          "src/renderer/vulkan/shaders/text.vert.glsl",
          "normalized.y * 2.0 - 1.0"},
      SourceEvidence{
          "src/renderer/vulkan/shaders/rounded_rect.vert.glsl",
          "normalized.y * 2.0 - 1.0"},
      SourceEvidence{
          "src/renderer/vulkan/shaders/image.vert.glsl",
          "normalized.y * 2.0 - 1.0"},
  };
  for (std::size_t index = 0; index < module_evidence.size(); ++index) {
    const std::string source = read_source(module_evidence[index].path);
    if (source.empty() || !contains(source, module_evidence[index].needle)) {
      return 20 + static_cast<int>(index);
    }
  }

  constexpr std::array behavior_evidence{
      SourceEvidence{
          "tests/renderer/renderer_frame_pixels_test.cpp",
          "test_pixel_snapshot_access"},
      SourceEvidence{
          "tests/renderer/vulkan_frame_pixel_capture_test.cpp",
          "test_captures_presented_clear_color"},
      SourceEvidence{
          "tests/renderer/vulkan_text_pixel_test.cpp", "draw_text("},
      SourceEvidence{
          "tests/renderer/vulkan_rounded_rect_pixel_test.cpp",
          "draw_rounded_rect("},
      SourceEvidence{
          "tests/renderer/vulkan_image_pixel_test.cpp", "draw_image("},
      SourceEvidence{
          "tests/renderer/vulkan_clip_pixel_test.cpp", ".clip_rect"},
      SourceEvidence{
          "tests/renderer/vulkan_transform_pixel_test.cpp",
          "AffineTransform::translation("},
      SourceEvidence{
          "tests/renderer/vulkan_opacity_pixel_test.cpp", "encoded_channel("},
      SourceEvidence{
          "tests/renderer/vulkan_resize_pixel_test.cpp", "resize_client("},
      SourceEvidence{
          "tests/renderer/wayland_frame_pixel_capture_test.cpp",
          "request_pixel_capture()"},
      SourceEvidence{
          "tests/architecture/renderer_source_structure_test.cpp",
          "PixelOutputSourceLimit"},
  };
  for (std::size_t index = 0; index < behavior_evidence.size(); ++index) {
    const std::string source = read_source(behavior_evidence[index].path);
    if (source.empty() || !contains(source, behavior_evidence[index].needle)) {
      return 40 + static_cast<int>(index);
    }
  }

  constexpr std::array required_docs{
      "Phase E Step 531",
      "pixel-output integration band",
      "Steps 523-530",
      "real Win32 Vulkan text/rounded/image/clip/transform/opacity/resize pixels",
      "active-display Wayland capture target",
      "Step 532 Windows full verification",
  };
  for (std::size_t index = 0; index < required_docs.size(); ++index) {
    if (!contains(roadmap, required_docs[index]) ||
        !contains(ledger_md, required_docs[index]) ||
        !contains(ledger_json, required_docs[index]) ||
        !contains(task_plan, required_docs[index]) ||
        !contains(findings, required_docs[index])) {
      return 70 + static_cast<int>(index);
    }
  }

  return contains(
             roadmap,
             "- [x] Phase E Step 531 closes the pixel-output integration band")
             ? 0
             : 80;
}
