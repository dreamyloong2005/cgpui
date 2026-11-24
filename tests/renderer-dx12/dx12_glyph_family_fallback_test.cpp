#include "dx12_glyph_fallback_controls_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;
int main() {
  try {
    if (!glyph_fallback_enable_debug()) return dx12_fail(1, "family fallback debug configuration");
    std::array<Dx12TextureCopyEvent, 32> events{};
    Dx12TextureCopyObserver observer(events);
    Dx12Window window;
    auto state = srv_native_state(&window);
    if (!state) return dx12_fail(1, "family fallback native fixture");
    auto& owner = *state->texture_renderer->glyph_native;
    owner.observer = &observer;
    GlyphFallbackHistory history;
    const auto a = history.append(0, 1, 1, 1);
    if (!glyph_fallback_submit(*state, a, history.lineage, history.inputs()))
      return dx12_fail(2, "family fallback completed page0 base");
    auto* base = state->texture_renderer->native_textures.at(a.get()).resource.Get();
    if (!glyph_fallback_full_copy(observer, *base, *a) || !glyph_fallback_readback(*state, *base, *a))
      return dx12_fail(3, "family fallback base full readback/debug gate");
    const auto a_successor = history.append(0, 2, 8, 1); // Not submitted: query old base without replacing it.
    (void)history.append(1, 3, 10, 10);
    const auto b = history.append(1, 4, 5, 5);
    const auto a_seal = history.lineage.find_snapshot(a_successor);
    const auto b_seal = history.lineage.find_snapshot(b);
    if (!a_seal || !b_seal || a_seal->family == b_seal->family || a_seal->page_index == b_seal->page_index ||
        a_seal->width != b_seal->width || a_seal->height != b_seal->height || b_seal->prefix != 2 ||
        owner.completed_base(*a_seal).resource.Get() != base || owner.completed_base(*b_seal).resource)
      return dx12_fail(4, "two valid same-extent families must not share completed base");
    const auto prior_ticket = state->fence_value;
    observer.clear();
    if (!glyph_fallback_submit(*state, b, history.lineage, history.inputs()))
      return dx12_fail(5, "different page-family native full fallback");
    const auto& native = state->texture_renderer->native_textures.at(b.get());
    if (!native.uploaded || native.resource.Get() == base || !native.srv.size() || state->submitted ||
        state->fence_value <= prior_ticket || owner.stats().completed_families != 2 ||
        owner.completed_base(*a_seal).resource.Get() != base || owner.completed_base(*a_seal).prefix != 1 ||
        !glyph_fallback_full_copy(observer, *native.resource.Get(), *b) ||
        !glyph_fallback_readback(*state, *native.resource.Get(), *b) || !glyph_fallback_readback(*state, *base, *a))
      return dx12_fail(6, "wrong family must not clone old bytes, erase old base or skip real fullP");
    const auto b_successor = history.append(1, 5, 1, 12);
    const auto next = history.lineage.find_snapshot(b_successor);
    if (!next || owner.completed_base(*next).resource.Get() != native.resource.Get() ||
        owner.completed_base(*next).prefix != 2 || !dx12_check_debug_errors(*state))
      return dx12_fail(7, "completion must promote only the compatible new family");
    std::cout << "private synthetic two-page-family control: old completed base retained; "
                 "actual fullP=256 clone=0; target/base whole-page readback and debug gates\n";
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
