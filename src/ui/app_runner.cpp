#include "ui_internal.hpp"

namespace cgpui {

int run_app(
    PlatformApplication& application,
    View& view,
    AppRendererFactory renderer_factory,
    AppRunnerOptions options) {
  WindowRuntime runtime(application, view, std::move(renderer_factory), WindowRuntime::OwnedRendererFactoryTag{});

  if (options.setup) {
    options.setup(runtime);
  }
  // The context outlives run(): setup_context receives it by reference, and
  // callbacks it installs (app_runner_test's on_run) may keep using it while
  // the runtime runs. Scoped to the `if`, it was a stack-use-after-scope
  // (AddressSanitizer, test_app_opened_window_close_cleans_runtime_owned_state).
  AppContext context{.runtime = runtime};
  if (options.setup_context) {
    options.setup_context(context);
  }
  return runtime.run(options.window, options.runtime);
}

} // namespace cgpui
