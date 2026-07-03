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
  if (options.setup_context) {
    AppContext context{.runtime = runtime};
    options.setup_context(context);
  }
  return runtime.run(options.window, options.runtime);
}

} // namespace cgpui
