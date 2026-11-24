#include "dx12_glyph_native_test_support.hpp"
#include "dx12_glyph_cpu_test_support.hpp"
#include <array>
#include <new>

using namespace cgpui;
using namespace cgpui_test;
int main() {
  auto state = texture_upload_state(); // Real device/heap, but no native Execute in this ownership-only test.
  if (!state) return dx12_fail(1, "node lease fixture");
  auto& renderer = *state->texture_renderer;
  Dx12GlyphCommandDependencies dependencies;
  alignas(Dx12Texture) std::byte storage[sizeof(Dx12Texture)];
  const auto at_same_address = [&] {
    return Dx12TextureHandle{new (storage) Dx12Texture{}, [](const Dx12Texture* value) { value->~Dx12Texture(); }};
  };
  auto source = at_same_address();
  std::weak_ptr<const Dx12Texture> weak = source;
  const auto* key = source.get();
  auto [it, inserted] = renderer.native_textures.try_emplace(key);
  auto& native = it->second;
  native.source = source;
  auto lease = state->srv_pool->allocate(*state->device.Get(), 1);
  if (!lease) return dx12_fail(2, "node lease allocation");
  const auto slot = lease->gpu().ptr;
  native.srv = std::move(*lease);
  dependencies.pin(native);
  // Rehash must not invalidate the pin's map-value identity.
  renderer.native_textures.reserve(2048);
  source.reset();
  dependencies.prune_expired(renderer);
  if (!weak.expired() || !renderer.native_textures.empty() || dependencies.detached_nodes() != 1 ||
      state->srv_pool->stats().live_slots != 1) return dx12_fail(3, "expired pinned node released lease before P");
  // Reuse the exact raw key while old detached node remains owned; no reinsertion/lookup of detached node.
  auto replacement = at_same_address();
  if (replacement.get() != key) return dx12_fail(9, "CPU address reuse fixture");
  auto [fresh, added] = renderer.native_textures.try_emplace(key);
  fresh->second.source = replacement;
  auto second = state->srv_pool->allocate(*state->device.Get(), 1);
  if (!added || !second || second->gpu().ptr == slot) return dx12_fail(4, "detached raw key/slot collided");
  fresh->second.srv = std::move(*second);
  dependencies.pin(fresh->second); dependencies.pin(fresh->second);
  if (dependencies.pins() != 2) return dx12_fail(5, "exact node pin did not deduplicate by identity");
  // A failed/omitted parent Reset supplies no P: another prune cannot return old slot.
  dependencies.prune_expired(renderer);
  if (state->srv_pool->stats().live_slots != 2 || dependencies.detached_nodes() != 1)
    return dx12_fail(6, "failed Reset simulation lost detached ownership");
  dependencies.parent_reset_or_release(); // No commands were executed; explicit synthetic G+P boundary.
  if (state->srv_pool->stats().live_slots != 1 || !renderer.native_textures.contains(key))
    return dx12_fail(7, "P erased live replacement or failed to return old lease exactly once");
  auto recycled = state->srv_pool->allocate(*state->device.Get(), 1);
  if (!recycled || recycled->gpu().ptr != slot) return dx12_fail(8, "P did not release original slot");
  return 0;
}
