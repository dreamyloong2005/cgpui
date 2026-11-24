#include "dx12_srv_descriptor_page_internal.hpp"
#include "dx12_srv_heap_api_internal.hpp"
#include <cassert>
#include <limits>
#include <new>
#include <stdexcept>

namespace cgpui {
Dx12SrvDescriptorPage::Dx12SrvDescriptorPage(UINT capacity)
    : free_(capacity), available_(capacity) {
  for (UINT index = 0; index != capacity; ++index) free_[index] = capacity - index - 1;
}
Result<std::shared_ptr<Dx12SrvDescriptorPage>> Dx12SrvDescriptorPage::create(
    ID3D12Device& device, UINT capacity, Dx12SrvHeapApi& api,
    std::uint64_t& successful_creations) {
  if (capacity < 3 || capacity > D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1)
    return std::unexpected(dx12_error(E_INVALIDARG, "SRV page capacity"));
  try {
    // Allocate every byte of free-list metadata BEFORE exposing any slot.
    auto page = std::shared_ptr<Dx12SrvDescriptorPage>(new Dx12SrvDescriptorPage(capacity));
    D3D12_DESCRIPTOR_HEAP_DESC descriptor{};
    descriptor.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    descriptor.NumDescriptors = capacity;
    descriptor.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    const HRESULT result = api.create(device, descriptor, page->heap_.GetAddressOf());
    if (FAILED(result)) return std::unexpected(dx12_error(result, "SRV descriptor page allocation"));
    if (successful_creations != std::numeric_limits<std::uint64_t>::max()) ++successful_creations;
    if (!page->heap_) return std::unexpected(dx12_error(E_UNEXPECTED, "SRV page missing native heap"));
    page->stride_ = device.GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    page->cpu_ = page->heap_->GetCPUDescriptorHandleForHeapStart();
    page->gpu_ = page->heap_->GetGPUDescriptorHandleForHeapStart();
    const auto last = static_cast<UINT64>(capacity - 1);
    if (!page->stride_ || !page->cpu_.ptr || !page->gpu_.ptr ||
        last > (std::numeric_limits<SIZE_T>::max() - page->cpu_.ptr) / page->stride_ ||
        last > (std::numeric_limits<UINT64>::max() - page->gpu_.ptr) / page->stride_)
      return std::unexpected(dx12_error(E_INVALIDARG, "SRV descriptor handle arithmetic"));
    return page;
  } catch (const std::bad_alloc&) {
    return std::unexpected(dx12_error(E_OUTOFMEMORY, "SRV page metadata allocation"));
  } catch (const std::length_error&) {
    return std::unexpected(dx12_error(E_OUTOFMEMORY, "SRV page metadata capacity"));
  }
}
ID3D12DescriptorHeap* Dx12SrvDescriptorPage::heap() const noexcept { return heap_.Get(); }
UINT Dx12SrvDescriptorPage::capacity() const noexcept { return static_cast<UINT>(free_.size()); }
UINT Dx12SrvDescriptorPage::available() const noexcept { return available_; }
D3D12_CPU_DESCRIPTOR_HANDLE Dx12SrvDescriptorPage::cpu(UINT slot) const noexcept {
  assert(slot < capacity());
  return {cpu_.ptr + static_cast<SIZE_T>(slot) * stride_};
}
D3D12_GPU_DESCRIPTOR_HANDLE Dx12SrvDescriptorPage::gpu(UINT slot) const noexcept {
  assert(slot < capacity());
  return {gpu_.ptr + static_cast<UINT64>(slot) * stride_};
}
void Dx12SrvDescriptorPage::acquire(std::array<UINT, 3>& slots, unsigned count) noexcept {
  assert(count <= slots.size() && count <= available_);
  for (unsigned index = 0; index != count; ++index) slots[index] = free_[--available_];
}
void Dx12SrvDescriptorPage::release(const std::array<UINT, 3>& slots, unsigned count) noexcept {
  assert(count <= slots.size() && count <= capacity() - available_);
  for (unsigned index = 0; index != count; ++index) free_[available_++] = slots[index];
}
} // namespace cgpui
