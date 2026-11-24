#include "dx12_command_reuse_test_support.hpp"
#include <limits>
#include <stdexcept>

namespace cgpui {
bool Dx12CommandReuseTestAccess::begin(Dx12CommandReuseLifetime& x, std::uint64_t g) noexcept {
  return x.begin_generation(g);
}
bool Dx12CommandReuseTestAccess::create(Dx12CommandReuseLifetime& x, std::uint64_t g) noexcept {
  return x.create_entry(g);
}
bool Dx12CommandReuseTestAccess::attach(Dx12CommandReuseLifetime& x,
    std::uint64_t g, std::uint64_t e, std::uint64_t p) noexcept { return x.attach_parent(g, e, p); }
bool Dx12CommandReuseTestAccess::reserve(Dx12CommandReuseLifetime& x,
    std::uint64_t g, std::uint64_t p, std::uint64_t s) noexcept { return x.reserve_submission(g, p, s); }
bool Dx12CommandReuseTestAccess::complete(Dx12CommandReuseLifetime& x,
    std::uint64_t g, std::uint64_t p, std::uint64_t s) noexcept { return x.completed(g, p, s); }
bool Dx12CommandReuseTestAccess::remove(Dx12CommandReuseLifetime& x, std::uint64_t g) noexcept {
  return x.removed(g);
}
bool Dx12CommandReuseTestAccess::reset(Dx12CommandReuseLifetime& x,
    std::uint64_t g, std::uint64_t p) noexcept { return x.parent_reset_succeeded(g, p); }
bool Dx12CommandReuseTestAccess::release(Dx12CommandReuseLifetime& x,
    std::uint64_t g, std::uint64_t p) noexcept { return x.parent_released(g, p); }
bool Dx12CommandReuseTestAccess::publish(Dx12CommandReuseLifetime& x, std::uint64_t g,
    std::uint64_t e, std::uint64_t p, std::uint64_t s) noexcept { return x.publish_success(g, e, p, s); }
void Dx12CommandReuseTestAccess::invalidate(Dx12CommandReuseLifetime& x) noexcept { x.invalidate(); }
bool Dx12CommandReuseTestAccess::forget(Dx12CommandReuseLifetime& x,
    std::uint64_t g, std::uint64_t e) noexcept { return x.forget_released_entry(g, e); }
Dx12CommandReuseSurfaceChange Dx12CommandReuseTestAccess::accept(
    Dx12CommandReuseSurfaceEpoch& x, const Dx12CommandReuseSurfaceKey& key) noexcept { return x.accept(key); }
void Dx12CommandReuseTestAccess::exhaust_entry_ids(Dx12CommandReuseLifetime& x) noexcept {
  x.last_entry_ = std::numeric_limits<std::uint64_t>::max();
}
void Dx12CommandReuseTestAccess::last_surface_epoch(Dx12CommandReuseSurfaceEpoch& x) noexcept {
  x.epoch_ = std::numeric_limits<std::uint64_t>::max();
}
} // namespace cgpui

namespace cgpui_test {
void require_reuse(bool value, const char* reason) { if (!value) throw std::runtime_error(reason); }
} // namespace cgpui_test
