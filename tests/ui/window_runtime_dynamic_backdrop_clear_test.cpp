#include "runtime_window_clear_test_support.hpp"

using namespace runtime_clear_test;

namespace {
int run_case(bool secondary, bool explicit_transparent, WindowBackdrop creation_backdrop,
             RendererCompositeAlpha alpha) {
  Fixture fixture({.supported = true, .composite_alpha = alpha});
  const auto options = WindowOptions{}.size(160, 90).transparent(explicit_transparent).backdrop(creation_backdrop);
  const auto root = secondary ? WindowOptions{}.size(160, 90).to_descriptor() : options.to_descriptor();
  int failure = 0;
  fixture.app.on_run = [&] {
    auto id = fixture.runtime.root_window_runtime_id();
    if (secondary) {
      const auto opened = fixture.runtime.try_open_window(
          options, std::make_unique<runtime_clear_test::View>());
      if (!opened) { failure = 1; return; }
      id = opened->runtime_id;
    }
    cgpui::Window window(fixture.runtime, id);
    Native& target = secondary ? fixture.app.child : fixture.app.root;
    const bool actual_alpha = alpha == RendererCompositeAlpha::pre_multiplied;
    const bool explicit_clear = explicit_transparent && actual_alpha;
    // The descriptor is the creation request, NOT the live backdrop state.
    // A retained alpha surface after removing Mica does not itself request a
    // transparent clear; only explicit transparency or an applied material does.
    const bool initial = painted(target,
        actual_alpha && (explicit_transparent || creation_backdrop != WindowBackdrop::none));
    const auto removed = window.set_backdrop(WindowBackdrop::none);
    const bool no_material = painted(target, explicit_clear);
    const auto applied = window.set_backdrop(WindowBackdrop::acrylic);
    const bool material = painted(target, actual_alpha);
    target.material_visible = false; // Simulate OS transparency-effects refusal.
    const auto unavailable = window.set_backdrop(WindowBackdrop::mica);
    const bool fallback = painted(target, explicit_clear);
    if (!initial || !no_material || !material || !fallback) failure = 2;
    if (removed.applied != WindowBackdrop::none ||
        applied.applied != (actual_alpha ? WindowBackdrop::acrylic : WindowBackdrop::none) ||
        unavailable.applied != WindowBackdrop::none ||
        window.descriptor().backdrop != creation_backdrop ||
        window.descriptor().chrome.transparent_background != explicit_transparent) failure = 3;
  };
  const int result = fixture.runtime.run(root, {.request_initial_redraw = false});
  if (result != 0 || fixture.app.quits) failure = 4;
  if (failure) std::cerr << "dynamic clear: secondary=" << secondary << " explicit=" << explicit_transparent
      << " creation=" << int(creation_backdrop) << " alpha=" << int(alpha) << " failure=" << failure << '\n';
  return failure;
}
} // namespace

int main() {
  int failures = 0;
  for (const bool secondary : {false, true}) {
    for (const bool explicit_transparent : {true, false}) {
      for (const auto creation : {WindowBackdrop::none, WindowBackdrop::mica}) {
        for (const auto alpha : {RendererCompositeAlpha::pre_multiplied,
                                RendererCompositeAlpha::inherit, RendererCompositeAlpha::opaque})
          failures += run_case(secondary, explicit_transparent, creation, alpha) != 0;
      }
    }
  }
  return failures == 0 ? 0 : 1;
}
