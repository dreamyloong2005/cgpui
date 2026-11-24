#pragma once
#include "dx12_native_internal.hpp"
#include <cstddef>
#include <memory>

namespace cgpui {
struct Dx12NativeTexture;
class Dx12GlyphWorkObserver;
struct Dx12TextureRenderer;
// Owns native references/real map-node leases, never strong CPU snapshots.
class Dx12GlyphCommandDependencies {
 public:
  Dx12GlyphCommandDependencies();
  ~Dx12GlyphCommandDependencies();
  Dx12GlyphCommandDependencies(const Dx12GlyphCommandDependencies&) = delete;
  Dx12GlyphCommandDependencies& operator=(const Dx12GlyphCommandDependencies&) = delete;
  void pin(Dx12NativeTexture&); // Fallible preparation before first command reference.
  void reserve_resources(std::size_t additional);
  void retain(ID3D12Resource&) noexcept; // Requires pre-reserved slot.
  void prune_expired(Dx12TextureRenderer&) noexcept;
  void parent_reset_or_release() noexcept; // Caller supplies G AND P.
  std::size_t pins() const noexcept;
  std::size_t detached_nodes() const noexcept;
  std::size_t resources() const noexcept;
  void observe_textures(Dx12GlyphWorkObserver&) const noexcept;
 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};
} // namespace cgpui
