#pragma once
#include "dx12_foreign_probe_native_test_support.hpp"
#include <atomic>
#include <mutex>
#include <thread>

namespace cgpui_test {
class ForeignProbePeer {
 public:
  ForeignProbeHandle ready, go, entered, release, pulse_seen, done;
  std::atomic<DWORD> owner{0};
  std::atomic<int> result{70};
  ForeignProbePeer();
  ~ForeignProbePeer();
  bool start(ForeignProbeMode mode, HANDLE cancel);
  std::shared_ptr<cgpui::Dx12State> take_state();
  Dx12PacingForwardApi* pacing = nullptr; // Borrowed until the coordinator releases this owner.
  ForeignProbeSnapshot before;
  int finish(bool recover);
 private:
  int run(ForeignProbeMode mode, HANDLE cancel);
  std::atomic<bool> recover_{false};
  std::mutex publication_;
  std::weak_ptr<cgpui::Dx12State> published_;
  std::thread worker_;
};
} // namespace cgpui_test
