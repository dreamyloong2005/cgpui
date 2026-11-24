#include "dx12_command_reuse_lifetime_internal.hpp"
#include <limits>

namespace cgpui {
std::uint64_t Dx12CommandReuseLifetime::generation() const noexcept { return generation_; }
std::uint64_t Dx12CommandReuseLifetime::entry_epoch() const noexcept { return entry_; }
std::uint64_t Dx12CommandReuseLifetime::parent_epoch() const noexcept { return parent_; }
std::uint64_t Dx12CommandReuseLifetime::submission_epoch() const noexcept { return submission_; }
bool Dx12CommandReuseLifetime::enabled() const noexcept { return enabled_; }
bool Dx12CommandReuseLifetime::gpu_retired() const noexcept { return gpu_ != GpuUse::pending; }
bool Dx12CommandReuseLifetime::parent_detached() const noexcept { return detached_; }
bool Dx12CommandReuseLifetime::can_release_entry() const noexcept {
  return entry_ != 0 && gpu_retired() && parent_detached();
}
bool Dx12CommandReuseLifetime::can_reuse_entry() const noexcept {
  return enabled_ && published_ && !invalidated_ && gpu_ == GpuUse::completed &&
      submission_ != 0 && published_submission_ == submission_ && can_release_entry();
}
bool Dx12CommandReuseLifetime::matches_entry(
    std::uint64_t generation, std::uint64_t entry) const noexcept {
  return generation != 0 && generation == generation_ && entry != 0 && entry == entry_;
}
bool Dx12CommandReuseLifetime::matches_parent(
    std::uint64_t generation, std::uint64_t parent) const noexcept {
  return entry_ != 0 && generation != 0 && generation == generation_ &&
      parent != 0 && parent == parent_;
}
bool Dx12CommandReuseLifetime::begin_generation(std::uint64_t generation) noexcept {
  if (entry_ || !generation || generation <= generation_) return false;
  generation_ = generation;
  enabled_ = true;
  return true;
}
bool Dx12CommandReuseLifetime::create_entry(std::uint64_t generation) noexcept {
  if (!enabled_ || generation != generation_ || entry_) return false;
  if (last_entry_ == std::numeric_limits<std::uint64_t>::max()) {
    enabled_ = false;
    return false;
  }
  entry_ = ++last_entry_;
  parent_ = submission_ = published_submission_ = 0;
  gpu_ = GpuUse::never_submitted;
  detached_ = true;
  published_ = false;
  invalidated_ = false;
  return true;
}
bool Dx12CommandReuseLifetime::attach_parent(
    std::uint64_t generation, std::uint64_t entry, std::uint64_t parent) noexcept {
  if (!matches_entry(generation, entry) || !enabled_ || invalidated_ ||
      !can_release_entry() || (parent_ && !can_reuse_entry())) return false;
  if (last_parent_ == std::numeric_limits<std::uint64_t>::max() ||
      last_submission_ == std::numeric_limits<std::uint64_t>::max()) {
    enabled_ = false;
    invalidated_ = true;
    return false;
  }
  if (!parent || parent <= last_parent_) return false;
  parent_ = last_parent_ = parent;
  // Prior success permits this attachment, not publication of the new attempt.
  submission_ = published_submission_ = 0;
  published_ = false;
  gpu_ = GpuUse::never_submitted;
  detached_ = false;
  return true;
}
bool Dx12CommandReuseLifetime::reserve_submission(
    std::uint64_t generation, std::uint64_t parent, std::uint64_t submission) noexcept {
  if (!matches_parent(generation, parent) || !enabled_ || invalidated_ || detached_ ||
      gpu_ != GpuUse::never_submitted || !submission || submission <= last_submission_)
    return false;
  submission_ = last_submission_ = submission;
  gpu_ = GpuUse::pending;
  return true;
}
bool Dx12CommandReuseLifetime::completed(
    std::uint64_t generation, std::uint64_t parent, std::uint64_t submission) noexcept {
  if (!matches_parent(generation, parent) || !submission || submission != submission_ ||
      gpu_ != GpuUse::pending) return false;
  gpu_ = GpuUse::completed;
  return true;
}
bool Dx12CommandReuseLifetime::removed(std::uint64_t generation) noexcept {
  if (!generation || generation != generation_) return false;
  enabled_ = false;
  invalidated_ = true;
  gpu_ = GpuUse::removed;
  return true;
}
bool Dx12CommandReuseLifetime::parent_reset_succeeded(
    std::uint64_t generation, std::uint64_t parent) noexcept {
  if (!gpu_retired()) return false;
  return parent_released(generation, parent);
}
bool Dx12CommandReuseLifetime::parent_released(
    std::uint64_t generation, std::uint64_t parent) noexcept {
  if (!matches_parent(generation, parent) || detached_) return false;
  detached_ = true;
  return true;
}
bool Dx12CommandReuseLifetime::publish_success(std::uint64_t generation, std::uint64_t entry,
    std::uint64_t parent, std::uint64_t submission) noexcept {
  if (!matches_entry(generation, entry) || !matches_parent(generation, parent) ||
      !enabled_ || invalidated_ || detached_ || gpu_ != GpuUse::completed ||
      !submission || submission != submission_ || submission == published_submission_) return false;
  published_ = true;
  published_submission_ = submission;
  return true;
}
void Dx12CommandReuseLifetime::invalidate() noexcept { invalidated_ = true; }
bool Dx12CommandReuseLifetime::forget_released_entry(
    std::uint64_t generation, std::uint64_t entry) noexcept {
  if (!matches_entry(generation, entry) || !can_release_entry()) return false;
  // The owning entry releases native objects first. High-water identities survive.
  entry_ = parent_ = submission_ = published_submission_ = 0;
  gpu_ = GpuUse::never_submitted;
  detached_ = true;
  published_ = false;
  invalidated_ = true;
  return true;
}
} // namespace cgpui
