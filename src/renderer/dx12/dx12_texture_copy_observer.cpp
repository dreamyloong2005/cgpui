#include "dx12_texture_copy_observer_internal.hpp"
namespace cgpui {
Dx12TextureCopyObserver::Dx12TextureCopyObserver(std::span<Dx12TextureCopyEvent> storage) noexcept : storage_(storage) {}
bool Dx12TextureCopyObserver::capacity_for(std::size_t count) const noexcept {
  return count <= storage_.size() - used_;
}
void Dx12TextureCopyObserver::record(Dx12TextureCopyEvent event) noexcept {
  if (used_ == storage_.size()) { overflowed_ = true; return; }
  storage_[used_++] = event; // No callbacks, allocation, exceptions or reentry.
}
std::span<const Dx12TextureCopyEvent> Dx12TextureCopyObserver::events() const noexcept { return storage_.first(used_); }
bool Dx12TextureCopyObserver::overflowed() const noexcept { return overflowed_; }
void Dx12TextureCopyObserver::clear() noexcept { used_ = 0; overflowed_ = false; }
} // namespace cgpui
