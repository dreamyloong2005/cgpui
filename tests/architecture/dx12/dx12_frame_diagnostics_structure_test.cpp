#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
const std::filesystem::path root{CGPUI_SOURCE_ROOT};
std::string read(const std::filesystem::path& path) {
  std::ifstream file(root / path);
  if (!file) throw std::runtime_error("missing diagnostic owner: " + path.string());
  return {std::istreambuf_iterator<char>(file), {}};
}
bool has(const std::string& source, const char* token) {
  return source.find(token) != std::string::npos;
}
void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
} // namespace

int main() {
  try {
    for (const auto* path : {"src/renderer/dx12/dx12_frame_diagnostics_internal.hpp",
                             "src/renderer/dx12/dx12_frame_diagnostics.cpp",
                             "src/renderer/dx12/dx12_frame_diagnostic_plan.cpp"}) {
      const auto source = read(path);
      const auto limit = std::filesystem::path(path).extension() == ".cpp" ? 280 : 200;
      require(std::count(source.begin(), source.end(), '\n') <= limit,
              "DX12 diagnostic planning and accumulation need focused private leaves");
      require(!has(source, "#include \"vulkan") && !has(source, "#include \"../vulkan"),
              "DX12 diagnostics cannot depend on Vulkan implementation details");
    }
    const auto header = read("src/renderer/dx12/dx12_frame_diagnostics_internal.hpp");
    require(has(header, "class Dx12FrameDiagnostics") &&
                !has(header, "DrawInstanced(") && !has(header, "DrawIndexedInstanced(") &&
                !has(header, "CopyTextureRegion(") && !has(header, "std::visit("),
            "the private diagnostic header declares ownership rather than implementing native/planning work");
    const auto plan = read("src/renderer/dx12/dx12_frame_diagnostic_plan.cpp");
    const auto diagnostics = read("src/renderer/dx12/dx12_frame_diagnostics.cpp");
    require(has(plan, "Dx12FrameDiagnostics::plan(") &&
                !has(diagnostics, "Dx12FrameDiagnostics::plan("),
            "eligible-resource planning has a distinct implementation owner");
    require(has(diagnostics, "Dx12FrameDiagnostics::~Dx12FrameDiagnostics(") &&
                has(diagnostics, "Dx12FrameDiagnostics::publish("),
            "the diagnostic owner controls scoped observation and publication");

    const auto command = read("src/renderer/dx12/dx12_draw_commands_internal.hpp");
    require(has(command, "RendererPrimitiveKind") && has(command, "command_index") &&
                has(command, "resource_offset"),
            "typed backend payloads must retain original command and resource identities");
    const auto state = read("src/renderer/dx12/dx12_state_internal.hpp");
    require(has(state, "Dx12FrameDiagnostics") && has(state, "RendererFrameDiagnosticSnapshot"),
            "private state connects scoped recording to the last published public snapshot");

    for (const auto* path : {"src/renderer/dx12/dx12_indexed_draw.cpp",
                             "src/renderer/dx12/dx12_textured_draw.cpp",
                             "src/renderer/dx12/dx12_texture_resources.cpp",
                             "src/renderer/dx12/dx12_backdrop_blur_recording.cpp"}) {
      const auto source = read(path);
      require(has(source, "dx12_frame_diagnostics_internal.hpp"),
              "actual native draw/upload owners must delegate diagnostic accounting");
      require(!has(source, "RendererFrameDiagnosticSnapshot{") &&
                  !has(source, "build_renderer_frame_diagnostics("),
              "native recording hooks cannot duplicate snapshot assembly");
    }
    for (const auto* path : {"src/renderer/dx12/dx12_frame.cpp",
                             "src/renderer/dx12/dx12_presentation.cpp",
                             "src/renderer/dx12/dx12_renderer.cpp",
                             "src/renderer/dx12/dx12_state_internal.hpp"}) {
      const auto source = read(path);
      require(!has(source, "Dx12FrameDiagnostics::") &&
                  !has(source, "build_renderer_frame_diagnostics("),
              "frame, presentation, renderer and state entries cannot absorb diagnostic implementations");
    }
    require(has(read("src/renderer/dx12/dx12_renderer.cpp"), "last_frame_diagnostic_snapshot()"),
            "DX12 exposes diagnostics through the existing renderer API");
    for (const auto& entry : std::filesystem::recursive_directory_iterator(root / "include/cgpui")) {
      if (!entry.is_regular_file() || entry.path().extension() != ".hpp") continue;
      const auto source = read(entry.path());
      require(!has(source, "dx12_frame_diagnostics_internal.hpp") &&
                  !has(source, "Dx12FrameDiagnostics") && !has(source, "Dx12DrawCommand"),
              "DX12 diagnostic accumulators and command metadata must stay private");
    }
    require(has(read("tests/renderer-dx12/mcpp.toml"), "dx12_frame_diagnostics_test.cpp") &&
                has(read("tests/renderer-dx12/mcpp.toml"), "dx12_diagnostic_ledger_test.cpp") &&
                has(read("tests/architecture/mcpp.toml"), "\"dx12/*_structure_test.cpp\""),
            "diagnostic behavior and module-boundary checks require explicit mcpp registration");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
