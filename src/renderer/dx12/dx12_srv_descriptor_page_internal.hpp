#pragma once

#include "dx12_native_internal.hpp"
#include <array>
#include <cstdint>
#include <memory>
#include <vector>

namespace cgpui {
class Dx12SrvHeapApi;
class Dx12SrvAllocation;
class Dx12SrvDescriptorPool;

// Stable, privately owned heap plus an upfront-sized free-slot stack.
class Dx12SrvDescriptorPage {
 public:
  static Result<std::shared_ptr<Dx12SrvDescriptorPage>> create(
      ID3D12Device& device, UINT capacity, Dx12SrvHeapApi& api,
      std::uint64_t& successful_creations);
  ID3D12DescriptorHeap* heap() const noexcept;
  UINT capacity() const noexcept;
  UINT available() const noexcept;
  D3D12_CPU_DESCRIPTOR_HANDLE cpu(UINT slot) const noexcept;
  D3D12_GPU_DESCRIPTOR_HANDLE gpu(UINT slot) const noexcept;
 private:
  friend class Dx12SrvAllocation;
  explicit Dx12SrvDescriptorPage(UINT capacity);
  void acquire(std::array<UINT, 3>& slots, unsigned count) noexcept;
  void release(const std::array<UINT, 3>& slots, unsigned count) noexcept;
  Dx12Ptr<ID3D12DescriptorHeap> heap_;
  D3D12_CPU_DESCRIPTOR_HANDLE cpu_{};
  D3D12_GPU_DESCRIPTOR_HANDLE gpu_{};
  UINT stride_ = 0;
  std::vector<UINT> free_;
  UINT available_ = 0;
};
} // namespace cgpui
