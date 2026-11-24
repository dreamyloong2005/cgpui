#pragma once
#include "dx12_glyph_lineage_internal.hpp"
#include "dx12_glyph_command_dependencies_internal.hpp"
#include "dx12_texture_copy_observer_internal.hpp"
#include <memory>

namespace cgpui {
struct Dx12NativeTexture;
struct Dx12GlyphNativeEpoch;
class Dx12GlyphCommandApi;
class Dx12GlyphWorkObserver;
struct Dx12GlyphNativeBase {
  Dx12Ptr<ID3D12Resource> resource;
  std::size_t prefix = 0;
};
struct Dx12GlyphNativeStats {
  std::size_t completed_families = 0, candidates = 0, pins = 0, detached_nodes = 0, resources = 0;
  bool executed_unknown = false, scope_active = false;
  std::size_t close_attempts = 0;
  HRESULT last_close = S_OK;
};

class Dx12GlyphNative {
 public:
  Dx12GlyphNative();
  ~Dx12GlyphNative();
  bool enter_scope() noexcept;
  void leave_scope() noexcept;
  void parent_reset() noexcept; // G was proven by preceding idle wait; Reset supplies P.
  void begin_recording(const Dx12GlyphLineage&);
  void record_close(HRESULT) noexcept;
  void mark_executed() noexcept;
  void commit_completed() noexcept; // Publishes progress only. Does NOT release dependencies.
  void abandon_unexecuted() noexcept;
  void reset_after_parent_release() noexcept;
  void pin(Dx12NativeTexture&);
  void reserve_resources(std::size_t);
  void retain(ID3D12Resource&) noexcept;
  void prune_expired(Dx12TextureRenderer&) noexcept;
  Dx12GlyphNativeBase completed_base(const Dx12GlyphSnapshot&) const noexcept;
  void prepare_candidate(const Dx12GlyphSnapshot&); // Create actual catalog node + reserve candidate.
  void publish_candidate(const Dx12GlyphSnapshot&, ID3D12Resource&) noexcept;
  Dx12GlyphNativeStats stats() const noexcept;
  void observe_owned_textures(Dx12GlyphWorkObserver&) const noexcept;
  Dx12GlyphWorkObserver* work_observer = nullptr; // Borrowed bounded success-work diagnostics only.
  Dx12GlyphCommandApi* command_api = nullptr; // Borrowed private seam; must outlive this owner.
  Dx12TextureCopyObserver* observer = nullptr; // Private bounded borrowed sink; no production callback.
 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};
} // namespace cgpui
