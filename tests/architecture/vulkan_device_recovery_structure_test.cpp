#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
std::string read(const std::string& path) {
  std::ifstream file(std::filesystem::path(CGPUI_SOURCE_ROOT) / path);
  if (!file) throw std::runtime_error("missing Vulkan recovery owner: " + path);
  return {std::istreambuf_iterator<char>(file), {}};
}
bool has(const std::string& value, const char* token) { return value.find(token) != std::string::npos; }
void require(bool condition, const char* reason) { if (!condition) throw std::runtime_error(reason); }
void cap(const std::string& path, unsigned lines) {
  const auto value = read(path);
  require(std::count(value.begin(), value.end(), '\n') <= lines, "Vulkan recovery owner exceeds its focused cap");
}
}
int main() {
  try {
    const auto recovery = read("src/renderer/vulkan/vulkan_device_recovery.cpp");
    const auto lifetime = read("src/renderer/vulkan/vulkan_device_resources.cpp");
    const auto state = read("src/renderer/vulkan/vulkan_state.cpp");
    require(has(recovery, "failure != VK_ERROR_DEVICE_LOST") &&
        recovery.find("failure != VK_ERROR_DEVICE_LOST") < recovery.find("destroy_device_resources(false)") &&
        has(recovery, "device_recovery_terminal_") && has(recovery, "device_recovery_error_") &&
        has(recovery, "frame_retry_required") && has(recovery, "++device_generation_"),
        "confirmed typed loss alone authorizes rebuild; unsuccessful rebuild stays terminal");
    require(!has(recovery, "vkDestroyInstance") && !has(recovery, "vkDestroySurfaceKHR") &&
        !has(lifetime, "vkDestroyInstance") && !has(lifetime, "vkDestroySurfaceKHR") &&
        has(state, "destroy_device_resources(true)") && has(state, "create_device_resources()"),
        "device lifecycle is a focused shared owner, preserving instance and surface through recovery");
    require(has(lifetime, "glyph_atlas_plan_state_ = {}") &&
        has(lifetime, "pending_glyph_atlas_plan_state_ = {}") &&
        !has(lifetime, "glyph_cache_ =") && !has(lifetime, "retained_image_uploads_.clear"),
        "native retirement resets GPU prefixes without deleting resident CPU recovery content");
    require(has(read("src/renderer/vulkan/vulkan_physical_device.cpp"), "device == preferred") &&
        has(read("src/renderer/vulkan/vulkan_physical_device.cpp"), "vulkan_prefer_device_candidate(") &&
        has(read("src/renderer/vulkan/vulkan_composition_device.cpp"), "vulkan_composition_device_suitable(device)"),
        "prior adapter preference must retain both ordinary and interop suitability policies");
    for (const auto* file : {"vulkan_swapchain_create.cpp", "vulkan_composition_resources.cpp", "vulkan_swapchain_install.cpp"}) {
      const auto source = read(std::string("src/renderer/vulkan/") + file);
      const auto caught = source.find("catch (...)");
      require(caught != std::string::npos && source.find("destroy_swapchain_resources(resources)", caught) != std::string::npos &&
          source.find("throw;", caught) != std::string::npos, "exceptional candidates must release their native children before rethrow");
    }
    const auto install = read("src/renderer/vulkan/vulkan_swapchain_lifecycle.cpp");
    const auto prepare = install.find("auto command_reuse = prepare_swapchain_install(resources)");
    require(prepare != std::string::npos && prepare < install.find("swapchain_ = resources.swapchain") &&
        has(install, "command_reuse_states_ = std::move(command_reuse)") && has(install, "resources.swapchain = VK_NULL_HANDLE"),
        "allocate reuse state before native transfer, then clear the candidate ownership without further allocation");
    const auto hook = read("tests/renderer/vulkan_device_recovery_hooks_test_support.cpp");
    const auto operations = read("tests/renderer/vulkan_device_recovery_operations_test_support.cpp");
    require(has(hook, "state.real.vkDeviceWaitIdle(device)") && has(hook, "idle != VK_SUCCESS") &&
        hook.find("idle != VK_SUCCESS") < hook.find("state.observed.injected = state.observed.safely_quiesced = true") &&
        !has(hook, "VulkanRendererState") && !has(operations, "VulkanRendererState"),
        "simulated loss must drain the healthy native device, never edit renderer state or invent idle success");
    require(has(operations, "VulkanLossSite::capture_wait") && has(operations, "VulkanLossSite::resize_wait") &&
        has(operations, "VulkanLossSite::submit") && has(operations, "VulkanLossSite::present") &&
        has(operations, "VulkanLossSite::acquire"), "all native result sites retain explicit fault coverage");
    const auto registration = read("tests/renderer/mcpp.toml");
    for (const auto* name : {"sites", "content", "failure", "nonloss", "popup", "retention", "accounting", "restore_policy",
                           "limits", "alpha", "acquire_sync", "eviction_failure"}) {
      const auto file = std::string("vulkan_device_recovery_") + name + "_test.cpp";
      require(registration.find(file) != std::string::npos, "native recovery regression is not registered");
      cap("tests/renderer/" + file, 100);
    }
    for (const auto* name : {"hooks", "lifetime", "operations", "scene"}) {
      const auto file = std::string("vulkan_device_recovery_") + name + "_test_support.cpp";
      require(registration.find(file) != std::string::npos, "native recovery support is not registered");
      cap("tests/renderer/" + file, 120);
    }
    cap("tests/renderer/vulkan_device_recovery_hooks_test_support.hpp", 45);
    cap("tests/renderer/vulkan_device_recovery_hook_state_test_support.hpp", 35);
    cap("tests/renderer/vulkan_device_recovery_scene_test_support.hpp", 45);
    cap("src/renderer/vulkan/vulkan_device_recovery.cpp", 80);
    cap("src/renderer/vulkan/vulkan_device_resources.cpp", 80);
    cap("src/renderer/vulkan/vulkan_state.cpp", 60);
    cap("src/renderer/vulkan/vulkan_swapchain_create.cpp", 100);
    cap("src/renderer/vulkan/vulkan_composition_resources.cpp", 70);
    cap("src/renderer/vulkan/vulkan_swapchain_install.cpp", 40);
    cap("src/renderer/vulkan/vulkan_swapchain_lifecycle.cpp", 120);
    cap("tests/architecture/vulkan_device_recovery_structure_test.cpp", 100);
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
