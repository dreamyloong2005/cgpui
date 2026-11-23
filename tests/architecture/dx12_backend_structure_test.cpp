#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
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
  if (types.find("return RendererBackendTarget::dx12") != std::string::npos) {
    return fail(7, "experimental DX12 became default before parity");
  }
  return 0;
}
