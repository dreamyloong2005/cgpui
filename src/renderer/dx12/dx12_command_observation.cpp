#include "dx12_command_observation_internal.hpp"
#include "dx12_state_internal.hpp"
#include <limits>

namespace cgpui {
bool Dx12CommandVerifier::submission_pending() const noexcept { return submitted_; }
struct Dx12CommandObservation::Impl {
  std::unique_ptr<Dx12CommandEvent[]> events;
  std::size_t capacity = 0, used = 0;
  std::uint64_t parent = 0, tail_parent = 0;
  bool overflow = false, tail_pending = false;
};
Dx12CommandObservation::Dx12CommandObservation(std::size_t capacity) : impl_(std::make_unique<Impl>()) {
  impl_->events = std::make_unique<Dx12CommandEvent[]>(capacity); impl_->capacity = capacity;
}
Dx12CommandObservation::~Dx12CommandObservation() = default;
void Dx12CommandObservation::record(Dx12CommandEvent event) noexcept {
  if (!event.parent) event.parent = impl_->parent;
  if (event.list) event.type = event.list->GetType();
  if (impl_->used == impl_->capacity) { impl_->overflow = true; return; }
  impl_->events[impl_->used++] = event;
}
void Dx12CommandObservation::parent_reset(ID3D12GraphicsCommandList& list, ID3D12CommandAllocator& allocator) noexcept {
  if (impl_->parent == std::numeric_limits<std::uint64_t>::max()) { impl_->overflow = true; return; }
  ++impl_->parent;
  record({.kind = Dx12CommandEventKind::parent_reset, .list = &list, .allocator = &allocator});
}
ID3D12CommandList* Dx12CommandObservation::prepare_tail(Dx12State& state, ID3D12Resource& target) noexcept {
  if (!verifier || impl_->tail_pending) return nullptr;
  verifier->prepare(state, target, impl_->parent);
  impl_->tail_parent = impl_->parent;
  return verifier->prepared_list();
}
void Dx12CommandObservation::tail_executed() noexcept {
  impl_->tail_pending = true; verifier->submitted_ = true; // After the actual tail Execute only.
}
void Dx12CommandObservation::completed(Dx12State& state, std::uint64_t ticket) noexcept {
  record({.kind = Dx12CommandEventKind::completed, .a = ticket});
  if (impl_->tail_pending) {
    impl_->tail_pending = false;
    verifier->completed(state, impl_->tail_parent, ticket);
    verifier->submitted_ = false;
  }
}
void Dx12CommandObservation::retire(bool removed) noexcept {
  // Only the central native-retirement owner calls after completion/removal/never-executed proof.
  if (verifier) verifier->retired(removed);
  if (verifier) verifier->submitted_ = false;
  impl_->tail_pending = false;
}
std::uint64_t Dx12CommandObservation::parent_epoch() const noexcept { return impl_->parent; }
std::span<const Dx12CommandEvent> Dx12CommandObservation::events() const noexcept {
  return {impl_->events.get(), impl_->used};
}
bool Dx12CommandObservation::overflowed() const noexcept { return impl_->overflow; }
void dx12_observe_command(Dx12State& state, Dx12CommandEvent event) noexcept {
  if (state.command_observation) state.command_observation->record(event);
}
} // namespace cgpui
