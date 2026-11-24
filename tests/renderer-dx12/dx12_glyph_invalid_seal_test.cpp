#include "dx12_glyph_fallback_controls_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;
int main() {
  try {
    if (!glyph_fallback_enable_debug()) return dx12_fail(1, "invalid seal debug configuration");
    std::array<Dx12TextureCopyEvent, 32> events{};
    Dx12TextureCopyObserver observer(events);
    Dx12Window window;
    auto state = srv_native_state(&window);
    if (!state) return dx12_fail(1, "invalid seal native fixture");
    auto& renderer = *state->texture_renderer;
    auto& owner = *renderer.glyph_native;
    owner.observer = &observer;
    GlyphFallbackHistory history;
    const auto a = history.append(0, 1, 1, 1);
    if (!glyph_fallback_submit(*state, a, history.lineage, history.inputs()))
      return dx12_fail(2, "invalid seal prior completed base");
    auto* base = renderer.native_textures.at(a.get()).resource.Get();
    if (!glyph_fallback_full_copy(observer, *base, *a) || !glyph_fallback_readback(*state, *base, *a))
      return dx12_fail(3, "invalid seal base full readback/debug gate");
    const auto successor = history.append(0, 2, 8, 1);
    auto invalid_value = *successor;
    invalid_value.format = DXGI_FORMAT_R8G8B8A8_UNORM;
    const auto invalid = std::make_shared<const Dx12Texture>(std::move(invalid_value));
    // Private lineage seal checks dimensions/stride/size, not R8. Native prepare
    // must reject this cross-field inconsistency. No published const object changes.
    if (!history.lineage.seal_snapshot(0, invalid) || !history.lineage.find_snapshot(invalid))
      return dx12_fail(4, "invalid sealed-format premise changed; do not manufacture a bypass hook");
    const auto prior_ticket = state->fence_value;
    const auto prior_slots = state->srv_pool->stats().live_slots;
    observer.clear();
    const auto denied = glyph_fallback_submit(*state, invalid, history.lineage, history.inputs());
    const auto& target = renderer.native_textures.at(invalid.get());
    const auto stats = owner.stats();
    if (denied || denied.error().code != ErrorCode::invalid_argument ||
        denied.error().message.find("sealed family") == std::string::npos || target.resource ||
        target.srv.size() || target.uploaded || !renderer.pending_uploads.empty() || !observer.events().empty() ||
        observer.overflowed() || state->submitted || state->fence_value != prior_ticket ||
        state->srv_pool->stats().live_slots != prior_slots || stats.candidates || stats.pins || stats.resources ||
        stats.executed_unknown || stats.scope_active || stats.close_attempts != 1 || FAILED(stats.last_close) ||
        stats.completed_families != 1)
      return dx12_fail(5, "invalid sealed metadata must error before allocation/copy/Execute, not full-fallback");
    if (!dx12_check_debug_errors(*state) || !glyph_fallback_readback(*state, *base, *a))
      return dx12_fail(6, "invalid control cleanup/held-base native readback debug gate");
    std::cout << "private synthetic invalid sealed R8-format control: error before native target/Execute; "
                 "prior completed full-page bytes and cleanup debug gate retained\n";
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
