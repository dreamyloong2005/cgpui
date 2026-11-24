#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
namespace {
std::string read(const char* path, unsigned cap) {
  std::ifstream file(std::filesystem::path(CGPUI_SOURCE_ROOT) / path);
  if (!file) throw std::runtime_error(std::string("missing adapter preference owner: ") + path);
  std::string text{std::istreambuf_iterator<char>(file), {}};
  if (std::count(text.begin(), text.end(), '\n') > cap) throw std::runtime_error("adapter owner grew broad");
  return text;
}
bool has(const std::string& text, const char* token) { return text.find(token) != std::string::npos; }
void require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
}
int main() {
  try {
    const auto option_header = read("src/renderer/dx12/dx12_adapter_option_internal.hpp", 25);
    const auto option = read("src/renderer/dx12/dx12_adapter_option.cpp", 40);
    require(!has(option_header, "from_chars") && !has(option_header, "windows.h") &&
        has(option, "std::from_chars") && has(option, "high_performance") && has(option, "minimum_power"),
        "strict diagnostic parsing belongs to one private CPU leaf");
    const auto native = read("src/renderer/dx12/dx12_adapter_preference.cpp", 35);
    read("src/renderer/dx12/dx12_adapter_preference_internal.hpp", 20);
    require(has(native, "IDXGIFactory6") && has(native, "QueryInterface") &&
        has(native, "EnumAdapterByGpuPreference") && has(native, "DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE") &&
        has(native, "DXGI_GPU_PREFERENCE_MINIMUM_POWER") && !has(native, "EnumAdapters1") &&
        has(native, "if (FAILED(queried)) return queried"), "native preferences must not degrade to legacy ordering");
    const auto selection = read("src/renderer/dx12/dx12_adapter_selection.cpp", 90);
    const auto fallback = read("src/renderer/dx12/dx12_initialization_failure.cpp", 65);
    require(has(selection, "dx12_parse_adapter_option") && has(fallback, "dx12_parse_adapter_option") &&
        !has(selection, "from_chars") && !has(fallback, "from_chars") &&
        has(fallback, "adapter->kind == Dx12AdapterKind::automatic"), "all explicit preferences forbid backend fallback");
    require(has(selection, "dx12_enumerate_preferred_adapter") && has(selection, "EnumAdapters1") &&
        has(selection, "if (!indexed && (description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)) continue") &&
        has(selection, "dx12_classify_initialization_failure(result) != Dx12InitializationFailureClass::backend_unavailable"),
        "ranked/default scans preserve hardware-only selection and terminal capability errors");
    const auto pinned = read("src/renderer/dx12/dx12_adapter_device.cpp", 90);
    require(has(pinned, "EnumAdapterByLuid") && !has(pinned, "getenv") &&
        !has(pinned, "dx12_enumerate_preferred_adapter"), "recovery must never re-read preference ordering");
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
