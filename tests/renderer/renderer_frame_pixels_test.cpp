#include "cgpui/renderer/renderer.hpp"

#include <cstdlib>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>

namespace {

class DefaultFrame final : public cgpui::RenderFrame {
 public:
  void clear(cgpui::Color) override {}
  void draw_rect(const cgpui::SolidRect&) override {}
  cgpui::Result<void> present() override { return {}; }
};

class DefaultRenderer final : public cgpui::Renderer {
 public:
  cgpui::Result<void> resize(cgpui::Size, cgpui::DpiScale) override {
    return {};
  }

  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    return std::make_unique<DefaultFrame>();
  }
};

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

int test_pixel_snapshot_access() {
  const cgpui::RendererFramePixels pixels{
      .width = 2,
      .height = 1,
      .encoding = cgpui::RendererFramePixelEncoding::srgb,
      .rgba8 = {255, 0, 0, 255, 0, 255, 0, 255},
  };
  if (!pixels.valid() || pixels.row_byte_count() != 8) {
    return 10;
  }
  const auto red = pixels.pixel_rgba8(0, 0);
  const auto green = pixels.pixel_rgba8(1, 0);
  const auto outside = pixels.pixel_rgba8(2, 0);
  return red.has_value() && *red == std::array<std::uint8_t, 4>{255, 0, 0, 255} &&
                 green.has_value() &&
                 *green == std::array<std::uint8_t, 4>{0, 255, 0, 255} &&
                 !outside.has_value()
             ? 0
             : 11;
}

int test_invalid_snapshot_and_compatible_defaults() {
  const cgpui::RendererFramePixels invalid{
      .width = 2,
      .height = 1,
      .rgba8 = {255, 0, 0, 255},
  };
  if (invalid.valid() || invalid.row_byte_count() != 8 ||
      invalid.pixel_rgba8(0, 0).has_value()) {
    return 20;
  }
  DefaultFrame frame;
  const cgpui::Result<void> request = frame.request_pixel_capture();
  if (request || request.error().code != cgpui::ErrorCode::unsupported_platform) {
    return 21;
  }
  const DefaultRenderer renderer;
  return renderer.last_frame_pixels() == nullptr ? 0 : 22;
}

int test_step_523_structure_and_documentation() {
  const std::string public_header =
      read_source("include/cgpui/renderer/renderer_frame_pixels.hpp");
  const std::string public_source =
      read_source("src/renderer/renderer_frame_pixels.cpp");
  const std::string private_header = read_source(
      "src/renderer/vulkan/vulkan_frame_pixel_capture_internal.hpp");
  const std::string capture_source = read_source(
      "src/renderer/vulkan/vulkan_frame_pixel_capture_recording.cpp");
  const std::string live_test =
      read_source("tests/renderer/vulkan_frame_pixel_capture_test.cpp");
  if (!contains(public_header, "struct RendererFramePixels") ||
      !contains(public_source, "RendererFramePixels::valid()") ||
      !contains(private_header, "struct VulkanFramePixelCaptureResources") ||
      !contains(capture_source, "vkCmdCopyImageToBuffer") ||
      !contains(live_test, "request_pixel_capture()")) {
    return 50;
  }

  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const char* required[] = {
      "Phase E Step 523",
      "RendererFramePixels",
      "explicit per-frame capture",
      "normalized RGBA8",
      "Step 524 text pixel coverage",
  };
  for (const char* value : required) {
    if (!contains(roadmap, value) || !contains(ledger_md, value) ||
        !contains(ledger_json, value) || !contains(task_plan, value) ||
        !contains(findings, value)) {
      return 60;
    }
  }
  return 0;
}

} // namespace

int main() {
  if (const int result = test_pixel_snapshot_access(); result != 0) {
    return result;
  }
  if (const int result = test_invalid_snapshot_and_compatible_defaults();
      result != 0) {
    return result;
  }
  return test_step_523_structure_and_documentation();
}
