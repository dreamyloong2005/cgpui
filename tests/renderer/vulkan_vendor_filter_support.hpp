#pragma once

// Runs a test once per GPU vendor: VK_LOADER_VENDOR_ID_FILTER makes the
// Vulkan loader show one vendor's devices to every later instance of the
// process, the renderer's included.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <vulkan/vulkan.h>

#include <cstdint>
#include <cstdio>
#include <iostream>
#include <set>
#include <vector>

namespace cgpui_test {

// The Vulkan 1.1 devices the loader shows under the current environment.
inline std::vector<VkPhysicalDeviceProperties> visible_vulkan_devices() {
  const VkApplicationInfo application{.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                                      .apiVersion = VK_API_VERSION_1_1};
  const VkInstanceCreateInfo create{.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                    .pApplicationInfo = &application};
  VkInstance instance = VK_NULL_HANDLE;
  if (vkCreateInstance(&create, nullptr, &instance) != VK_SUCCESS) return {};
  std::uint32_t count = 0;
  vkEnumeratePhysicalDevices(instance, &count, nullptr);
  std::vector<VkPhysicalDevice> devices(count);
  vkEnumeratePhysicalDevices(instance, &count, devices.data());
  std::vector<VkPhysicalDeviceProperties> result;
  for (VkPhysicalDevice device : devices) {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(device, &properties);
    if (properties.apiVersion >= VK_API_VERSION_1_1) result.push_back(properties);
  }
  vkDestroyInstance(instance, nullptr);
  return result;
}

// The vendors of the devices the loader shows unfiltered (printed in
// enumeration order).
inline std::set<std::uint32_t> vulkan_vendors() {
  std::set<std::uint32_t> vendors;
  for (const auto& device : visible_vulkan_devices()) {
    std::cout << "unfiltered: " << device.deviceName << '\n';
    vendors.insert(device.vendorID);
  }
  return vendors;
}

// Sets the filter to `vendor`; false unless the loader then shows devices
// of that vendor only.
inline bool filter_vulkan_vendor(std::uint32_t vendor) {
  char filter[16]{};
  std::snprintf(filter, sizeof(filter), "0x%04x", vendor);
  SetEnvironmentVariableA("VK_LOADER_VENDOR_ID_FILTER", filter);
  const auto devices = visible_vulkan_devices();
  for (const auto& device : devices) {
    std::cout << "VK_LOADER_VENDOR_ID_FILTER=" << filter << ": " << device.deviceName << '\n';
    if (device.vendorID != vendor) return false;
  }
  return !devices.empty();
}

} // namespace cgpui_test
