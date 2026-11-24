#pragma once

#include "dx12_frame_pacing_native_internal.hpp"
#include "dx12_initialization_failure_internal.hpp"

namespace cgpui {
enum class Dx12FrameAdmission { disabled, pending, ready };

// One swapchain's app-owned handle and unconsumed admission observation.
// Callers serialize all operations; polling never leaves a pending native wait.
// No device/queue/swapchain COM ownership, message pump, or GPU retirement proof.
class Dx12FramePacing {
 public:
  explicit Dx12FramePacing(Dx12FramePacingApi& api = dx12_native_frame_pacing_api()) noexcept;
  ~Dx12FramePacing();
  Dx12FramePacing(const Dx12FramePacing&) = delete;
  Dx12FramePacing& operator=(const Dx12FramePacing&) = delete;

  // Configure once; even a disabled route needs reset before reconfiguration.
  // A failed configuration adopts no handle and leaves this owner unconfigured.
  Result<void> configure(IDXGISwapChain2* chain, bool enabled, Dx12InitializationFailureClass* failure = nullptr);
  Result<Dx12FrameAdmission> try_admit();
  // Only after an actual Present attempt, not CPU-frame abandonment or timeout.
  void presentation_attempted() noexcept;
  // Caller first proves completion/removal/never-submitted before native teardown.
  void reset() noexcept;
  // Preserve the same outliving private native/test API across chain replacement.
  Dx12FramePacingApi& api() const noexcept;

 private:
  Dx12FramePacingApi& api_;
  HANDLE handle_ = nullptr;
  bool configured_ = false;
  bool enabled_ = false;
  bool granted_ = false;
};
} // namespace cgpui
