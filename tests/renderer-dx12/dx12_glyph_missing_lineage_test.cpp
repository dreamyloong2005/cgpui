#include "dx12_glyph_fallback_controls_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;
namespace {
int run(bool absent_inputs) {
  std::array<Dx12TextureCopyEvent, 32> events{};
  Dx12TextureCopyObserver observer(events);
  Dx12Window window;
  auto state = srv_native_state(&window);
  if (!state) return dx12_fail(1, "missing-lineage native fixture");
  auto& owner = *state->texture_renderer->glyph_native;
  owner.observer = &observer;
  GlyphFallbackHistory history;
  const auto a = history.append(0, 1, 1, 1);
  if (!glyph_fallback_submit(*state, a, history.lineage, history.inputs()))
    return dx12_fail(2, "missing-lineage prior completed base");
  auto* base = state->texture_renderer->native_textures.at(a.get()).resource.Get();
  if (!glyph_fallback_full_copy(observer, *base, *a) || !glyph_fallback_readback(*state, *base, *a))
    return dx12_fail(3, "missing-lineage base full copy/readback/debug gate");
  const auto sealed_target = history.append(0, 2, 8, 1);
  const auto seal = history.lineage.find_snapshot(sealed_target);
  if (!seal) return dx12_fail(4, "missing-lineage fixture successor seal");
  const auto plan = dx12_plan_glyph_regions(history.lineage, *seal, 1, history.records, history.pages);
  if (!plan || plan->dirty_bytes != 6 || plan->full_bytes != 256 ||
      owner.completed_base(*seal).resource.Get() != base)
    return dx12_fail(4, "otherwise eligible completed base must exist before omitting lineage");
  Dx12TextureHandle target = absent_inputs ? sealed_target
      : std::make_shared<const Dx12Texture>(*sealed_target);
  if (!absent_inputs && history.lineage.find_snapshot(target))
    return dx12_fail(5, "distinct immutable copy must have no optional snapshot seal");
  const auto inputs = absent_inputs ? Dx12GlyphTargetInputs{} : history.inputs();
  const auto prior_ticket = state->fence_value;
  observer.clear();
  if (!glyph_fallback_submit(*state, target, history.lineage, inputs))
    return dx12_fail(6, "valid missing-lineage input must full-initialize");
  const auto& native = state->texture_renderer->native_textures.at(target.get());
  const auto stats = owner.stats();
  if (!native.uploaded || native.resource.Get() == base || !native.srv.size() ||
      state->submitted || state->fence_value <= prior_ticket || stats.executed_unknown ||
      stats.candidates || stats.completed_families != 1 ||
      owner.completed_base(*seal).resource.Get() != base || owner.completed_base(*seal).prefix != 1 ||
      !glyph_fallback_full_copy(observer, *native.resource.Get(), *target) ||
      !glyph_fallback_readback(*state, *native.resource.Get(), *target) ||
      !glyph_fallback_readback(*state, *base, *a))
    return dx12_fail(7, "missing lineage must be real fullP/zero-clone without catalog promotion");
  std::cout << "private synthetic missing-lineage " << (absent_inputs ? "null-input" : "unsealed-snapshot")
            << " actual fullP=256 clone=0; completed target/base whole-page readback and debug gates\n";
  return 0;
}
}
int main() {
  try {
    if (!glyph_fallback_enable_debug()) return dx12_fail(1, "missing-lineage debug configuration");
    if (const auto result = run(true)) return result;
    return run(false);
  }
  catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
