#pragma once

#include <cstdint>

namespace cgpui {
class Dx12FrameCommandReuse;
class Dx12CommandReuseTestAccess;
struct Dx12CommandReuseSurfaceKey {
  std::uint32_t width = 0, height = 0;
  float scale = 1.0F;
  bool composition = false;
  bool transparent = false;
};
enum class Dx12CommandReuseSurfaceChange { invalid, unchanged, changed, exhausted };

// Pure accepted-surface identity. No parent proof or resource freeing is implied.
class Dx12CommandReuseSurfaceEpoch {
public:
  Dx12CommandReuseSurfaceEpoch() = default;
  Dx12CommandReuseSurfaceEpoch(const Dx12CommandReuseSurfaceEpoch&) = delete;
  Dx12CommandReuseSurfaceEpoch& operator=(const Dx12CommandReuseSurfaceEpoch&) = delete;
  Dx12CommandReuseSurfaceEpoch(Dx12CommandReuseSurfaceEpoch&&) = delete;
  Dx12CommandReuseSurfaceEpoch& operator=(Dx12CommandReuseSurfaceEpoch&&) = delete;
  [[nodiscard]] std::uint64_t value() const noexcept;
  [[nodiscard]] bool enabled() const noexcept;
  [[nodiscard]] const Dx12CommandReuseSurfaceKey& key() const noexcept;

private:
  friend class Dx12FrameCommandReuse;
  friend class Dx12CommandReuseTestAccess;
  Dx12CommandReuseSurfaceChange accept(const Dx12CommandReuseSurfaceKey& key) noexcept;
  Dx12CommandReuseSurfaceKey key_;
  std::uint64_t epoch_ = 0;
  bool exhausted_ = false;
};
} // namespace cgpui
