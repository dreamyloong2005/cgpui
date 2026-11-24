#include "dx12_surface_policy_internal.hpp"

namespace cgpui {
Dx12SurfaceAlphaChange dx12_surface_alpha_change(
    const Dx12SurfacePolicy& policy, bool premultiplied, bool requested) noexcept {
  if (policy.dynamic_ordinary && dx12_composition_route(policy))
    return premultiplied == requested ? Dx12SurfaceAlphaChange::intent_only
                                     : Dx12SurfaceAlphaChange::replace_chain;
  // Native Popup and initially-alpha ordinary surfaces retain their sticky
  // PREMULT route even when the caller changes only transparent intent to false.
  return requested && !premultiplied ? Dx12SurfaceAlphaChange::unsupported
                                   : Dx12SurfaceAlphaChange::intent_only;
}
} // namespace cgpui
