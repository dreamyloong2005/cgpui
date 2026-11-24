#pragma once
#include "dx12_srv_pool_test_support.hpp"
#include "../../src/renderer/dx12/dx12_glyph_native_internal.hpp"
#include "../../src/renderer/dx12/dx12_text_cache_internal.hpp"
#include "../../src/renderer/dx12/dx12_texture_renderer_internal.hpp"

namespace cgpui_test {
void glyph_add_record(cgpui::Dx12State&, unsigned id, unsigned width = 3,
                      unsigned height = 2, unsigned stride = 5);
cgpui::Dx12TextureHandle glyph_publish(cgpui::Dx12State&);
cgpui::Dx12DrawCommand glyph_draw(cgpui::Dx12TextureHandle, float left = 0,
                                 std::size_t command_index = 0);
bool glyph_submit(cgpui::Dx12State&, std::span<const cgpui::Dx12DrawCommand>);
bool glyph_pixels(const cgpui::Dx12State&, const cgpui::Dx12Texture&, unsigned left = 0);
cgpui::Result<std::vector<std::uint8_t>> glyph_readback(cgpui::Dx12State&, ID3D12Resource&);
std::size_t glyph_copy_bytes(const cgpui::Dx12TextureCopyObserver&, cgpui::Dx12TextureCopyKind);
std::size_t glyph_copy_count(const cgpui::Dx12TextureCopyObserver&, cgpui::Dx12TextureCopyKind);
} // namespace cgpui_test
