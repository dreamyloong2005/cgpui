#include "dx12_texture_renderer_internal.hpp"
#include "dx12_glyph_native_internal.hpp"
namespace cgpui {
Dx12TextureRenderer::Dx12TextureRenderer() : glyph_native(std::make_unique<Dx12GlyphNative>()) {}
Dx12TextureRenderer::~Dx12TextureRenderer() = default;
} // namespace cgpui
