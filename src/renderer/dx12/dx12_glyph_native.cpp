#include "dx12_glyph_native_internal.hpp"
#include "dx12_glyph_work_observer_internal.hpp"
#include <cassert>
#include <map>
#include <stdexcept>
#include <utility>
#include <vector>

namespace cgpui {
struct Dx12GlyphNativeEpoch {};
struct Dx12GlyphNative::Impl {
  struct Catalog {
    std::size_t page = 0, prefix = 0;
    std::uint32_t width = 0, height = 0;
    std::shared_ptr<const Dx12GlyphNativeEpoch> epoch;
    Dx12Ptr<ID3D12Resource> resource;
  };
  struct Candidate { Catalog* slot; std::size_t prefix; Dx12Ptr<ID3D12Resource> resource; };
  std::map<std::shared_ptr<const Dx12GlyphFamily>, Catalog,
           std::owner_less<std::shared_ptr<const Dx12GlyphFamily>>> catalog;
  std::vector<Candidate> candidates;
  Dx12GlyphCommandDependencies dependencies;
  std::shared_ptr<const Dx12GlyphNativeEpoch> epoch;
  // One non-recycled recording identity until P; pointers cannot ABA while their node is retained.
  std::unique_ptr<unsigned char> recording;
  bool active = false, executed = false, open = false;
  std::size_t close_attempts = 0;
  HRESULT last_close = S_OK;
};
Dx12GlyphNative::Dx12GlyphNative() : impl_(std::make_unique<Impl>()) {}
Dx12GlyphNative::~Dx12GlyphNative() = default;
bool Dx12GlyphNative::enter_scope() noexcept {
  if (impl_->active) return false;
  impl_->active = true; return true;
}
void Dx12GlyphNative::leave_scope() noexcept { impl_->active = false; }
void Dx12GlyphNative::parent_reset() noexcept {
  assert(!impl_->executed);
  impl_->dependencies.parent_reset_or_release();
  impl_->candidates.clear();
  impl_->recording.reset();
  impl_->open = true;
  impl_->close_attempts = 0; impl_->last_close = S_OK;
}
void Dx12GlyphNative::begin_recording(const Dx12GlyphLineage& lineage) {
  if (!impl_->epoch) impl_->epoch = std::make_shared<const Dx12GlyphNativeEpoch>();
  impl_->recording = std::make_unique<unsigned char>(0);
  for (auto it = impl_->catalog.begin(); it != impl_->catalog.end();) {
    const auto& slot = it->second;
    const Dx12GlyphSnapshot family{.family = it->first, .page_index = slot.page,
                                  .width = slot.width, .height = slot.height};
    if (!lineage.current(family)) it = impl_->catalog.erase(it); else ++it;
  }
}
void Dx12GlyphNative::record_close(HRESULT result) noexcept {
  ++impl_->close_attempts; impl_->last_close = result;
}
void Dx12GlyphNative::mark_executed() noexcept { impl_->executed = true; impl_->open = false; }
void Dx12GlyphNative::commit_completed() noexcept {
  if (!impl_->executed) return;
  for (auto& candidate : impl_->candidates) {
    auto& slot = *candidate.slot;
    if (!slot.resource || candidate.prefix > slot.prefix) {
      slot.prefix = candidate.prefix;
      slot.resource = std::move(candidate.resource);
    }
  }
  impl_->executed = false;
  // Candidates and every recorded source/staging/node owner remain until parent_reset.
}
void Dx12GlyphNative::abandon_unexecuted() noexcept { impl_->open = false; }
void Dx12GlyphNative::reset_after_parent_release() noexcept {
  impl_->dependencies.parent_reset_or_release();
  impl_->candidates.clear(); impl_->catalog.clear(); impl_->recording.reset(); impl_->epoch.reset();
  impl_->executed = false; impl_->open = false;
}
void Dx12GlyphNative::pin(Dx12NativeTexture& native) {
  assert(impl_->active && impl_->open && impl_->recording);
  impl_->dependencies.pin(native);
}
void Dx12GlyphNative::reserve_resources(std::size_t count) { impl_->dependencies.reserve_resources(count); }
void Dx12GlyphNative::retain(ID3D12Resource& resource) noexcept { impl_->dependencies.retain(resource); }
void Dx12GlyphNative::prune_expired(Dx12TextureRenderer& renderer) noexcept {
  assert(!impl_->executed);
  impl_->dependencies.prune_expired(renderer);
}
Dx12GlyphNativeBase Dx12GlyphNative::completed_base(const Dx12GlyphSnapshot& target) const noexcept {
  const auto found = impl_->catalog.find(target.family);
  if (found == impl_->catalog.end()) return {};
  const auto& slot = found->second;
  if (!slot.resource || slot.epoch != impl_->epoch || slot.page != target.page_index ||
      slot.width != target.width || slot.height != target.height || slot.prefix >= target.prefix) return {};
  return {slot.resource, slot.prefix};
}
void Dx12GlyphNative::prepare_candidate(const Dx12GlyphSnapshot& target) {
  auto [it, inserted] = impl_->catalog.try_emplace(target.family);
  if (inserted) it->second = {.page = target.page_index, .width = target.width,
                             .height = target.height, .epoch = impl_->epoch};
  assert(it->second.epoch == impl_->epoch && it->second.width == target.width &&
         it->second.height == target.height);
  if (impl_->candidates.size() == impl_->candidates.max_size()) throw std::length_error("glyph candidates");
  impl_->candidates.reserve(impl_->candidates.size() + 1);
}
void Dx12GlyphNative::publish_candidate(const Dx12GlyphSnapshot& target, ID3D12Resource& resource) noexcept {
  const auto found = impl_->catalog.find(target.family);
  assert(found != impl_->catalog.end() && impl_->candidates.size() < impl_->candidates.capacity());
  Dx12Ptr<ID3D12Resource> retained = &resource;
  impl_->candidates.push_back({&found->second, target.prefix, std::move(retained)});
}
Dx12GlyphNativeStats Dx12GlyphNative::stats() const noexcept {
  std::size_t completed = 0;
  for (const auto& [family, slot] : impl_->catalog) completed += bool(slot.resource);
  return {completed, impl_->candidates.size(), impl_->dependencies.pins(),
      impl_->dependencies.detached_nodes(), impl_->dependencies.resources(), impl_->executed, impl_->active,
      impl_->close_attempts, impl_->last_close};
}
void Dx12GlyphNative::observe_owned_textures(Dx12GlyphWorkObserver& observer) const noexcept {
  for (const auto& [family, slot] : impl_->catalog) observer.include_texture(slot.resource.Get());
  for (const auto& candidate : impl_->candidates) observer.include_texture(candidate.resource.Get());
  impl_->dependencies.observe_textures(observer);
}
} // namespace cgpui
