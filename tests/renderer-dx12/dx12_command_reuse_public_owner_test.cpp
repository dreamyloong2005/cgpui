#include "dx12_command_reuse_public_owner_test_support.hpp"
#include "dx12_srv_pool_test_support.hpp"
#include "../support/frame_wait_test_support.hpp"
#include "../../src/renderer/dx12/dx12_operation_internal.hpp"
#include <array>

int main() {
  using namespace cgpui; using namespace cgpui_test;
  Dx12RecoveryDeadline deadline; Dx12Window window;
  std::atomic_bool parent_released = false, bundle_released = false, heap_released = false;
  if (!deadline.done || !window.hwnd) return dx12_fail(1, "public BUNDLE owner HWND/deadline");
  auto created = create_dx12_renderer(window.descriptor());
  if (!created) return dx12_fail(2, "actual public DX12 factory");
  auto renderer = std::move(*created);
  const auto info = renderer->backend_info();
  if (info.backend != RendererBackend::dx12 || !info.adapter_luid || !info.device_generation)
    return dx12_fail(3, "public owner real native identity");
  const auto weak = reuse_public_state(info, window.hwnd);
  ReuseQueueTail* verifier = nullptr;
  { auto state = weak.lock();
    if (!state) return dx12_fail(4, "exact public cohort state observation unavailable");
    Dx12OperationGuard guard(*state);
    state->command_observation = std::make_unique<Dx12CommandObservation>(16384);
    auto tail = std::make_unique<ReuseQueueTail>(); verifier = tail.get();
    state->command_observation->verifier = std::move(tail);
  }
  { auto warm = wait_for_test_frame(*renderer);
    if (!warm) return dx12_fail(5, "public owner warm frame");
    reuse_public_scene(**warm, true);
    if (!(*warm)->request_pixel_capture() || !(*warm)->present()) return dx12_fail(6, "public owner warm capture");
  } // Finished frames cannot accidentally keep the state alive later.
  const auto* captured = renderer->last_frame_pixels();
  if (!captured || !captured->valid()) return dx12_fail(7, "public owner warm pixels");
  const auto pixels = captured->rgba8;
  std::array<std::uint64_t, 3> parents{};
  for (unsigned i = 0; i < 2; ++i) {
    auto frame = wait_for_test_frame(*renderer);
    if (!frame) return dx12_fail(8, "public owner fresh/hit begin");
    reuse_public_scene(**frame, false);
    if (!(*frame)->present()) return dx12_fail(9, "public owner fresh/hit Present");
    frame->reset();
    { auto state = weak.lock();
      if (!state) return dx12_fail(10, "public owner state disappeared");
      Dx12OperationGuard guard(*state);
      if (!reuse_trace_pixels_ready(*state, *verifier, pixels)) return dx12_fail(11, "public owner fresh/hit native pixels");
      parents[i] = verifier->parent;
    }
  }
  { auto state = weak.lock();
    if (!state) return dx12_fail(12, "public owner native sentinel setup");
    Dx12OperationGuard guard(*state); auto* bundle = reuse_last_bundle(*state);
    if (!bundle || state->texture_renderer->native_textures.size() != 2 ||
        reuse_event_count(*state, Dx12CommandEventKind::bundle_created) != 1 ||
        !reuse_native_sentinel(*state->commands.Get(), parent_released) ||
        !reuse_native_sentinel(*bundle, bundle_released) ||
        !srv_heap_sentinel(*state->texture_renderer->native_textures.begin()->second.srv.heap(), heap_released))
      return dx12_fail(13, "public owner native parent/BUNDLE/heap sentinels");
  }
  auto acquired = wait_for_test_frame(*renderer);
  if (!acquired) return dx12_fail(14, "public owner unfinished frame unavailable");
  auto orphan = std::move(*acquired);
  renderer.reset(); // Actual Dx12Renderer destructor; no manual owner_alive mutation.
  if (weak.use_count() != 1 || parent_released || bundle_released || heap_released || !IsWindow(window.hwnd))
    return dx12_fail(15, "public facade deletion retired resources or retained an extra state owner");
  reuse_public_scene(*orphan, false);
  const auto presented = orphan->present(); // Single original-epoch completion, never retry Present.
  if (!presented) return dx12_fail(16, "publicly acquired BUNDLE-backed orphan failed original-epoch Present");
  const auto again = orphan->present(); const auto capture = orphan->request_pixel_capture();
  if (again || again.error().code != ErrorCode::invalid_argument ||
      capture || capture.error().code != ErrorCode::invalid_argument)
    return dx12_fail(17, "finished public orphan regained frame permission");
  { auto state = weak.lock();
    if (!state) return dx12_fail(18, "public orphan lost its retained state");
    Dx12OperationGuard guard(*state); parents[2] = verifier->parent;
    if (state->owner_alive || state->info.device_generation != info.device_generation || state->submitted ||
        !reuse_trace_pixels_ready(*state, *verifier, pixels) || !reuse_native_history(state->command_observation->events(), parents) ||
        reuse_event_count(*state, Dx12CommandEventKind::bundle_released) ||
        parent_released || bundle_released || heap_released || state->srv_pool->stats().live_slots != 2 ||
        state->command_observation->overflowed() || !dx12_check_debug_errors(*state))
      return dx12_fail(19, "public orphan changed epoch/re-recorded BUNDLE/lost exact pixels or ownership");
  }
  if (weak.use_count() != 1) return dx12_fail(20, "fixture retained a strong state owner before final frame release");
  orphan.reset();
  if (!weak.expired() || !parent_released || !bundle_released || !heap_released || !IsWindow(window.hwnd))
    return dx12_fail(21, "last public frame failed native parent/BUNDLE/heap retirement");
  return 0;
}
