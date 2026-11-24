#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>

namespace {
const std::filesystem::path root{CGPUI_SOURCE_ROOT};
std::string read(const std::filesystem::path& path) {
  std::ifstream source(root / path);
  return {std::istreambuf_iterator<char>(source), {}};
}
int fail(int code, const std::string& message) {
  std::cerr << "DX12 structure: " << message << '\n';
  return code;
}
}

int main() {
  const auto native = root / "src/renderer/dx12";
  if (!std::filesystem::is_directory(native)) return fail(1, "native module absent");
  unsigned implementations = 0;
  for (const auto& entry : std::filesystem::directory_iterator(native)) {
    const auto extension = entry.path().extension();
    if (extension != ".cpp" && extension != ".hpp") continue;
    const auto text = read(entry.path());
    const auto lines = std::count(text.begin(), text.end(), '\n');
    if (lines > (extension == ".cpp" ? 280 : 200)) {
      return fail(2, entry.path().filename().string() + " exceeds focused module budget");
    }
    if (text.find("#include \"vulkan") != std::string::npos ||
        text.find("#include \"../vulkan") != std::string::npos ||
        text.find("D3DCompile(") != std::string::npos) {
      return fail(3, "DX12 depends on Vulkan internals or runtime shader compilation");
    }
    implementations += extension == ".cpp";
  }
  if (implementations < 4) return fail(4, "device/surface/pipeline/frame need focused owners");
  for (const auto* name : {"renderer_backend.hpp", "renderer_backend_info.hpp"}) {
    const auto text = read(std::filesystem::path("include/cgpui/renderer") / name);
    if (text.empty() || text.find("windows.h") != std::string::npos ||
        text.find("d3d12.h") != std::string::npos ||
        text.find("../../../src/") != std::string::npos) return fail(5, "public boundary leaks native code");
  }
  const auto manifest = read("packages/renderer-dx12/mcpp.toml");
  if (manifest.find("renderer-vulkan") != std::string::npos ||
      manifest.find("renderer-windows") != std::string::npos ||
      manifest.find("cgpui.ui") != std::string::npos) return fail(6, "backend dependency points upward");
  const auto types = read("include/cgpui/renderer/renderer_types.hpp");
  if (types.find("return RendererBackendTarget::dx12") == std::string::npos ||
      read("src/renderer/windows/windows_renderer_factory.cpp").find("dx12_attempt_renderer_initialization") == std::string::npos) {
    return fail(7, "public Windows default and classified automatic factory must agree");
  }
  const auto state = read(native / "dx12_state_internal.hpp");
  const auto aggregate = read(native / "dx12_state_operations_internal.hpp");
  if (std::count(state.begin(), state.end(), '\n') > 100 ||
      state.find("#include \"dx12_state_operations_internal.hpp\"") == std::string::npos ||
      std::count(aggregate.begin(), aggregate.end(), '\n') > 15) {
    return fail(8, "state storage and compatibility operation aggregate must remain thin");
  }
  std::istringstream lines(aggregate);
  for (std::string line; std::getline(lines, line);) {
    const auto first = line.find_first_not_of(" \t\r");
    if (first == std::string::npos) continue;
    const auto content = line.substr(first);
    if (!content.starts_with("#pragma once") && !content.starts_with("#include ") &&
        !content.starts_with("//")) return fail(9, "operation aggregate must contain only includes, not declarations or bodies");
  }
  struct OperationOwner { const char* header; const char* declaration; unsigned cap; };
  for (const auto& owner : {
      OperationOwner{"dx12_device_internal.hpp", "dx12_create_device(", 30},
      OperationOwner{"dx12_debug_internal.hpp", "dx12_check_debug_errors(", 20},
      OperationOwner{"dx12_pipeline_state_internal.hpp", "dx12_create_pipeline(", 20},
      OperationOwner{"dx12_synchronization_internal.hpp", "dx12_wait_idle(", 60},
      OperationOwner{"dx12_swapchain_internal.hpp", "dx12_create_swapchain(", 20},
      OperationOwner{"dx12_swapchain_internal.hpp", "dx12_create_targets(", 20},
      OperationOwner{"dx12_swapchain_internal.hpp", "dx12_resize(", 20},
      OperationOwner{"dx12_capture_internal.hpp", "dx12_prepare_capture(", 20},
      OperationOwner{"dx12_capture_internal.hpp", "dx12_record_capture(", 20},
      OperationOwner{"dx12_capture_internal.hpp", "dx12_read_capture(", 20},
      OperationOwner{"dx12_resource_transition_internal.hpp", "dx12_transition(", 20}}) {
    const auto leaf = read(native / owner.header);
    const auto declared = leaf.find(owner.declaration);
    if (leaf.empty() || declared == std::string::npos ||
        leaf.find(owner.declaration, declared + 1) != std::string::npos ||
        std::count(leaf.begin(), leaf.end(), '\n') > owner.cap ||
        leaf.find("#include \"dx12_state_internal.hpp\"") != std::string::npos ||
        aggregate.find(std::string("#include \"") + owner.header + "\"") == std::string::npos ||
        state.find(owner.declaration) != std::string::npos) {
      return fail(10, std::string("native operation must have a focused acyclic declaration owner: ") + owner.declaration);
    }
  }
  const auto transition = read(native / "dx12_resource_transition.cpp");
  if (transition.find("#include \"dx12_resource_transition_internal.hpp\"") == std::string::npos ||
      transition.find("commands.ResourceBarrier(1, &barrier)") == std::string::npos ||
      std::count(transition.begin(), transition.end(), '\n') > 20 ||
      read(native / "dx12_synchronization.cpp").find("void dx12_transition(") != std::string::npos) {
    return fail(11, "resource transitions must not be implemented in the fence-wait owner");
  }
  return 0;
}
