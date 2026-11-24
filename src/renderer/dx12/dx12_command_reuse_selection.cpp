#include "dx12_command_reuse_signature_internal.hpp"
#include "dx12_state_internal.hpp"
#include "dx12_texture_renderer_internal.hpp"
#include "cgpui/renderer/planning/clip_scissor.hpp"
#include <bit>

namespace cgpui {
namespace {
struct Budget {
  std::size_t pages = 0, used = 0, draws = 0;
  bool slice(std::size_t bytes) {
    constexpr std::size_t capacity = 256 * 1024;
    if (!bytes || bytes > capacity) return false;
    auto aligned = (used + 15) & ~std::size_t(15);
    if (!pages || aligned + bytes > capacity) { ++pages; aligned = 0; }
    used = aligned + bytes; return pages <= 4;
  }
};
bool same(const D3D12_RECT& a, const D3D12_RECT& b) {
  return a.left == b.left && a.top == b.top && a.right == b.right && a.bottom == b.bottom;
}
bool visible_scissor(Dx12State& state, const std::optional<Rect>& clip, Dx12ReuseScene& scene, Budget& budget) {
  const auto resolved = resolve_clip_stack_scissor(PixelExtent{state.width, state.height}, clip, {});
  if (!resolved.visible || ++budget.draws > 512) return false;
  const auto r = resolved.scissor;
  const D3D12_RECT rect{r.x, r.y, LONG(r.x + r.width), LONG(r.y + r.height)};
  if (budget.draws == 1) scene.scissor = rect;
  return same(scene.scissor, rect);
}
bool ready(Dx12State& state, const Dx12TexturedDraw& image, Dx12ReadyImageBinding& out) {
  const auto& renderer = *state.texture_renderer;
  const auto found = renderer.native_textures.find(image.texture.get()); // No insertion/upload/descriptor mutation.
  if (found == renderer.native_textures.end() || !renderer.image_pipeline || !renderer.root_signature ||
      !renderer.samplers || !found->second.uploaded || !found->second.resource || !found->second.srv.heap()) return false;
  const auto& native = found->second;
  const std::weak_ptr<const Dx12Texture> expected = image.texture;
  if (native.source.owner_before(expected) || expected.owner_before(native.source)) return false;
  auto sampler = renderer.samplers->GetGPUDescriptorHandleForHeapStart();
  if (image.sampling == ImageSamplingMode::linear) sampler.ptr += renderer.sampler_stride;
  out = {native.resource.Get(), renderer.image_pipeline.Get(), renderer.root_signature.Get(),
      native.srv.heap(), renderer.samplers.Get(), native.srv.gpu(), sampler};
  return true;
}
}
Result<std::optional<Dx12ReuseScene>> dx12_select_reuse_scene(Dx12State& state, Color clear,
    std::span<const Dx12DrawCommand> draws, bool capture, bool mutations, std::uint64_t surface) {
  const auto no = [] { return std::optional<Dx12ReuseScene>{}; };
  if (!state.device || !state.hwnd || !state.swapchain || state.submitted || state.failed || state.recovery_pending || FAILED(state.removal_reason) ||
      state.composition_surface || state.transparent_background || capture || mutations || !surface ||
      draws.empty() || draws.size() > 256 || !state.pipeline || !state.root_signature) return no();
  Dx12ReuseScene scene; Budget budget; scene.bindings.resize(draws.size());
  for (std::size_t i = 0; i < draws.size(); ++i) {
    const auto& command = draws[i];
    if (const auto* geometry = std::get_if<RectGeometry>(&command.payload)) {
      if ((command.kind != RendererPrimitiveKind::solid_rect && command.kind != RendererPrimitiveKind::rounded_rect &&
           command.kind != RendererPrimitiveKind::text_selection && command.kind != RendererPrimitiveKind::text_caret) ||
          geometry->draws.empty() || !budget.slice(geometry->vertices.size() * sizeof(RectVertex)) ||
          !budget.slice(geometry->indices.size() * sizeof(std::uint32_t))) return no();
      for (const auto& range : geometry->draws)
        if (!range.index_count || !visible_scissor(state, range.clip_rect, scene, budget)) return no();
    } else if (const auto* image = std::get_if<Dx12TexturedDraw>(&command.payload)) {
      if (image->glyph || command.kind != RendererPrimitiveKind::image || !image->texture ||
          image->texture->format != DXGI_FORMAT_R8G8B8A8_UNORM || !budget.slice(image->vertices.size() * sizeof(Dx12TextureVertex)) ||
          !visible_scissor(state, image->clip_rect, scene, budget) || !ready(state, *image, scene.bindings[i])) return no();
      const auto& b = scene.bindings[i];
      if (scene.heap && scene.heap != b.heap) return no();
      scene.heap = b.heap; scene.samplers = b.samplers; scene.pins.push_back(image->texture);
    } else return no(); // Includes glyph/shadow/backdrop/pending/missing/unknown payloads.
  }
  scene.signature = {state.info.device_generation, surface, state.width, state.height,
      std::bit_cast<std::uint32_t>(state.scale.value), std::bit_cast<std::uint32_t>(clear.r),
      std::bit_cast<std::uint32_t>(clear.g), std::bit_cast<std::uint32_t>(clear.b), std::bit_cast<std::uint32_t>(clear.a),
      reinterpret_cast<std::uintptr_t>(state.pipeline.Get()), reinterpret_cast<std::uintptr_t>(state.root_signature.Get()),
      DXGI_FORMAT_R8G8B8A8_UNORM, 1};
  dx12_append_reuse_signature(scene, draws);
  return std::optional<Dx12ReuseScene>{std::move(scene)};
}
} // namespace cgpui
