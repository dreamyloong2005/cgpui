#include "dx12_ordinary_pixels_support.hpp"
#include "dx12_diagnostics_support.hpp"
#include "dx12_command_reuse_public_owner_test_support.hpp"
#include "../../src/renderer/dx12/dx12_composition_internal.hpp"
#include "../../src/renderer/dx12/dx12_composition_api_internal.hpp"
#include "../../src/renderer/dx12/dx12_window_alpha_internal.hpp"

namespace {
class ForwardingFailure final : public cgpui::Dx12CompositionApi {
 public:
  unsigned bind_failures = 0, binds = 0, commits = 0;
  bool fail_commit = false;
  cgpui::Renderer* renderer = nullptr;
  bool reentry_checked = false, reentry_rejected = false;
  HRESULT bind(IDCompositionVisual* visual, IUnknown* content) override {
    ++binds;
    if (renderer && !reentry_checked) {
      reentry_checked = true;
      auto nested_frame = renderer->begin_frame();
      auto nested_change = renderer->set_transparent_surface(true);
      reentry_rejected = !nested_frame && !nested_change &&
          nested_frame.error().code == cgpui::ErrorCode::invalid_argument &&
          nested_change.error().code == cgpui::ErrorCode::invalid_argument;
    }
    if (bind_failures) { --bind_failures; return E_FAIL; }
    return visual->SetContent(content);
  }
  HRESULT commit(IDCompositionDevice* device) override {
    ++commits;
    if (fail_commit) return E_FAIL;
    return device->Commit();
  }
};
}
int main() {
  using namespace cgpui;
  using namespace cgpui_test;
  Dx12RecoveryDeadline deadline{DesktopTestAccess::exclusive};
  OrdinaryDesktop desktop;
  auto app = create_platform_application();
  if (!deadline.done || !desktop.valid() || !app) return 1;
  auto window = (*app)->create_window(desktop.descriptor(false), [](const PlatformEvent&) {});
  if (!window || !ordinary_install_menu(**app, **window) || !desktop.place(**window)) return 2;
  if (const auto gated = ordinary_alpha_os_gate(**window)) return *gated;
  ForwardingFailure api; // Outlives renderer, local state reference and retained frame.
  auto renderer = ordinary_renderer(**window, false);
  if (!renderer || !ordinary_draw_bands(**renderer, **window, false)) return 3;
  auto state = reuse_public_state((*renderer)->backend_info(), ordinary_hwnd(**window)).lock();
  if (!state || !state->composition_host || !state->window_alpha || !state->surface_policy.dynamic_ordinary) return 4;
  state->composition_host->api = &api;
  api.renderer = renderer->get();
  auto* chain = state->swapchain.Get();
  auto* host = state->composition_host.get();
  api.bind_failures = 1;
  const auto rejected = (*renderer)->set_transparent_surface(true);
  if (rejected || state->failed || state->swapchain.Get() != chain || state->composition_host.get() != host ||
      host->retired_chain || state->window_alpha->enabled || !ordinary_composition((*renderer)->surface_composition(), false) ||
      api.binds != 2 || api.commits || !api.reentry_rejected || !ordinary_draw_bands(**renderer, **window, true) ||
      !ordinary_desktop_bands(**window, true, false)) return 5;
  const auto saved_pixels = *(*renderer)->last_frame_pixels();
  const auto saved_diagnostics = *(*renderer)->last_frame_diagnostic_snapshot();
  const auto* borrowed_diagnostics = (*renderer)->last_frame_diagnostic_snapshot();
  if (!(*renderer)->set_transparent_surface(true) || state->swapchain.Get() == chain ||
      state->composition_host.get() != host || !host->retired_chain || host->committed) return 6;
  api.fail_commit = true;
  auto frame = wait_for_test_frame(**renderer);
  if (!frame) return 7;
  (*frame)->clear({0, 1, 0, .5F});
  if (!(*frame)->request_pixel_capture()) return 8;
  const auto failed = (*frame)->present();
  const auto* pixels = (*renderer)->last_frame_pixels();
  if (failed || api.commits != 1 || host->committed || !host->retired_chain || !state->failed ||
      !pixels || pixels->rgba8 != saved_pixels.rgba8 ||
      (*renderer)->last_frame_diagnostic_snapshot() != borrowed_diagnostics ||
      !diagnostic_same((*renderer)->last_frame_diagnostic_snapshot(), saved_diagnostics) ||
      FAILED(state->device->GetDeviceRemovedReason())) return 9;
  // Public Present failure must retain first-success capture/diagnostics and the
  // old visual chain; it must not claim completion merely because GPU work ran.
  return 0;
}
