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
  if (!file) throw std::runtime_error(std::string("missing text owner: ") + relative);
  return {std::istreambuf_iterator<char>(file), {}};
}
bool has(const std::string& source, const char* token) {
  return source.find(token) != std::string::npos;
}
void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
}

int main() {
  try {
    for (const auto* path : {"src/renderer/dx12/dx12_text_frame.cpp",
                             "src/renderer/dx12/dx12_text_draws.cpp",
                             "src/renderer/dx12/dx12_text_atlas.cpp",
                             "src/renderer/dx12/dx12_text_cache_internal.hpp"}) {
      const auto text = read(path);
      require(std::count(text.begin(), text.end(), '\n') <= 100,
              "DX12 text owners must retain their focused module budgets");
      require(!has(text, "vulkan_") && !has(text, "DWriteCreateFactory") &&
                  !has(text, "CreateCommittedResource") && !has(text, "CreateGraphicsPipelineState"),
              "text reuses neutral rasterization and the private texture substrate");
    }
    const auto frame = read("src/renderer/dx12/dx12_text_frame.cpp");
    require(has(frame, "Dx12Frame::draw_text(") && has(frame, "renderer_to_device_pixels(") &&
                has(frame, "dx12_build_text_draws(") && has(frame, "append(std::move(draw))"),
            "text commands must join the ordered frame stream after one DPI conversion");
    const auto draws = read("src/renderer/dx12/dx12_text_draws.cpp");
    require(has(draws, "build_textured_glyph_quads(") && has(draws, "build_text_vertices(") &&
                has(draws, "apply_composed_opacity(") && has(draws, "resolve_effective_clip_rect("),
            "DX12 text must consume the shared geometry, clipping and opacity owners");
    const auto atlas = read("src/renderer/dx12/dx12_text_atlas.cpp");
    require(has(atlas, "processed_uploads") && has(atlas, "page.dirty") &&
                has(atlas, "dx12_create_texture(") && has(atlas, "DXGI_FORMAT_R8_UNORM"),
            "glyph cache publishes dirty R8 page snapshots instead of per-frame rerasterization");
    const auto state = read("src/renderer/dx12/dx12_state_internal.hpp");
    require(has(state, "std::unique_ptr<Dx12TextCache> text_cache"),
            "text cache must survive frames and remain private to its renderer state");
    const auto common_frame = read("src/renderer/dx12/dx12_frame.cpp");
    require(!has(common_frame, "Dx12Frame::draw_text(") && !has(common_frame, "GlyphCache"),
            "common frame entry must not absorb the text implementation");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
