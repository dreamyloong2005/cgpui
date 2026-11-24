#include "dx12_initialization_internal.hpp"
#include "dx12_state_internal.hpp"
#include "dx12_device_internal.hpp"
#include "dx12_debug_internal.hpp"
#include "dx12_backdrop_blur_internal.hpp"
#include "dx12_device_cohort_internal.hpp"
#include "dx12_recovery_work_internal.hpp"
#include "dx12_surface_internal.hpp"
#include "dx12_window_alpha_internal.hpp"

namespace cgpui {
Result<void> dx12_initialize_renderer_state(const std::shared_ptr<Dx12State>& state,
    const RenderSurfaceDescriptor& descriptor, bool* rollback_candidate) {
  if (rollback_candidate) *rollback_candidate = false;
  const auto allowed = dx12_initialization_fallback_allowed();
  if (!allowed) return std::unexpected(allowed.error());
  if (auto descriptor_valid = dx12_validate_surface_descriptor(descriptor); !descriptor_valid) return descriptor_valid;
  // Diagnostics are a requested contract, not evidence for trying another GPU API.
  if (auto debug = dx12_enable_debug(); !debug) return std::unexpected(debug.error());
  if (auto surface = dx12_configure_surface(*state, descriptor); !surface) return surface;
  dx12_enroll_recovery_work(state);
  Dx12InitializationFailureClass device_failure = Dx12InitializationFailureClass::terminal;
  if (auto device = dx12_create_device(*state, &device_failure); !device) {
    if (rollback_candidate) *rollback_candidate = *allowed && device_failure == Dx12InitializationFailureClass::backend_unavailable;
    return device;
  }
  state->backdrop_blur->support = dx12_backdrop_blur_support(*state->device.Get());
  if (auto pipeline = dx12_create_pipeline(*state, &device_failure); !pipeline) {
    if (rollback_candidate) *rollback_candidate = *allowed && device_failure == Dx12InitializationFailureClass::backend_unavailable;
    return pipeline;
  }
  if (auto history = dx12_mark_opaque_window(*state); !history) return history;
  if (auto surface = dx12_create_swapchain(*state, &device_failure); !surface) {
    if (rollback_candidate) *rollback_candidate = *allowed && device_failure == Dx12InitializationFailureClass::backend_unavailable;
    return surface;
  }
  // Never hide an InfoQueue error behind a successful Vulkan initialization.
  if (auto debug = dx12_check_debug_errors(*state); !debug) return debug;
  dx12_register_device_cohort(state);
  return {};
}
} // namespace cgpui
