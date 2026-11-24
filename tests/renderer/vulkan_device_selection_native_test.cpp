#include "vulkan_device_recovery_scene_test_support.hpp"
#include <cstdlib>

using namespace cgpui;
using namespace cgpui_test;
namespace {
class DeviceSetting {
 public:
  DeviceSetting() { if (const auto* value = std::getenv("CGPUI_VULKAN_DEVICE")) saved_ = value; }
  ~DeviceSetting() { (void)set(saved_ ? saved_->c_str() : ""); }
  static bool set(const char* value) { return _putenv_s("CGPUI_VULKAN_DEVICE", value) == 0; }
 private:
  std::optional<std::string> saved_;
};
std::optional<std::uint32_t> default_index() {
  if (!DeviceSetting::set("")) return std::nullopt;
  VulkanRecoveryHooks hooks;
  RendererPixelTestWindow window(L"CGPUIVulkanDefaultIndex", L"Vulkan default adapter identity");
  if (!hooks.installed() || !window.valid()) return std::nullopt;
  auto renderer = create_vulkan_renderer({.native_surface = window.surface(),
      .framebuffer_size = window.framebuffer_size(), .scale = {1}});
  const auto observed = hooks.observation();
  return renderer && observed.reliable ? observed.initial_enumeration_index : std::nullopt;
}
bool rejects(const char* value, ErrorCode code, unsigned native_instances) {
  if (!DeviceSetting::set(value)) return false;
  VulkanRecoveryHooks hooks;
  RendererPixelTestWindow window(L"CGPUIVulkanBadIndex", L"Vulkan invalid adapter selector");
  if (!hooks.installed() || !window.valid()) return false;
  const auto renderer = create_vulkan_renderer({.native_surface = window.surface(),
      .framebuffer_size = window.framebuffer_size(), .scale = {1}});
  const auto o = hooks.observation();
  return !renderer && renderer.error().code == code && o.reliable && !o.device_attempts &&
      o.instances_created == native_instances && o.instances_destroyed == native_instances &&
      o.surfaces_created == native_instances && o.surfaces_destroyed == native_instances;
}
}
int main() {
  VulkanRecoveryDeadline deadline;
  DeviceSetting restore_setting;
  if (!deadline.valid()) return 1;
  const auto index = default_index();
  if (!index) return recovery_fail(2, "default Vulkan adapter identity unavailable");
  if (!rejects("not-an-index", ErrorCode::invalid_argument, 0) ||
      !rejects("4294967295", ErrorCode::renderer_initialization_failed, 1))
    return recovery_fail(3, "invalid/out-of-range selector allocated a native device or fell back");
  const auto selected = std::to_string(*index);
  if (!DeviceSetting::set(selected.c_str())) return 4;
  VulkanRecoveryHooks hooks;
  RendererPixelTestWindow window(L"CGPUIVulkanSelectedIndex", L"Vulkan selected adapter recovery");
  if (!hooks.installed() || !window.valid()) return 5;
  auto renderer = create_vulkan_renderer({.native_surface = window.surface(),
      .framebuffer_size = window.framebuffer_size(), .scale = {1}});
  if (!renderer || hooks.observation().initial_enumeration_index != index) return 6;
  const auto red = recovery_image();
  const auto baseline = recovery_scene_capture(**renderer, &red);
  if (!baseline || !recovery_scene_pixels(*baseline)) return 7;
  const auto before = (*renderer)->backend_info();
  // Re-parsing either a new index or an invalid string would fail this recovery.
  if (!DeviceSetting::set("changed-after-initialization")) return 8;
  auto frame = recovery_scene_frame(**renderer);
  if (!frame) return 9;
  hooks.arm(VulkanLossSite::submit);
  if (!recovery_retry((*frame)->present()) ||
      !recovery_native_cycle(before, (*renderer)->backend_info(), hooks.observation()))
    return recovery_fail(10, "explicit selection was not identity-bound across environment change");
  frame->reset();
  const auto recovered = recovery_scene_capture(**renderer);
  if (!recovered || !recovery_same_pixels(&*recovered, *baseline)) return 11;
  return 0;
}
