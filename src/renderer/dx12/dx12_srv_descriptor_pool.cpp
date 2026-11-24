#include "dx12_srv_descriptor_pool_internal.hpp"
#include "dx12_srv_descriptor_page_internal.hpp"
#include "dx12_srv_heap_api_internal.hpp"
#include <cassert>
#include <limits>
#include <new>
#include <stdexcept>
#include <utility>

namespace cgpui {
Dx12SrvAllocation::Dx12SrvAllocation() noexcept = default;
Dx12SrvAllocation::~Dx12SrvAllocation() { reset(); }
Dx12SrvAllocation::Dx12SrvAllocation(
    std::shared_ptr<Dx12SrvDescriptorPage> page, unsigned count) noexcept
    : page_(std::move(page)), count_(count) { page_->acquire(slots_, count_); }
Dx12SrvAllocation::Dx12SrvAllocation(Dx12SrvAllocation&& other) noexcept
    : page_(std::move(other.page_)), slots_(other.slots_), count_(std::exchange(other.count_, 0)) {}
Dx12SrvAllocation& Dx12SrvAllocation::operator=(Dx12SrvAllocation&& other) noexcept {
  if (this != &other) {
    reset();
    page_ = std::move(other.page_);
    slots_ = other.slots_;
    count_ = std::exchange(other.count_, 0);
  }
  return *this;
}
void Dx12SrvAllocation::reset() noexcept {
  if (page_) page_->release(slots_, count_);
  count_ = 0;
  page_.reset();
}
ID3D12DescriptorHeap* Dx12SrvAllocation::heap() const noexcept { return page_ ? page_->heap() : nullptr; }
D3D12_CPU_DESCRIPTOR_HANDLE Dx12SrvAllocation::cpu(unsigned index) const noexcept {
  return index < count_ ? page_->cpu(slots_[index]) : D3D12_CPU_DESCRIPTOR_HANDLE{};
}
D3D12_GPU_DESCRIPTOR_HANDLE Dx12SrvAllocation::gpu(unsigned index) const noexcept {
  return index < count_ ? page_->gpu(slots_[index]) : D3D12_GPU_DESCRIPTOR_HANDLE{};
}
unsigned Dx12SrvAllocation::size() const noexcept { return count_; }
Dx12SrvDescriptorPool::Dx12SrvDescriptorPool(UINT page_capacity, Dx12SrvHeapApi* api)
    : page_capacity_(page_capacity), api_(api ? api : &dx12_native_srv_heap_api()) {}
Dx12SrvDescriptorPool::~Dx12SrvDescriptorPool() = default;
Result<Dx12SrvAllocation> Dx12SrvDescriptorPool::allocate(ID3D12Device& device, unsigned count) {
  if ((count != 1 && count != 3) || page_capacity_ < 3 ||
      page_capacity_ > D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1 ||
      (epoch_device_ && epoch_device_ != &device))
    return std::unexpected(dx12_error(E_INVALIDARG, "SRV pool count, capacity or native epoch"));
  for (const auto& page : pages_)
    if (page->available() >= count) return Dx12SrvAllocation(page, count);
  // Capacity sum is kept representable, so stats cannot silently wrap.
  if (pages_.size() >= std::numeric_limits<std::size_t>::max() / page_capacity_)
    return std::unexpected(dx12_error(E_OUTOFMEMORY, "SRV pool capacity overflow"));
  try {
    auto page = Dx12SrvDescriptorPage::create(device, page_capacity_, *api_, successful_page_creations_);
    if (!page) return std::unexpected(page.error());
    pages_.push_back(*page); // May fail; no existing slot/lease has changed.
    epoch_device_ = &device;
    return Dx12SrvAllocation(std::move(*page), count); // Entire group, one page.
  } catch (const std::bad_alloc&) {
    return std::unexpected(dx12_error(E_OUTOFMEMORY, "SRV page catalog allocation"));
  } catch (const std::length_error&) {
    return std::unexpected(dx12_error(E_OUTOFMEMORY, "SRV page catalog capacity"));
  }
}
void Dx12SrvDescriptorPool::clear_after_retirement() noexcept {
  // Detect owner-order violations; never turn unknown completion into reuse.
  for (const auto& page : pages_) {
    assert(page->available() == page->capacity());
    if (page->available() != page->capacity()) return;
  }
  pages_.clear();
  epoch_device_ = nullptr;
}
Dx12SrvPoolStats Dx12SrvDescriptorPool::stats() const noexcept {
  Dx12SrvPoolStats value{.page_count = pages_.size(),
      .capacity = pages_.size() * page_capacity_,
      .successful_page_creations = successful_page_creations_};
  for (const auto& page : pages_) value.live_slots += page->capacity() - page->available();
  return value;
}
} // namespace cgpui
