#include "vulkan_device_recovery_hook_state_test_support.hpp"
#include "vulkan_device_recovery_scene_test_support.hpp"
#include <cstdlib>

using namespace cgpui;
using namespace cgpui_test;
namespace {
PFN_vkGetPhysicalDeviceProperties native_properties = nullptr;
unsigned refused_queries = 0;
VKAPI_ATTR void VKAPI_CALL unsuitable(VkPhysicalDevice device, VkPhysicalDeviceProperties* properties) {
  native_properties(device, properties);
  const auto& state = recovery_hook_state();
  if (state.enumeration_count && device == state.enumerated[0]) {
    properties->apiVersion = VK_API_VERSION_1_0; // Simulated capability refusal, never native resource mutation.
    ++refused_queries;
  }
}
class SelectionScope {
 public:
  SelectionScope() {
    if (const auto* value = std::getenv("CGPUI_VULKAN_DEVICE")) saved_ = value;
    valid = _putenv_s("CGPUI_VULKAN_DEVICE", "0") == 0;
    native_properties = cgpui::vkGetPhysicalDeviceProperties;
    cgpui::vkGetPhysicalDeviceProperties = unsuitable;
  }
  ~SelectionScope() {
    cgpui::vkGetPhysicalDeviceProperties = native_properties;
    (void)_putenv_s("CGPUI_VULKAN_DEVICE", saved_ ? saved_->c_str() : "");
  }
  bool valid = false;
 private:
  std::optional<std::string> saved_;
};
}
int main() {
  VulkanRecoveryDeadline deadline;
  VulkanRecoveryHooks hooks;
  if (!deadline.valid() || !hooks.installed()) return 1;
  SelectionScope selected;
  RendererPixelTestWindow window(L"CGPUIVulkanUnsuitableIndex", L"Vulkan explicitly unsuitable adapter");
  if (!selected.valid || !window.valid()) return 2;
  auto surface = window.surface();
  for (const bool composition : {false, true}) {
    surface.composition = composition;
    const auto before = refused_queries;
    const auto renderer = create_vulkan_renderer({.native_surface = surface,
        .framebuffer_size = window.framebuffer_size(), .scale = {1}});
    const auto o = hooks.observation();
    if (renderer || renderer.error().code != ErrorCode::renderer_initialization_failed ||
        !o.reliable || o.device_attempts || refused_queries <= before || o.live_devices ||
        o.instances_created != o.instances_destroyed || o.surfaces_created != o.surfaces_destroyed)
      return recovery_fail(3, "explicit unsuitable ordinary/interop adapter was accepted or replaced");
  }
  return 0;
}
