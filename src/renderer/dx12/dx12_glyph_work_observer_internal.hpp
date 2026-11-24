#pragma once
#include "dx12_native_internal.hpp"
#include <cstddef>
#include <span>

namespace cgpui {
struct Dx12State;
struct Dx12GlyphUploadPlan;
enum class Dx12GlyphWorkKind { planner_chunk, barrier, owned_resources };
struct Dx12GlyphWorkEvent {
  Dx12GlyphWorkKind kind{};
  const void* source_data = nullptr;
  const void* planned_data = nullptr;
  const void* handed_off_data = nullptr;
  ID3D12Resource* resource = nullptr;
  D3D12_RESOURCE_STATES before{}, after{};
  std::size_t payload = 0, capacity = 0, source_index = 0;
  std::size_t texture_count = 0, live_slots = 0, dependency_refs = 0;
  std::size_t catalog_count = 0, pool_pages = 0, pool_capacity = 0, pool_used = 0;
};
// Private optional diagnostic sink: fixed storage, no callbacks or ownership.
// Resource peak is ONLY the enumerated glyph-owner domain, never whole-renderer VRAM.
class Dx12GlyphWorkObserver {
 public:
  Dx12GlyphWorkObserver(std::span<Dx12GlyphWorkEvent>, std::span<ID3D12Resource*>) noexcept;
  void record(Dx12GlyphWorkEvent) noexcept;
  void begin_texture_sample() noexcept;
  void include_texture(ID3D12Resource*) noexcept;
  std::size_t texture_count() const noexcept;
  bool overflowed() const noexcept;
  std::span<const Dx12GlyphWorkEvent> events() const noexcept;
  void clear() noexcept;
 private:
  std::span<Dx12GlyphWorkEvent> events_;
  std::span<ID3D12Resource*> textures_;
  std::size_t used_ = 0, texture_count_ = 0;
  bool overflowed_ = false;
};
void dx12_observe_glyph_plan(Dx12GlyphWorkObserver*, const Dx12GlyphUploadPlan&) noexcept;
void dx12_observe_glyph_lifetime(Dx12State&) noexcept;
} // namespace cgpui
