#pragma once
#include "dx12_native_internal.hpp"
#include <cstddef>
#include <span>

namespace cgpui {
enum class Dx12TextureCopyKind { allocation, full_upload, region_upload, clone };
struct Dx12TextureCopyEvent {
  Dx12TextureCopyKind kind{};
  ID3D12Resource* source = nullptr; // Non-owning observations; never dereference after retirement.
  ID3D12Resource* target = nullptr;
  std::size_t payload_bytes = 0;
  UINT x = 0, y = 0, width = 0, height = 0, row_pitch = 0;
  UINT64 offset = 0;
};
class Dx12TextureCopyObserver {
 public:
  explicit Dx12TextureCopyObserver(std::span<Dx12TextureCopyEvent>) noexcept;
  bool capacity_for(std::size_t count) const noexcept;
  void record(Dx12TextureCopyEvent) noexcept;
  std::span<const Dx12TextureCopyEvent> events() const noexcept;
  bool overflowed() const noexcept;
  void clear() noexcept;
 private:
  std::span<Dx12TextureCopyEvent> storage_;
  std::size_t used_ = 0;
  bool overflowed_ = false;
};
} // namespace cgpui
