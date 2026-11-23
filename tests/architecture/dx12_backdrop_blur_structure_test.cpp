#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
std::string read(const char* relative) {
  std::ifstream file(std::filesystem::path(CGPUI_SOURCE_ROOT) / relative);
  if (!file) throw std::runtime_error(std::string("missing backdrop owner: ") + relative);
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
    for (const auto* path : {"src/renderer/dx12/dx12_backdrop_blur_frame.cpp",
                             "src/renderer/dx12/dx12_backdrop_blur_validation.cpp",
                             "src/renderer/dx12/dx12_backdrop_blur_support.cpp",
                             "src/renderer/dx12/dx12_backdrop_blur_root.cpp",
                             "src/renderer/dx12/dx12_backdrop_blur_pipelines.cpp",
                             "src/renderer/dx12/dx12_backdrop_blur_targets.cpp",
                             "src/renderer/dx12/dx12_backdrop_blur_recording.cpp",
                             "src/renderer/dx12/dx12_backdrop_blur_passes.cpp",
                             "src/renderer/dx12/dx12_backdrop_blur_composite.cpp"}) {
      const auto source = read(path);
      require(std::count(source.begin(), source.end(), '\n') <= 100,
              "DX12 backdrop implementations must remain focused within existing budgets");
      require(!has(source, "vulkan_") && !has(source, "D3DCompile(") &&
                  !has(source, "std::exp(") && !has(source, "std::sqrt("),
              "backdrop integration cannot duplicate neutral planning or shader algorithms");
    }
    const auto frame = read("src/renderer/dx12/dx12_backdrop_blur_frame.cpp");
    require(has(frame, "Dx12Frame::draw_backdrop_blur(") &&
                has(frame, "renderer_to_device_pixels(") && has(frame, "plan_backdrop_blur("),
            "backdrop frame must convert DPI once and consume neutral planning");
    require(!has(read("src/renderer/dx12/dx12_frame.cpp"), "Dx12Frame::draw_backdrop_blur("),
            "common frame cannot absorb backdrop implementation");
    require(has(read("src/renderer/dx12/dx12_draw_commands_internal.hpp"), "BackdropBlurPlan") &&
                has(read("src/renderer/dx12/dx12_draw_dispatch.cpp"), "dx12_record_backdrop_blur("),
            "backdrops must occupy their original place in the typed draw stream");
    const auto presentation = read("src/renderer/dx12/dx12_presentation.cpp");
    require(presentation.find("dx12_wait_idle(") < presentation.find("dx12_begin_backdrop_blur_frame(") &&
                !has(presentation, "CopyResource(") && !has(presentation, "CreateCommittedResource("),
            "presentation only resets frame effects after idle, without owning effect recording");
    const auto recording = read("src/renderer/dx12/dx12_backdrop_blur_recording.cpp");
    require(has(recording, "CopyResource(") && has(recording, "D3D12_RESOURCE_STATE_COPY_SOURCE") &&
                has(recording, "dx12_record_backdrop_blur_passes(") &&
                has(recording, "dx12_record_backdrop_blur_composite("),
            "each backdrop snapshots preceding draws then filters and composites in order");
    const auto support = read("src/renderer/dx12/dx12_backdrop_blur_support.cpp");
    require(has(support, "CheckFeatureSupport(") && has(support, "D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE") &&
                has(support, "D3D12_FORMAT_SUPPORT1_RENDER_TARGET"),
            "capability reporting must probe the actual offscreen format");
    const auto pipeline = read("src/renderer/dx12/dx12_backdrop_blur_pipelines.cpp");
    require(has(pipeline, "dx12_pipeline_defaults(") &&
                has(pipeline, "dx12_backdrop_composite_fragment_shader(") &&
                has(pipeline, "dx12_backdrop_blur_fragment_shader("),
            "native effect PSOs must reuse fixed state and embedded shared-shader DXIL");
    const auto state = read("src/renderer/dx12/dx12_state_internal.hpp");
    require(has(state, "std::unique_ptr<Dx12BackdropBlurRenderer> backdrop_blur"),
            "effect pipelines and retained frame targets need a private owning module");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
