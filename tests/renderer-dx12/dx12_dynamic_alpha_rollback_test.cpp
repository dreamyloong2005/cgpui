#include "dx12_ordinary_pixels_support.hpp"
#include "dx12_command_reuse_public_owner_test_support.hpp"
#include "../../src/renderer/dx12/dx12_composition_internal.hpp"
#include "../../src/renderer/dx12/dx12_composition_api_internal.hpp"
#include "../../src/renderer/dx12/dx12_swapchain_candidate_internal.hpp"

namespace {
class FailedBindings final : public cgpui::Dx12CompositionApi {
 public:
  unsigned remaining = 0, calls = 0;
  HRESULT bind(IDCompositionVisual* visual, IUnknown* content) override {
    ++calls;
    if (remaining) { --remaining; return E_FAIL; }
    return visual->SetContent(content);
  }
  HRESULT commit(IDCompositionDevice* device) override { return device->Commit(); }
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
  FailedBindings api;
  auto renderer = ordinary_renderer(**window, false);
  if (!renderer || !ordinary_draw_bands(**renderer, **window, false)) return 3;
  auto state = reuse_public_state((*renderer)->backend_info(), ordinary_hwnd(**window)).lock();
  if (!state || !state->composition_host) return 4;
  auto& host = *state->composition_host;
  host.api = &api;
  auto* original = state->swapchain.Get();
  const auto previous = (*renderer)->last_frame_pixels()->rgba8;
  if (!(*renderer)->set_transparent_surface(true) || !host.retired_chain ||
      host.retired_chain->swapchain.Get() != original) return 5;
  auto* pending = state->swapchain.Get();
  // The second candidate's bind AND restoration of the prior pending binding
  // fail. Scripted HRESULTs exercise ownership, not actual driver-failure proof.
  api.remaining = 2;
  const auto failed = (*renderer)->set_transparent_surface(false);
  if (failed || !state->failed || api.calls != 3 || !host.retired_chain || !host.failed_binding ||
      host.retired_chain->swapchain.Get() != original || state->swapchain.Get() != pending ||
      host.failed_binding->swapchain.Get() == pending || host.failed_binding->swapchain.Get() == original ||
      (*renderer)->last_frame_pixels()->rgba8 != previous ||
      !ordinary_composition((*renderer)->surface_composition(), true)) return 6;
  const auto calls = api.calls;
  if ((*renderer)->set_transparent_surface(false) || api.calls != calls) return 7;
  state.reset(); renderer->reset(); // Detaches visual before retiring the three bounded owners.
  auto next = ordinary_renderer(**window, false);
  if (!next || !ordinary_draw_bands(**next, **window, true) || !ordinary_desktop_bands(**window, true, false)) return 8;
  return 0;
}
