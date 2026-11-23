#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "dx12_test_support.hpp"
#include "dx12_sync_resource_sentinel.hpp"
#include "../../src/renderer/dx12/dx12_geometry_upload_internal.hpp"
#include "../../src/renderer/dx12/dx12_geometry_upload_frame_internal.hpp"
#include <cstring>
#include <span>
#include <tuple>

namespace cgpui_test {
inline constexpr std::size_t geometry_page_bytes = 256 * 1024;
inline auto geometry_stats(const cgpui::Dx12GeometryUploads& arena) {
  const auto value = arena.stats();
  return std::make_tuple(value.page_count, value.capacity_bytes, value.used_bytes,
                        value.allocations_this_frame, value.total_allocations);
}
inline bool geometry_bytes(const cgpui::Dx12GeometryUploadSlice& slice,
                           std::span<const unsigned char> expected) {
  if (!slice.resource || slice.size != expected.size() || slice.offset % 16 ||
      slice.address != slice.resource->GetGPUVirtualAddress() + slice.offset) return false;
  const auto capacity = slice.resource->GetDesc().Width;
  if (slice.offset > capacity || slice.size > capacity - slice.offset) return false;
  void* mapped = nullptr;
  const D3D12_RANGE read{slice.offset, slice.offset + slice.size};
  if (FAILED(slice.resource->Map(0, &read, &mapped))) return false;
  const bool equal = std::memcmp(static_cast<unsigned char*>(mapped) + slice.offset,
                                 expected.data(), expected.size()) == 0;
  const D3D12_RANGE no_writes{0, 0};
  slice.resource->Unmap(0, &no_writes);
  return equal;
}
inline bool geometry_sentinel(ID3D12Resource& resource, std::atomic_bool& released) {
  released.store(false, std::memory_order_relaxed);
  cgpui::Dx12Ptr<IUnknown> sentinel;
  sentinel.Attach(new (std::nothrow) Dx12ResourceReleaseSentinel(released));
  if (!sentinel) return false;
  constexpr GUID key{0x6f632a12, 0x6916, 0x41c0, {0x94, 0xd6, 0x49, 0x27, 0x80, 0x3f, 0xc5, 0x26}};
  const bool attached = SUCCEEDED(resource.SetPrivateDataInterface(key, sentinel.Get()));
  sentinel.Reset(); // Only the arena's native resource now owns this observer.
  return attached && !released.load();
}
inline std::unique_ptr<cgpui::Dx12State> geometry_state() {
  auto state = std::make_unique<cgpui::Dx12State>();
  if (!cgpui::dx12_create_device(*state)) return {};
  return state;
}
} // namespace cgpui_test
