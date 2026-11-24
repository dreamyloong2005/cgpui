#include "vulkan_device_recovery_hook_state_test_support.hpp"

namespace cgpui_test {
VulkanRecoveryHookState& recovery_hook_state() {
  static VulkanRecoveryHookState state;
  return state;
}
VulkanRecoveryHooks::VulkanRecoveryHooks() {
  auto& state = recovery_hook_state();
  if (state.active || !cgpui::ensure_vulkan_windows_loader()) return;
  state = {};
#define CGPUI_VULKAN_ENTRY(name) state.real.name = cgpui::name;
#include "vulkan_loader_windows_symbols.inc"
#undef CGPUI_VULKAN_ENTRY
  state.active = installed_ = true;
  install_recovery_lifetime_hooks();
  install_recovery_operation_hooks();
}
VulkanRecoveryHooks::~VulkanRecoveryHooks() {
  if (!installed_) return;
  auto& state = recovery_hook_state();
  state.armed = false;
  // On an early test exit, never restore imports while a healthy simulated
  // device still has unfinished work. Fixture ownership normally leaves none.
  for (auto device : state.devices) if (device) (void)state.real.vkDeviceWaitIdle(device);
#define CGPUI_VULKAN_ENTRY(name) cgpui::name = state.real.name;
#include "vulkan_loader_windows_symbols.inc"
#undef CGPUI_VULKAN_ENTRY
  state.active = false;
}
void VulkanRecoveryHooks::arm(VulkanLossSite site, VkResult result, VulkanRebuildFailure failure) {
  auto& state = recovery_hook_state();
  state.site = site; state.injected_result = result; state.rebuild_failure = failure;
  state.fence_calls = 0; state.armed = installed_;
  state.observed.injected = state.observed.safely_quiesced = false;
}
VulkanRecoveryObservation VulkanRecoveryHooks::observation() const {
  auto value = recovery_hook_state().observed;
  value.reliable = value.reliable && installed_;
  return value;
}
bool simulate_vulkan_loss(VulkanLossSite site, VkResult& result) {
  auto& state = recovery_hook_state();
  if (!state.armed || state.site != site) return false;
  state.armed = false;
  bool found = false;
  for (auto device : state.devices) if (device) {
    found = true;
    const auto idle = state.real.vkDeviceWaitIdle(device);
    if (idle != VK_SUCCESS) {
      state.observed.reliable = false;
      result = idle; // Do not overwrite a real failure with a claimed simulation.
      return true;
    }
  }
  if (!found) { state.observed.reliable = false; return false; }
  state.observed.injected = state.observed.safely_quiesced = true;
  result = state.injected_result;
  return true;
}
} // namespace cgpui_test
