#pragma once
#include "vulkan_device_recovery_hooks_test_support.hpp"
#include "renderer_pixel_test_support.hpp"
#include <optional>
#include <thread>

namespace cgpui_test {
class VulkanRecoveryDeadline {
 public:
  VulkanRecoveryDeadline();
  ~VulkanRecoveryDeadline();
  [[nodiscard]] bool valid() const { return done_ != nullptr; }
 private:
  Win32DesktopTestLease desktop_;
  HANDLE done_ = nullptr;
  std::thread worker_;
};
cgpui::ImageAsset recovery_image(std::array<std::uint8_t, 4> color = {255, 0, 0, 255});
cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> recovery_scene_frame(
    cgpui::Renderer& renderer, const cgpui::ImageAsset* upload = nullptr, bool capture = true);
std::optional<cgpui::RendererFramePixels> recovery_scene_capture(
    cgpui::Renderer& renderer, const cgpui::ImageAsset* upload = nullptr);
bool recovery_scene_pixels(const cgpui::RendererFramePixels& pixels,
    std::array<std::uint8_t, 4> image_color = {255, 0, 0, 255});
bool recovery_same_pixels(const cgpui::RendererFramePixels* actual, const cgpui::RendererFramePixels& expected);
bool recovery_native_cycle(const cgpui::RendererBackendInfo& before,
    const cgpui::RendererBackendInfo& after, const VulkanRecoveryObservation& observation,
    unsigned expected_surfaces = 1);
bool recovery_retry(const cgpui::Result<void>& result);
int recovery_fail(int code, const char* reason);
} // namespace cgpui_test
