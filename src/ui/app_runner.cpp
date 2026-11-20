#include "ui_internal.hpp"

namespace cgpui {

int run_app(
    PlatformApplication& application,
    View& view,
    AppRendererFactory renderer_factory,
    AppRunnerOptions options) {
  std::vector<std::unique_ptr<Renderer>> renderers;
  WindowRuntime runtime(
      application,
      view,
      [&](const RenderSurfaceDescriptor& descriptor) -> Result<Renderer*> {
        if (!renderer_factory) {
          return std::unexpected(Error{
              .code = ErrorCode::renderer_initialization_failed,
              .message = "App runner requires a renderer factory"});
        }
        auto result = renderer_factory(descriptor);
        if (!result) {
          return std::unexpected(result.error());
        }
        if (*result == nullptr) {
          return std::unexpected(Error{
              .code = ErrorCode::renderer_initialization_failed,
              .message = "App renderer factory returned an empty renderer"});
        }
        Renderer* renderer = result->get();
        renderers.push_back(std::move(*result));
        return renderer;
      });

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
