#include "dx12_texture_renderer_internal.hpp"
#include "dx12_state_internal.hpp"
#include "dx12_glyph_native_internal.hpp"

namespace cgpui {
void dx12_reset_texture_resources(Dx12State& state) noexcept {
  auto& renderer = *state.texture_renderer;
  renderer.pending_uploads.clear();
  renderer.retained.clear();
  renderer.glyph_native->reset_after_parent_release();
  renderer.native_textures.clear();
  renderer.samplers.Reset();
  renderer.image_pipeline.Reset();
  renderer.text_pipeline.Reset();
  renderer.root_signature.Reset();
  renderer.sampler_stride = 0;
}
} // namespace cgpui
