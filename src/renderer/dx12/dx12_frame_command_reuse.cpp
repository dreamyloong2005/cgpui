#include "dx12_frame_command_reuse_state_internal.hpp"
#include "dx12_state_internal.hpp"
#include <cassert>

namespace cgpui {
Dx12FrameCommandReuse::Dx12FrameCommandReuse() : impl_(std::make_unique<Impl>()) {}
Dx12FrameCommandReuse::~Dx12FrameCommandReuse() = default;
void Dx12FrameCommandReuse::discard_retired() noexcept {
  auto& life = impl_->lifetime;
  if (!life.entry_epoch()) return;
  assert(life.can_release_entry());
  impl_->entry.reset(); // Native list/allocator/storage/bindings/pins FIRST; metadata afterward.
  const bool forgot = life.forget_released_entry(life.generation(), life.entry_epoch());
  assert(forgot); (void)forgot;
}
void Dx12FrameCommandReuse::accept_surface(const Dx12CommandReuseSurfaceKey& key) noexcept {
  const auto change = impl_->surface.accept(key);
  if (change != Dx12CommandReuseSurfaceChange::unchanged) impl_->lifetime.invalidate();
}
Result<bool> Dx12FrameCommandReuse::record(Dx12State& state, Color clear,
    std::span<const Dx12DrawCommand> draws, bool capture, bool mutations) {
  auto& life = impl_->lifetime;
  if (life.generation() != state.info.device_generation) {
    if (life.entry_epoch()) return std::unexpected(dx12_error(E_UNEXPECTED, "reuse stale generation owner"));
    if (!life.begin_generation(state.info.device_generation)) return false;
  }
  accept_surface({state.width, state.height, state.scale.value, state.composition_surface, state.transparent_background});
  auto selected = dx12_select_reuse_scene(state, clear, draws, capture, mutations,
      impl_->surface.enabled() ? impl_->surface.value() : 0);
  if (!selected) return std::unexpected(selected.error());
  if (!*selected || impl_->exhausted) {
    life.invalidate(); discard_retired(); return false;
  }
  const bool hit = impl_->entry && life.can_reuse_entry() &&
      dx12_reuse_signature_equal(impl_->entry->scene(), **selected);
  if (!hit) {
    life.invalidate(); discard_retired();
    if (!life.create_entry(state.info.device_generation)) return false; // Identity exhaustion disables optimization only.
    auto candidate = Dx12CommandBundle::create(state, std::move(**selected), draws);
    if (!candidate) return std::unexpected(candidate.error());
    impl_->entry = std::move(*candidate); // State owns the closed bundle BEFORE any ExecuteBundle reference.
  }
  if (!life.attach_parent(state.info.device_generation, life.entry_epoch(), impl_->parent))
    return std::unexpected(dx12_error(E_UNEXPECTED, "reuse parent attachment"));
  impl_->attached = true;
  impl_->entry->execute(state);
  return true;
}
void Dx12FrameCommandReuse::publish_success(Dx12State& state) noexcept {
  if (!impl_->attached) return;
  auto& life = impl_->lifetime;
  const bool published = life.publish_success(state.info.device_generation, life.entry_epoch(), impl_->parent, impl_->submission);
  assert(published); (void)published;
}
} // namespace cgpui
