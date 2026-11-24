#pragma once
#include "../../src/renderer/dx12/dx12_command_reuse_lifetime_internal.hpp"
#include "../../src/renderer/dx12/dx12_command_reuse_surface_epoch_internal.hpp"

namespace cgpui {
// CPU model adapter ONLY: these calls are not native completion/reset evidence.
class Dx12CommandReuseTestAccess {
public:
  static bool begin(Dx12CommandReuseLifetime&, std::uint64_t) noexcept;
  static bool create(Dx12CommandReuseLifetime&, std::uint64_t) noexcept;
  static bool attach(Dx12CommandReuseLifetime&, std::uint64_t, std::uint64_t, std::uint64_t) noexcept;
  static bool reserve(Dx12CommandReuseLifetime&, std::uint64_t, std::uint64_t, std::uint64_t) noexcept;
  static bool complete(Dx12CommandReuseLifetime&, std::uint64_t, std::uint64_t, std::uint64_t) noexcept;
  static bool remove(Dx12CommandReuseLifetime&, std::uint64_t) noexcept;
  static bool reset(Dx12CommandReuseLifetime&, std::uint64_t, std::uint64_t) noexcept;
  static bool release(Dx12CommandReuseLifetime&, std::uint64_t, std::uint64_t) noexcept;
  static bool publish(Dx12CommandReuseLifetime&, std::uint64_t, std::uint64_t,
                      std::uint64_t, std::uint64_t) noexcept;
  static void invalidate(Dx12CommandReuseLifetime&) noexcept;
  static bool forget(Dx12CommandReuseLifetime&, std::uint64_t, std::uint64_t) noexcept;
  static Dx12CommandReuseSurfaceChange accept(
      Dx12CommandReuseSurfaceEpoch&, const Dx12CommandReuseSurfaceKey&) noexcept;
  static void exhaust_entry_ids(Dx12CommandReuseLifetime&) noexcept;
  static void last_surface_epoch(Dx12CommandReuseSurfaceEpoch&) noexcept;
};
} // namespace cgpui

namespace cgpui_test {
void require_reuse(bool value, const char* reason);
}
