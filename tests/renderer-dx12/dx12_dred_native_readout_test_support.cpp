#include "dx12_dred_native_test_support.hpp"
#include "dx12_command_reuse_public_owner_test_support.hpp"
#include "../../src/renderer/dx12/dx12_operation_internal.hpp"

namespace cgpui_test {
namespace {
class ScalarRetirementReadout final : public cgpui::Dx12CommandVerifier {
 public:
  ScalarRetirementReadout(cgpui::Dx12State& state, DredNativeReadout& out)
      : state_(state), out_(out) {}
  void prepare(cgpui::Dx12State&, ID3D12Resource&, std::uint64_t) noexcept override {}
  ID3D12CommandList* prepared_list() const noexcept override { return nullptr; }
  void completed(cgpui::Dx12State&, std::uint64_t, std::uint64_t) noexcept override {}
  void retired(bool) noexcept override {
    // Existing callback runs inside common cleanup, after P but before device Reset.
    ++out_.calls;
    out_.snapshot = state_.dred_status.snapshot_locked();
    out_.generation = state_.info.device_generation;
    out_.device_owned = state_.device.Get() != nullptr;
    out_.submitted = state_.submitted;
    out_.actual_reason = state_.device ? state_.device->GetDeviceRemovedReason() : S_OK;
  }
 private:
  cgpui::Dx12State& state_; // Valid during its owning state's destructor body too.
  DredNativeReadout& out_;  // Fixed owned scalars only escape into the external readout.
};
}
bool dred_attach_readout(cgpui::Dx12State& state, DredNativeReadout& out) {
  if (state.command_observation) return false;
  state.command_observation = std::make_unique<cgpui::Dx12CommandObservation>(512);
  state.command_observation->verifier = std::make_unique<ScalarRetirementReadout>(state, out);
  return true;
}
std::weak_ptr<cgpui::Dx12State> dred_attach_public_readout(
    const cgpui::RendererBackendInfo& info, HWND hwnd, DredNativeReadout& out) {
  // Reuse the existing exact HWND/generation weak registry lookup, not a new API.
  const auto weak = reuse_public_state(info, hwnd);
  const auto state = weak.lock();
  if (!state) return {};
  cgpui::Dx12OperationGuard gate(*state);
  return dred_attach_readout(*state, out) ? weak : std::weak_ptr<cgpui::Dx12State>{};
}
bool dred_current_readout(const DredNativeReadout& out, const DredNativeLoss& loss,
                         std::uint64_t generation) {
  std::cout << "DRED retirement calls=" << out.calls << " current=" << out.snapshot.last_is_current
            << " generation=" << out.generation << " has_scalar=" << bool(out.snapshot.last_loss);
  if (out.snapshot.last_loss) std::cout << " setup_kind=" << out.snapshot.last_loss->setup.index();
  std::cout << '\n';
  return out.device_owned && !out.submitted && out.actual_reason == loss.reason &&
      out.generation == generation && out.snapshot.phase == cgpui::Dx12DredSitePhase::complete &&
      out.snapshot.last_is_current && out.snapshot.last_loss &&
      dred_status_matches(*out.snapshot.last_loss, loss);
}
} // namespace cgpui_test
