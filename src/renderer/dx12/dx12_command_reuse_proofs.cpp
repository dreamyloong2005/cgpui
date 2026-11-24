#include "dx12_frame_command_reuse_state_internal.hpp"
#include "dx12_state_internal.hpp"
#include <cassert>
#include <limits>

namespace cgpui {
void Dx12FrameCommandReuse::parent_reset(Dx12State&) noexcept {
  auto& life = impl_->lifetime;
  if (life.entry_epoch() && !life.parent_detached()) {
    const bool detached = life.parent_reset_succeeded(life.generation(), life.parent_epoch());
    assert(detached); (void)detached;
  }
  impl_->attached = false;
  if (impl_->parent == std::numeric_limits<std::uint64_t>::max() ||
      impl_->submission == std::numeric_limits<std::uint64_t>::max()) impl_->exhausted = true;
  else ++impl_->parent;
}
bool Dx12FrameCommandReuse::prearm(Dx12State& state) noexcept {
  if (!impl_->attached) return true;
  if (impl_->submission == std::numeric_limits<std::uint64_t>::max()) return false;
  const auto next = impl_->submission + 1;
  if (!impl_->lifetime.reserve_submission(state.info.device_generation, impl_->parent, next)) return false;
  impl_->submission = next; return true;
}
void Dx12FrameCommandReuse::completed(Dx12State& state) noexcept {
  auto& life = impl_->lifetime;
  if (!impl_->attached || life.gpu_retired()) return;
  const bool done = life.completed(state.info.device_generation, impl_->parent, impl_->submission);
  assert(done); (void)done;
}
void Dx12FrameCommandReuse::removed(Dx12State&) noexcept {
  if (impl_->lifetime.generation()) (void)impl_->lifetime.removed(impl_->lifetime.generation());
}
void Dx12FrameCommandReuse::parent_released(Dx12State&) noexcept {
  auto& life = impl_->lifetime;
  if (life.entry_epoch() && !life.parent_detached()) {
    const bool detached = life.parent_released(life.generation(), life.parent_epoch());
    assert(detached); (void)detached;
  }
  // Existing central caller already proved actual completed/removal/never-executed G.
  // Do not manufacture G from list release: a pending ledger remains a hard invariant failure.
  assert(!life.entry_epoch() || life.can_release_entry());
  discard_retired(); impl_->attached = false;
}
} // namespace cgpui
