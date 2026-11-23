#pragma once

#include "../../src/renderer/dx12/dx12_buffers_internal.hpp"
#include "../../src/renderer/dx12/dx12_state_internal.hpp"
#include <atomic>
#include <new>

namespace cgpui_test {
class Dx12ResourceReleaseSentinel final : public IUnknown {
 public:
  explicit Dx12ResourceReleaseSentinel(std::atomic_bool& released) : released_(released) {}
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** object) override {
    if (!object) return E_POINTER;
    *object = nullptr;
    if (iid != __uuidof(IUnknown)) return E_NOINTERFACE;
    *object = static_cast<IUnknown*>(this);
    AddRef();
    return S_OK;
  }
  ULONG STDMETHODCALLTYPE AddRef() override {
    return references_.fetch_add(1, std::memory_order_relaxed) + 1;
  }
  ULONG STDMETHODCALLTYPE Release() override {
    const ULONG remaining = references_.fetch_sub(1, std::memory_order_acq_rel) - 1;
    if (!remaining) {
      released_.store(true, std::memory_order_release);
      delete this;
    }
    return remaining;
  }
 private:
  ~Dx12ResourceReleaseSentinel() = default;
  std::atomic<ULONG> references_{1};
  std::atomic_bool& released_;
};

// The observer must outlive state destruction (including its destroyer thread).
// On success, ONLY state.uploads owns the resource; the resource's private-data
// strong reference is the sentinel's sole owner. No probe keeps the GPU owner
// alive artificially. Its final release is therefore externally observable.
inline cgpui::Result<void> dx12_attach_resource_sentinel(
    cgpui::Dx12State& state, std::atomic_bool& released) {
  released.store(false, std::memory_order_relaxed);
  if (!state.device) return std::unexpected(cgpui::Error{
      .code = cgpui::ErrorCode::invalid_argument,
      .message = "resource sentinel requires a live native device"});
  auto buffer = cgpui::dx12_create_buffer(*state.device.Get(), 16, D3D12_HEAP_TYPE_UPLOAD);
  if (!buffer) return std::unexpected(buffer.error());
  cgpui::Dx12Ptr<IUnknown> sentinel;
  sentinel.Attach(new (std::nothrow) Dx12ResourceReleaseSentinel(released));
  if (!sentinel) return std::unexpected(cgpui::dx12_error(E_OUTOFMEMORY, "resource sentinel allocation"));
  constexpr GUID key{0xb02470dd, 0x2a61, 0x46b1, {0xa8, 0x90, 0x17, 0x4b, 0x41, 0x96, 0x7e, 0x1d}};
  const HRESULT attached = (*buffer)->SetPrivateDataInterface(key, sentinel.Get());
  if (FAILED(attached)) return std::unexpected(cgpui::dx12_error(attached, "resource sentinel attachment"));
  state.uploads.push_back(std::move(*buffer));
  sentinel.Reset(); // The resource now holds the sole sentinel reference.
  return {};
}
} // namespace cgpui_test
