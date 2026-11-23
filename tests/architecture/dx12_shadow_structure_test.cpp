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
  if (!file) throw std::runtime_error(std::string("missing shadow owner: ") + relative);
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
    for (const auto* path : {"src/renderer/dx12/dx12_shadow_frame.cpp",
                             "src/renderer/dx12/dx12_shadow_validation.cpp",
                             "src/renderer/dx12/dx12_shadow_pipeline.cpp",
                             "src/renderer/dx12/dx12_shadow_recording.cpp",
                             "src/renderer/dx12/dx12_shadow_internal.hpp",
                             "src/renderer/dx12/dx12_draw_dispatch.cpp",
                             "src/renderer/dx12/dx12_indexed_draw.cpp",
                             "src/renderer/dx12/dx12_indexed_draw_internal.hpp",
                             "src/renderer/dx12/dx12_pipeline_state.cpp",
                             "src/renderer/dx12/dx12_pipeline_state_internal.hpp"}) {
      const auto source = read(path);
      require(std::count(source.begin(), source.end(), '\n') <= 100,
              "DX12 shadow and shared PSO owners must remain focused");
      require(!has(source, "vulkan_") && !has(source, "D3DCompile(") &&
                  !has(source, "erf_approx(") && !has(source, "spread_radii("),
              "native shadow integration cannot duplicate CPU geometry or shaders");
    }
    const auto frame = read("src/renderer/dx12/dx12_shadow_frame.cpp");
    require(has(frame, "Dx12Frame::draw_box_shadow(") &&
                has(frame, "renderer_to_device_pixels(") &&
                has(frame, "build_box_shadow_geometry(") &&
                has(frame, "dx12_finite_shadow_geometry("),
            "shadow frame must scale once and consume the shared geometry owner");
    require(!has(read("src/renderer/dx12/dx12_frame.cpp"), "Dx12Frame::draw_box_shadow("),
            "the common frame must not absorb shadow implementation");
    const auto commands = read("src/renderer/dx12/dx12_draw_commands_internal.hpp");
    require(has(commands, "BoxShadowGeometry"),
            "shadows must participate in the ordered command variant");
    const auto presentation = read("src/renderer/dx12/dx12_presentation.cpp");
    require(has(presentation, "dx12_record_draw(") &&
                !has(presentation, "dx12_create_shadow_pipeline(") &&
                !has(presentation, "CreateGraphicsPipelineState("),
            "presentation must dispatch without owning shadow resources or PSOs");
    const auto shadow = read("src/renderer/dx12/dx12_shadow_pipeline.cpp");
    require(has(shadow, "dx12_shadow_vertex_shader(") &&
                has(shadow, "dx12_shadow_fragment_shader(") &&
                has(shadow, "offsetof(BoxShadowVertex, sigma)"),
            "shadow PSO must consume embedded shared-shader DXIL and neutral vertex layout");
    for (const auto* path : {"src/renderer/dx12/dx12_pipeline.cpp",
                             "src/renderer/dx12/dx12_textured_psos.cpp",
                             "src/renderer/dx12/dx12_shadow_pipeline.cpp"}) {
      const auto source = read(path);
      require(has(source, "dx12_pipeline_defaults(") && !has(source, "D3D12_BLEND_SRC_ALPHA"),
              "all native primitive PSOs must share one fixed-state/blend owner");
    }
    const auto defaults = read("src/renderer/dx12/dx12_pipeline_state.cpp");
    require(has(defaults, "D3D12_BLEND_SRC_ALPHA") && has(defaults, "D3D12_BLEND_INV_SRC_ALPHA"),
            "shared pipeline defaults must retain the existing straight-alpha blend contract");
    for (const auto* path : {"src/renderer/dx12/dx12_geometry.cpp",
                             "src/renderer/dx12/dx12_shadow_recording.cpp"}) {
      const auto source = read(path);
      require(has(source, "dx12_record_indexed_draw(") &&
                  !has(source, "dx12_upload_buffer(") && !has(source, "DrawIndexedInstanced("),
              "rectangles and shadows must share indexed upload/recording ownership");
    }
    const auto indexed = read("src/renderer/dx12/dx12_indexed_draw.cpp");
    require(has(indexed, "dx12_upload_buffer(") && has(indexed, "DrawIndexedInstanced(") &&
                has(indexed, "resolve_clip_stack_scissor("),
            "the focused indexed helper must own uploads, range clipping and native draws");
    require(!has(read("src/renderer/dx12/dx12_textured_draw.cpp"), "dx12_record_indexed_draw("),
            "nonindexed texture recording must stay outside the indexed helper");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
