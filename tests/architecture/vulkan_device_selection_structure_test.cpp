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
  if (!file) throw std::runtime_error("missing Vulkan device selector owner: " + path);
  return {std::istreambuf_iterator<char>(file), {}};
}
bool has(const std::string& value, const char* text) { return value.find(text) != std::string::npos; }
void require(bool value, const char* reason) { if (!value) throw std::runtime_error(reason); }
void cap(const std::string& path, unsigned limit) {
  const auto source = read(path);
  require(std::count(source.begin(), source.end(), '\n') <= limit, "Vulkan selector exceeded its focused owner cap");
}
}
int main() {
  try {
    const auto policy = read("src/renderer/vulkan/vulkan_device_selection_policy.cpp");
    require(has(policy, "value.empty()") && has(policy, "std::uint32_t index") && has(policy, "std::from_chars(") &&
        has(policy, "parsed.ptr != value.data() + value.size()") && has(policy, "ErrorCode::invalid_argument"),
        "selector accepts only empty/default or a fully parsed unsigned decimal index");
    require(has(policy, "if (!requested_index) return {}") && has(policy, "std::ranges::find(devices, preferred)") &&
        has(policy, "selected = preferred") && has(policy, "*requested_index >= devices.size()") &&
        has(policy, "devices.resize(1)") && !has(policy, "getenv"),
        "automatic candidate order stays intact; explicit recovery locks identity and cannot fall back");
    const auto initialize = read("src/renderer/vulkan/vulkan_device_selection.cpp");
    const auto state = read("src/renderer/vulkan/vulkan_state.cpp");
    require(has(initialize, "std::getenv(\"CGPUI_VULKAN_DEVICE\")") &&
        has(initialize, "requested_device_index_ = *selection") &&
        state.find("initialize_device_selection()") < state.find("create_instance()"),
        "snapshot/validate the private selector once before initial native creation");
    for (const auto* path : {"vulkan_physical_device.cpp", "vulkan_composition_device.cpp"}) {
      const auto source = read(std::string("src/renderer/vulkan/") + path);
      const auto filter = source.find("vulkan_filter_device_candidates(devices, requested_device_index_, preferred)");
      require(filter != std::string::npos && filter < source.find("for (VkPhysicalDevice device : devices)") &&
          !has(source, "getenv"), "ordinary/interop selection must prefilter without bypassing suitability or re-reading env");
    }
    const auto recovery = read("src/renderer/vulkan/vulkan_device_recovery.cpp");
    require(has(recovery, "select_physical_device(preferred)") && !has(recovery, "initialize_device_selection") &&
        !has(recovery, "getenv"), "device recovery must not reparse the initial selector");
    const auto registration = read("tests/renderer/mcpp.toml");
    for (const auto* name : {"policy", "native", "unsuitable"}) {
      const auto file = std::string("vulkan_device_selection_") + name + "_test.cpp";
      require(registration.find(file) != std::string::npos, "device selection regression is not registered");
      cap("tests/renderer/" + file, 100);
    }
    cap("src/renderer/vulkan/vulkan_device_selection_internal.hpp", 20);
    cap("src/renderer/vulkan/vulkan_device_selection_policy.cpp", 60);
    cap("src/renderer/vulkan/vulkan_device_selection.cpp", 25);
    cap("tests/architecture/vulkan_device_selection_structure_test.cpp", 80);
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
