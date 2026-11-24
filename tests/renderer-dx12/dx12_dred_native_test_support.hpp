#pragma once
#include "dx12_recovery_test_support.hpp"
#include "../../src/renderer/dx12/dx12_dred_site_internal.hpp"
#include <memory>

namespace cgpui_test {
struct DredNativeLoss {
  bool valid = false, debug = false;
  HRESULT reason = S_OK;
  cgpui::Dx12DredInterfaceResult query{};
  HRESULT breadcrumb_hresult = S_OK;
  bool breadcrumb_requested = false;
};
struct DredNativeReadout {
  unsigned calls = 0;
  cgpui::Dx12DredSiteSnapshot snapshot;
  std::uint64_t generation = 0;
  bool device_owned = false, submitted = false;
  HRESULT actual_reason = S_OK;
};
// Readout outlives state/frames; attachment owns no extra state or native refs.
bool dred_attach_readout(cgpui::Dx12State&, DredNativeReadout&);
std::weak_ptr<cgpui::Dx12State> dred_attach_public_readout(
    const cgpui::RendererBackendInfo&, HWND, DredNativeReadout&);
void dred_capture_requests();
bool dred_select_debug(bool enabled);
bool dred_record_identity(const cgpui::RendererBackendInfo&, const char* stage);
bool dred_flush_output();
void dred_observe_breadcrumb_reference(ID3D12Device&, DredNativeLoss&);
DredNativeLoss dred_remove_actual(ID3D12Device&, bool debug);
DredNativeLoss dred_remove_actual(const cgpui::RendererBackendInfo&, bool debug);
bool dred_status_matches(const cgpui::Dx12DredStatus&, const DredNativeLoss&);
bool dred_current_readout(const DredNativeReadout&, const DredNativeLoss&,
                         std::uint64_t generation);
int dred_native_cycles(bool debug);
} // namespace cgpui_test
