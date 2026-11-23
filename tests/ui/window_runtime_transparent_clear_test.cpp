#include "runtime_window_clear_test_support.hpp"

using namespace runtime_clear_test;

namespace {
int run_case(bool secondary, bool supported, RendererCompositeAlpha alpha, bool transparent) {
  Fixture fixture({.supported = supported, .composite_alpha = alpha});
  auto root = WindowOptions{}.size(160, 90).transparent(!secondary && transparent).to_descriptor();
  int failure = 0;
  fixture.app.on_run = [&] {
    const auto child = fixture.runtime.try_open_window(
        WindowOptions{}.size(160, 90).transparent(secondary && transparent),
        std::make_unique<runtime_clear_test::View>());
    if (!child || fixture.surface_requests.size() != 2) { failure = 1; return; }
    if (fixture.surface_requests[secondary ? 1 : 0].transparent_background != transparent ||
        fixture.surface_requests[secondary ? 0 : 1].transparent_background) { failure = 2; return; }
    const bool expected = transparent && supported &&
        (alpha == RendererCompositeAlpha::pre_multiplied || alpha == RendererCompositeAlpha::post_multiplied);
    const bool root_ok = painted(fixture.app.root, !secondary && expected);
    const bool child_ok = painted(fixture.app.child, secondary && expected);
    if (!root_ok || !child_ok) {
      std::cerr << "clear root=" << root_ok << " secondary=" << child_ok << '\n';
      failure = 3;
    }
    if (expected) {
      auto& actual = fixture.renderers[secondary ? 1 : 0]->composition;
      auto& target = secondary ? fixture.app.child : fixture.app.root;
      // Recovery can change surface alpha. The original descriptor/factory
      // request stays transparent, but every new frame must inspect reality.
      for (const auto recovered : {RendererCompositeAlpha::inherit, RendererCompositeAlpha::opaque,
                                    RendererCompositeAlpha::post_multiplied}) {
        actual.composite_alpha = recovered;
        if (!painted(target, recovered == RendererCompositeAlpha::post_multiplied)) failure = 5;
      }
      actual.supported = false;
      if (!painted(target, false)) failure = 6;
    }
  };
  const int result = fixture.runtime.run(root, {.request_initial_redraw = false});
  if (result != 0 || fixture.app.quits) failure = 4;
  if (failure) std::cerr << "transparent clear: secondary=" << secondary << " supported=" << supported
      << " alpha=" << int(alpha) << " intent=" << transparent << " failure=" << failure << '\n';
  return failure;
}
} // namespace

int main() {
  int failures = 0;
  for (const bool secondary : {false, true}) {
    for (const bool supported : {true, false}) {
      for (const auto alpha : {RendererCompositeAlpha::pre_multiplied,
                              RendererCompositeAlpha::post_multiplied, RendererCompositeAlpha::inherit,
                              RendererCompositeAlpha::opaque, RendererCompositeAlpha::unknown}) {
        for (const bool transparent : {true, false})
          failures += run_case(secondary, supported, alpha, transparent) != 0;
      }
    }
  }
  return failures == 0 ? 0 : 1;
}
