#pragma once
#include "dx12_composition_probe_windows_test_support.hpp"
#include "../../src/renderer/dx12/dx12_frame_pacing_internal.hpp"
#include "../../src/renderer/dx12/dx12_state_internal.hpp"

namespace cgpui_test {
enum class CompositionProbeExit : int { observed = 0, unmeasured = 2, pending = 3, invalid = 4 };
void composition_probe_note(const char* stage, const char* detail);
bool composition_probe_hresult(const char* stage, HRESULT result);
struct Dx12CompositionProbeApi final : cgpui::Dx12FramePacingApi {
  cgpui::Dx12NativeFramePacingApi native;
  unsigned sets = 0, gets = 0, acquired = 0, polls = 0, ready = 0, pending = 0, closes = 0;
  bool close_ok = true;
  HANDLE observed_handle = nullptr; // Non-owning: never waited/closed independently.
  HRESULT set_maximum_latency(IDXGISwapChain2*, UINT) noexcept override;
  HANDLE latency_object(IDXGISwapChain2*) noexcept override;
  cgpui::Dx12FramePacingWaitResult poll(HANDLE) noexcept override;
  bool close(HANDLE) noexcept override;
};
class Dx12CompositionProbe {
 public:
  Dx12CompositionProbe();
  ~Dx12CompositionProbe();
  bool initialize(bool visible);
  bool await_admission(bool before_commit);
  bool present_pixels(bool blue);
  bool resize_same_chain(UINT extent);
  bool debug_clean();
  bool close_and_verify();
  bool invariant(bool value, const char* detail);
  bool result(const char* stage, const cgpui::Result<void>& value);
  bool native(const char* stage, HRESULT value);
  CompositionProbeExit outcome = CompositionProbeExit::unmeasured;
  Dx12CompositionProbeWindows windows;
  Dx12CompositionProbeApi api;
  std::shared_ptr<cgpui::Dx12State> state;
 private:
  bool create_chain();
  bool bind_composition();
  bool record_pixels(bool blue);
  bool verify_pixels(bool blue);
  bool visible_ = false;
  UINT creation_flags_ = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
};
int run_composition_probe(bool visible);
} // namespace cgpui_test
