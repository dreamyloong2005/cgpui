#pragma once
#include "dx12_native_internal.hpp"

namespace cgpui {
struct Dx12State;
class Dx12GlyphRecordingScope {
 public:
  explicit Dx12GlyphRecordingScope(Dx12State&) noexcept;
  ~Dx12GlyphRecordingScope();
  Dx12GlyphRecordingScope(const Dx12GlyphRecordingScope&) = delete;
  Dx12GlyphRecordingScope& operator=(const Dx12GlyphRecordingScope&) = delete;
  bool entered() const noexcept;
  // Requires successful scope entry and prior completion/never-executed proof.
  Result<void> reset_and_begin();
  void opened_after_reset() noexcept;
  HRESULT close() noexcept;
  void executed() noexcept;
 private:
  Dx12State& state_;
  bool entered_ = false, open_ = false, attempted_close_ = false, executed_ = false;
  HRESULT cleanup_close_ = S_OK;
};
} // namespace cgpui
