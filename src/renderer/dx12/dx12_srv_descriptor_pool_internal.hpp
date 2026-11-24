#pragma once

#include "dx12_native_internal.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace cgpui {
class Dx12SrvDescriptorPage;
class Dx12SrvHeapApi;
class Dx12SrvDescriptorPool;

// Native owners only. Returning a lease requires idle/removal proof, or an
// unsubmitted/abandoned command list. Shared page ownership is NOT fence proof.
class Dx12SrvAllocation {
 public:
  Dx12SrvAllocation() noexcept;
  ~Dx12SrvAllocation();
  Dx12SrvAllocation(Dx12SrvAllocation&& other) noexcept;
  Dx12SrvAllocation& operator=(Dx12SrvAllocation&& other) noexcept;
  Dx12SrvAllocation(const Dx12SrvAllocation&) = delete;
  Dx12SrvAllocation& operator=(const Dx12SrvAllocation&) = delete;
  ID3D12DescriptorHeap* heap() const noexcept;
  D3D12_CPU_DESCRIPTOR_HANDLE cpu(unsigned index = 0) const noexcept;
  D3D12_GPU_DESCRIPTOR_HANDLE gpu(unsigned index = 0) const noexcept;
  unsigned size() const noexcept;
 private:
  friend class Dx12SrvDescriptorPool;
  explicit Dx12SrvAllocation(std::shared_ptr<Dx12SrvDescriptorPage> page, unsigned count) noexcept;
  void reset() noexcept;
  std::shared_ptr<Dx12SrvDescriptorPage> page_;
  std::array<UINT, 3> slots_{};
  unsigned count_ = 0;
};

struct Dx12SrvPoolStats {
  std::size_t page_count = 0;
  std::size_t capacity = 0;
  std::size_t live_slots = 0;
  // Saturating successful native creations over this pool's whole lifetime,
  // including a successfully created page whose later publication failed.
  std::uint64_t successful_page_creations = 0;
};

class Dx12SrvDescriptorPool {
 public:
  // Private fixture configuration only; 4096 is a page size, not a total limit.
  // An injected API must outlive the pool. State's operation lock serializes use.
  explicit Dx12SrvDescriptorPool(UINT page_capacity = 4096, Dx12SrvHeapApi* api = nullptr);
  ~Dx12SrvDescriptorPool();
  Dx12SrvDescriptorPool(const Dx12SrvDescriptorPool&) = delete;
  Dx12SrvDescriptorPool& operator=(const Dx12SrvDescriptorPool&) = delete;
  Result<Dx12SrvAllocation> allocate(ID3D12Device& device, unsigned count);
  // Requires ALL native consumers released after completion/removal proof.
  // Not a frame rewind. Empty high-water pages remain until this epoch reset.
  void clear_after_retirement() noexcept;
  Dx12SrvPoolStats stats() const noexcept;
 private:
  UINT page_capacity_;
  Dx12SrvHeapApi* api_;
  ID3D12Device* epoch_device_ = nullptr; // Borrowed; pages retain the native device.
  std::vector<std::shared_ptr<Dx12SrvDescriptorPage>> pages_;
  std::uint64_t successful_page_creations_ = 0;
};
} // namespace cgpui
