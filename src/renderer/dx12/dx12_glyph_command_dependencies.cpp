#include "dx12_glyph_command_dependencies_internal.hpp"
#include "dx12_texture_renderer_internal.hpp"
#include "dx12_glyph_work_observer_internal.hpp"
#include <cassert>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace cgpui {
struct Dx12GlyphCommandDependencies::Impl {
  using Map = decltype(Dx12TextureRenderer::native_textures);
  using Node = Map::node_type;
  struct Pin {
    Dx12NativeTexture* identity;
    Node detached;
  };
  // Each pointee stays stable across vector growth; no cached reference into extracted value.
  std::vector<std::unique_ptr<Pin>> pins;
  std::vector<Dx12Ptr<ID3D12Resource>> resources;
};
static_assert(std::is_nothrow_default_constructible_v<decltype(Dx12TextureRenderer::native_textures)::node_type>);
static_assert(std::is_nothrow_move_assignable_v<decltype(Dx12TextureRenderer::native_textures)::node_type>);
static_assert(std::is_nothrow_destructible_v<decltype(Dx12TextureRenderer::native_textures)::node_type>);
static_assert(std::is_nothrow_invocable_v<decltype(Dx12TextureRenderer::native_textures)::hasher,
                                        const Dx12Texture*>);
Dx12GlyphCommandDependencies::Dx12GlyphCommandDependencies() : impl_(std::make_unique<Impl>()) {}
Dx12GlyphCommandDependencies::~Dx12GlyphCommandDependencies() = default;
void Dx12GlyphCommandDependencies::pin(Dx12NativeTexture& native) {
  for (const auto& pin : impl_->pins) if (pin->identity == &native) return;
  impl_->pins.push_back(std::make_unique<Impl::Pin>(Impl::Pin{&native, {}}));
}
void Dx12GlyphCommandDependencies::reserve_resources(std::size_t additional) {
  if (additional > impl_->resources.max_size() - impl_->resources.size()) throw std::length_error("glyph dependencies");
  impl_->resources.reserve(impl_->resources.size() + additional);
}
void Dx12GlyphCommandDependencies::retain(ID3D12Resource& resource) noexcept {
  assert(impl_->resources.size() < impl_->resources.capacity());
  Dx12Ptr<ID3D12Resource> retained = &resource;
  impl_->resources.push_back(std::move(retained));
}
void Dx12GlyphCommandDependencies::prune_expired(Dx12TextureRenderer& renderer) noexcept {
  for (auto it = renderer.native_textures.begin(); it != renderer.native_textures.end();) {
    if (!it->second.source.expired()) { ++it; continue; }
    Impl::Pin* owner = nullptr;
    for (const auto& pin : impl_->pins) if (pin->identity == &it->second) { owner = pin.get(); break; }
    if (!owner) { it = renderer.native_textures.erase(it); continue; }
    assert(owner->detached.empty());
    const auto expired = it++;
    // MSVC iterator extract hashes the pointer. Concrete default pointer hash above is noexcept.
    // No key-based extraction, slot growth, reinsertion or raw-key identity lookup occurs here.
    owner->detached = renderer.native_textures.extract(expired);
    owner->identity = nullptr; // Access detached data only through its node handle henceforth.
  }
}
void Dx12GlyphCommandDependencies::parent_reset_or_release() noexcept {
  impl_->pins.clear();
  impl_->resources.clear();
}
std::size_t Dx12GlyphCommandDependencies::pins() const noexcept { return impl_->pins.size(); }
std::size_t Dx12GlyphCommandDependencies::detached_nodes() const noexcept {
  std::size_t count = 0;
  for (const auto& pin : impl_->pins) count += !pin->detached.empty();
  return count;
}
std::size_t Dx12GlyphCommandDependencies::resources() const noexcept { return impl_->resources.size(); }
void Dx12GlyphCommandDependencies::observe_textures(Dx12GlyphWorkObserver& observer) const noexcept {
  for (const auto& pin : impl_->pins)
    if (!pin->detached.empty()) observer.include_texture(pin->detached.mapped().resource.Get());
  for (const auto& resource : impl_->resources) observer.include_texture(resource.Get());
}
} // namespace cgpui
