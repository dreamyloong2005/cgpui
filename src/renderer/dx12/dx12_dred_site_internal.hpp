#pragma once
#include "dx12_dred_status_internal.hpp"
#include <optional>

namespace cgpui {
class Dx12DredStatusApi;
struct Dx12State;
enum class Dx12DredSitePhase { closed, open, checking, collecting, complete };
struct Dx12DredSiteSnapshot {
  Dx12DredSitePhase phase = Dx12DredSitePhase::closed;
  std::optional<Dx12DredStatus> last_loss;
  bool last_is_current = false;
};
// Inline CPU values only. All methods require the caller's existing state gate.
// Lifecycle open/close do not overlap observe; native teardown is not reentrant.
class Dx12DredSite {
 public:
  void configure_setup(Dx12DredSetupObservation setup) noexcept;
  void open_local_lease() noexcept;
  void close_local_lease() noexcept;
  bool observe(Dx12DredStatusApi& api) noexcept;
  [[nodiscard]] Dx12DredSiteSnapshot snapshot_locked() const noexcept;
 private:
  Dx12DredSitePhase phase_ = Dx12DredSitePhase::closed;
  Dx12DredSetupObservation setup_;
  std::optional<Dx12DredStatus> last_loss_;
};
void dx12_observe_dred_status(Dx12State& state) noexcept;
} // namespace cgpui
