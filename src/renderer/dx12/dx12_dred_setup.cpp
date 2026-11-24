#include "dx12_dred_api_internal.hpp"
#include "dx12_native_internal.hpp"

namespace cgpui {
Dx12DredSetupObservation dx12_request_dred_setup(bool requested,
                                               Dx12DredSetupApi& api) noexcept {
  if (!requested) return Dx12DredNotRequested{};
  const auto query = api.query_settings();
  if (query.hresult < 0 || !query.received) return Dx12DredSetupUnavailable{query};
  api.enable_breadcrumbs();
  return Dx12DredSetupRequestReturned{query};
}
namespace {
class NativeSetup final : public Dx12DredSetupApi {
 public:
  Dx12DredInterfaceResult query_settings() noexcept override {
    const HRESULT result = D3D12GetDebugInterface(IID_PPV_ARGS(&settings_));
    return {static_cast<std::int32_t>(result), settings_.Get() != nullptr};
  }
  void enable_breadcrumbs() noexcept override {
    settings_->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
  }
 private:
  Dx12Ptr<ID3D12DeviceRemovedExtendedDataSettings> settings_;
};
}
Dx12DredSetupObservation dx12_dred_setup(bool debug_enabled) noexcept {
  if (!debug_enabled) return Dx12DredNotRequested{};
  // Serialized within this implementation copy, not a cross-copy device guarantee.
  static const Dx12DredSetupObservation configured = []() noexcept {
    NativeSetup api;
    return dx12_request_dred_setup(true, api);
  }();
  return configured;
}
} // namespace cgpui
