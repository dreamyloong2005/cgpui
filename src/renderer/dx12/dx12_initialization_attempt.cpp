#include "dx12_initialization_attempt_internal.hpp"
#include "dx12_initialization_internal.hpp"
#include "dx12_initialization_rollback_internal.hpp"
#include "dx12_initialization_failure_internal.hpp"
#include "dx12_frame_pacing_native_internal.hpp"
#include "dx12_operation_internal.hpp"
#include "dx12_state_internal.hpp"

namespace cgpui {
detail::RendererInitializationAttempt dx12_attempt_renderer_initialization(
    const RenderSurfaceDescriptor& descriptor) {
  return dx12_attempt_renderer_initialization_with_api(descriptor, dx12_native_frame_pacing_api());
}
detail::RendererInitializationAttempt dx12_attempt_renderer_initialization_with_api(
    const RenderSurfaceDescriptor& descriptor, Dx12FramePacingApi& pacing, Dx12SynchronizationApi* synchronization) {
  auto state = std::make_shared<Dx12State>(&pacing, synchronization);
  Dx12OperationGuard operation(*state);
  const auto window = dx12_initialization_window(descriptor);
  bool rollback_candidate = false;
  auto initialized = dx12_initialize_renderer_state(state, descriptor, &rollback_candidate);
  if (initialized) return dx12_wrap_initialized_renderer(state);
  Error error = initialized.error();
  if (!rollback_candidate) return std::unexpected(detail::RendererInitializationFailure{std::move(error)});
  if (auto retired = dx12_rollback_initialization(*state, window); !retired) {
    error.message += "; " + retired.error().message;
    return std::unexpected(detail::RendererInitializationFailure{std::move(error)});
  }
  // Native/window callbacks may have changed configuration during construction.
  // A new diagnostic requirement or malformed setting must not be bypassed.
  auto allowed = dx12_initialization_fallback_allowed();
  if (!allowed) return std::unexpected(detail::RendererInitializationFailure{allowed.error()});
  return std::unexpected(detail::RendererInitializationFailure{std::move(error), *allowed});
}
} // namespace cgpui
