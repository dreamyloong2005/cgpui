#pragma once

#include <cstdint>

namespace cgpui {
class Dx12FrameCommandReuse;
class Dx12CommandReuseTestAccess;

// Metadata only. Native owners supply the proofs; this does not own a COM list,
// wait for GPU work, or authorize destroying an entry without native retirement.
// One entry and one referencing DIRECT recording, under the state operation lock.
class Dx12CommandReuseLifetime {
public:
  Dx12CommandReuseLifetime() = default;
  Dx12CommandReuseLifetime(const Dx12CommandReuseLifetime&) = delete;
  Dx12CommandReuseLifetime& operator=(const Dx12CommandReuseLifetime&) = delete;
  Dx12CommandReuseLifetime(Dx12CommandReuseLifetime&&) = delete;
  Dx12CommandReuseLifetime& operator=(Dx12CommandReuseLifetime&&) = delete;

  [[nodiscard]] std::uint64_t generation() const noexcept;
  [[nodiscard]] std::uint64_t entry_epoch() const noexcept;
  [[nodiscard]] std::uint64_t parent_epoch() const noexcept;
  [[nodiscard]] std::uint64_t submission_epoch() const noexcept;
  [[nodiscard]] bool enabled() const noexcept;
  [[nodiscard]] bool gpu_retired() const noexcept;
  [[nodiscard]] bool parent_detached() const noexcept;
  [[nodiscard]] bool can_release_entry() const noexcept;
  [[nodiscard]] bool can_reuse_entry() const noexcept;

private:
  friend class Dx12FrameCommandReuse;
  friend class Dx12CommandReuseTestAccess; // Defined only in test support.
  enum class GpuUse { never_submitted, pending, completed, removed };

  bool begin_generation(std::uint64_t generation) noexcept;
  bool create_entry(std::uint64_t generation) noexcept;
  bool attach_parent(std::uint64_t generation, std::uint64_t entry,
                     std::uint64_t parent) noexcept;
  // All fallible preparation must finish first. On success the native owner must
  // immediately forward ExecuteCommandLists without fallible/callback work.
  // Conservatively consumes G BEFORE execution, so no post-Execute mutation can
  // fail or allocate. No cancellation shortcut restores never-submitted proof.
  bool reserve_submission(std::uint64_t generation, std::uint64_t parent,
                          std::uint64_t submission) noexcept;
  bool completed(std::uint64_t generation, std::uint64_t parent,
                 std::uint64_t submission) noexcept;
  bool removed(std::uint64_t generation) noexcept;
  bool parent_reset_succeeded(std::uint64_t generation, std::uint64_t parent) noexcept;
  bool parent_released(std::uint64_t generation, std::uint64_t parent) noexcept;
  bool publish_success(std::uint64_t generation, std::uint64_t entry,
                       std::uint64_t parent, std::uint64_t submission) noexcept;
  void invalidate() noexcept;
  bool forget_released_entry(std::uint64_t generation, std::uint64_t entry) noexcept;
  bool matches_entry(std::uint64_t generation, std::uint64_t entry) const noexcept;
  bool matches_parent(std::uint64_t generation, std::uint64_t parent) const noexcept;

  std::uint64_t generation_ = 0;
  std::uint64_t entry_ = 0, last_entry_ = 0;
  std::uint64_t parent_ = 0, last_parent_ = 0;
  std::uint64_t submission_ = 0, last_submission_ = 0;
  std::uint64_t published_submission_ = 0;
  GpuUse gpu_ = GpuUse::never_submitted;
  bool enabled_ = false;
  bool detached_ = true;
  bool published_ = false;
  bool invalidated_ = true;
};
} // namespace cgpui
