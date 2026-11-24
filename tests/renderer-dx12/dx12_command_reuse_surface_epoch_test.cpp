#include "dx12_command_reuse_test_support.hpp"
#include <initializer_list>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <type_traits>

using namespace cgpui;
using cgpui_test::require_reuse;
using Access = Dx12CommandReuseTestAccess;
using Change = Dx12CommandReuseSurfaceChange;
static_assert(!std::is_copy_constructible_v<Dx12CommandReuseSurfaceEpoch>);

int main() {
  try {
    Dx12CommandReuseSurfaceEpoch surface;
    Dx12CommandReuseSurfaceKey key{.width = 64, .height = 32, .scale = 1.0F};
    require_reuse(!surface.enabled() && !surface.value(), "unconfigured surface has no reusable identity");
    require_reuse(Access::accept(surface, key) == Change::changed && surface.value() == 1, "first accepted surface");
    require_reuse(Access::accept(surface, key) == Change::unchanged && surface.value() == 1, "identical no-op");
    for (const auto scale : {0.0F, -1.0F, std::numeric_limits<float>::infinity(),
                             std::numeric_limits<float>::quiet_NaN()}) {
      auto bad = key; bad.scale = scale;
      require_reuse(Access::accept(surface, bad) == Change::invalid && surface.value() == 1 &&
          surface.key().scale == 1.0F && surface.enabled(), "invalid scale must not mutate accepted identity");
    }
    auto bad = key; bad.width = 0;
    require_reuse(Access::accept(surface, bad) == Change::invalid && surface.value() == 1, "invalid size no-op");
    Dx12CommandReuseLifetime life;
    require_reuse(Access::begin(life, 1) && Access::create(life, 1), "scale retention setup");
    const auto entry = life.entry_epoch();
    require_reuse(Access::attach(life, 1, entry, 1), "old same-size parent still references entry");
    key.scale = 2.0F;
    require_reuse(Access::accept(surface, key) == Change::changed && surface.value() == 2,
        "same pixel size with new scale MUST advance epoch before resize early-return");
    Access::invalidate(life); // Explicit future-owner coordination, not hidden resource freeing.
    require_reuse(!life.can_release_entry() && !life.parent_detached(), "scale invalidation defers old entry release");
    require_reuse(Access::release(life, 1, 1) && Access::forget(life, 1, entry), "old parent must detach first");
    key.composition = true;
    require_reuse(Access::accept(surface, key) == Change::changed && surface.value() == 3, "topology identity");
    key.transparent = true;
    require_reuse(Access::accept(surface, key) == Change::changed && surface.value() == 4, "intent identity");
    ++key.width;
    require_reuse(Access::accept(surface, key) == Change::changed && surface.value() == 5, "width identity");
    ++key.height;
    require_reuse(Access::accept(surface, key) == Change::changed && surface.value() == 6, "height identity");
    Access::last_surface_epoch(surface);
    const auto last = surface.value();
    require_reuse(Access::accept(surface, key) == Change::unchanged && surface.enabled(), "unchanged maximum remains valid");
    key.scale = 3.0F;
    require_reuse(Access::accept(surface, key) == Change::exhausted && surface.value() == last && !surface.enabled(),
        "changed maximum disables reuse rather than wrapping identity");
    require_reuse(Access::accept(surface, key) == Change::exhausted && !surface.enabled(), "exhaustion is sticky");
    bad = key; bad.scale = -1.0F;
    require_reuse(Access::accept(surface, bad) == Change::invalid && surface.key().scale == 3.0F,
        "invalid request cannot mutate even an exhausted owner");
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
