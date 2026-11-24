#pragma once

#include <cstdint>
#include <memory>

namespace cgpui {

// Private platform/renderer seam. No native headers or owning window reference.
enum class NativeWindowWorkResult { complete, deferred };
class NativeWindowWork {
 public:
  virtual ~NativeWindowWork();
  virtual NativeWindowWorkResult run() noexcept = 0;
};
struct NativeWindowWorkToken {
  std::uintptr_t instance = 0;
  std::uintptr_t work = 0;
  explicit constexpr operator bool() const noexcept { return instance && work; }
};
// Enqueue-only transport, called under the metadata mutex: must not dispatch or
// reenter this module. Win32 satisfies this with PostMessageW, never SendMessage.
using NativeWindowWorkPost = bool (*)(void*, NativeWindowWorkToken) noexcept;
enum class NativeWindowWorkDispatch { ignored, complete, deferred };

// Platform opens/closes managed windows on their owner thread. Each attachment is unique.
std::uintptr_t native_window_work_open(void* window, NativeWindowWorkPost) noexcept;
void native_window_work_close(void* window, std::uintptr_t instance) noexcept;
// Any thread may register weak metadata; unmanaged windows fail closed.
NativeWindowWorkToken native_window_work_register(
    void* window, std::weak_ptr<NativeWindowWork>) noexcept;
// Any thread may post/cancel. Cancellation does not wait for an executing callback.
// true means queued/coalesced, never that the work completed. false means no wake accepted.
bool native_window_work_post(NativeWindowWorkToken) noexcept;
void native_window_work_cancel(NativeWindowWorkToken) noexcept;
// Owner-thread only. deferred alone never reposts; the caller must explicitly retry.
NativeWindowWorkDispatch native_window_work_dispatch(
    void* window, NativeWindowWorkToken) noexcept;

} // namespace cgpui
