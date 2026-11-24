#include "vulkan_device_recovery_scene_test_support.hpp"
#include "renderer_text_pixel_shape_support.hpp"
#include "../support/frame_wait_test_support.hpp"
#include <iostream>

namespace cgpui_test {
VulkanRecoveryDeadline::VulkanRecoveryDeadline() : done_(CreateEventW(nullptr, TRUE, FALSE, nullptr)) {
  if (done_) worker_ = std::thread([this] {
    if (WaitForSingleObject(done_, 120000) == WAIT_TIMEOUT) {
      const char message[] = "Vulkan simulated recovery test timed out\n";
      DWORD written = 0;
      WriteFile(GetStdHandle(STD_ERROR_HANDLE), message, sizeof(message) - 1, &written, nullptr);
      TerminateProcess(GetCurrentProcess(), 90); // Only this standalone test process.
    }
  });
}
VulkanRecoveryDeadline::~VulkanRecoveryDeadline() {
  if (done_) SetEvent(done_);
  if (worker_.joinable()) worker_.join();
  if (done_) CloseHandle(done_);
}
cgpui::ImageAsset recovery_image(std::array<std::uint8_t, 4> color) {
  return {.id = {2026112501}, .logical_size = {1, 1},
      .bitmap = {.width = 1, .height = 1, .stride = 4,
          .pixels = {color[0], color[1], color[2], color[3]}}};
}
cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> recovery_scene_frame(
    cgpui::Renderer& renderer, const cgpui::ImageAsset* upload, bool capture) {
  auto frame = wait_for_test_frame(renderer);
  if (!frame) return std::unexpected(frame.error());
  (*frame)->clear({0, 0, 0, 1});
  if (upload) (*frame)->upload_image(*upload);
  const auto image = recovery_image();
  (*frame)->draw_image({.bounds = {{36, 36}, {16, 16}}, .asset = cgpui::describe_image_asset(image),
      .sampling = cgpui::ImageSamplingMode::nearest});
  const cgpui::FontDescriptor font{.family = "Segoe UI"};
  (*frame)->draw_text({.bounds = {{4, 4}, {40, 28}}, .color = {1, 1, 1, 1}, .font = font,
      .content = "A", .byte_length = 1, .font_size = 20, .device_font_size = 20,
      .glyphs = cgpui::text_glyph_paint_metadata(cgpui::shape_text("A", font, 20), {4, 4})});
  if (capture) if (auto result = (*frame)->request_pixel_capture(); !result)
    return std::unexpected(result.error());
  return frame;
}
std::optional<cgpui::RendererFramePixels> recovery_scene_capture(
    cgpui::Renderer& renderer, const cgpui::ImageAsset* upload) {
  auto frame = recovery_scene_frame(renderer, upload);
  if (!frame || !pixel_test_ok((*frame)->present(), "recovery scene present")) return std::nullopt;
  const auto* pixels = renderer.last_frame_pixels();
  return pixels && pixels->valid() ? std::optional{*pixels} : std::nullopt;
}
bool recovery_scene_pixels(const cgpui::RendererFramePixels& pixels, std::array<std::uint8_t, 4> color) {
  const auto ink = text_ink_shape(pixels);
  return pixels.valid() && pixels.width == 64 && pixels.height == 64 && ink.shaped() && ink.bottom < 32 &&
      pixel_near(pixels, 42, 42, color) && pixel_near(pixels, 60, 60, {0, 0, 0, 255});
}
bool recovery_same_pixels(const cgpui::RendererFramePixels* actual, const cgpui::RendererFramePixels& expected) {
  return actual && actual->valid() && actual->width == expected.width && actual->height == expected.height &&
      actual->encoding == expected.encoding && actual->rgba8 == expected.rgba8;
}
bool recovery_native_cycle(const cgpui::RendererBackendInfo& before,
    const cgpui::RendererBackendInfo& after, const VulkanRecoveryObservation& o, unsigned surfaces) {
  const bool valid = o.reliable && o.injected && o.safely_quiesced && o.device_attempts == 2 &&
      o.devices_created == 2 && o.devices_destroyed == 1 && o.live_devices == 1 &&
      o.instances_created == 1 && !o.instances_destroyed && o.surfaces_created == surfaces &&
      !o.surfaces_destroyed && o.initial_physical && o.latest_physical == o.initial_physical &&
      before.backend == cgpui::RendererBackend::vulkan && after.backend == before.backend &&
      after.adapter_luid == before.adapter_luid && after.adapter_name == before.adapter_name &&
      after.device_generation == before.device_generation + 1;
  if (valid) std::cout << "VULKAN_RECOVERY simulated_loss=1 real_idle=success adapter=" << after.adapter_name
      << " generation=" << after.device_generation << " created=" << o.devices_created
      << " destroyed=" << o.devices_destroyed << " preserved_instance_surface=1\n";
  return valid;
}
bool recovery_retry(const cgpui::Result<void>& result) {
  return !result && result.error().code == cgpui::ErrorCode::frame_retry_required;
}
int recovery_fail(int code, const char* reason) {
  std::cerr << "Vulkan simulated recovery: " << reason << '\n';
  return code;
}
} // namespace cgpui_test
