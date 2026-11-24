#include "dx12_dred_native_status_internal.hpp"
#include "dx12_dred_api_internal.hpp"
#include "dx12_dred_breadcrumb_native_internal.hpp"
#include "dx12_dred_site_internal.hpp"
#include "dx12_dred_status_sink_internal.hpp"
#include "dx12_state_internal.hpp"

namespace cgpui {
Dx12DredInterfaceResult dx12_query_dred_interface(IUnknown& source) noexcept {
  Dx12Ptr<ID3D12DeviceRemovedExtendedData> dred;
  const HRESULT result = source.QueryInterface(IID_PPV_ARGS(&dred));
  return {static_cast<std::int32_t>(result), dred.Get() != nullptr};
  // Local COM ownership ends before the caller receives these scalar values.
}
namespace {
class NativeStatus final : public Dx12DredStatusApi {
 public:
  explicit NativeStatus(ID3D12Device& device) noexcept : device_(device) {}
  std::int32_t removed_reason() noexcept override {
    return static_cast<std::int32_t>(device_.GetDeviceRemovedReason());
  }
  Dx12DredInterfaceResult query_interface() noexcept override {
    return dx12_query_dred_breadcrumbs(device_, breadcrumbs_);
  }
  Dx12DredBreadcrumbSummary breadcrumb_summary() const noexcept override {
    return breadcrumbs_;
  }
 private:
  Dx12DredBreadcrumbSummary breadcrumbs_;
  ID3D12Device& device_; // Borrowed only for the synchronous observation call.
};
}
void dx12_observe_dred_status(Dx12State& state) noexcept {
  if (!state.device) return;
  NativeStatus api(*state.device.Get());
  if (!state.dred_status.observe(api)) return;
  const auto observed = state.dred_status.snapshot_locked();
  if (observed.last_loss &&
      !std::holds_alternative<Dx12DredNotRequested>(observed.last_loss->setup))
    dx12_emit_dred_status(*observed.last_loss);
}
} // namespace cgpui
