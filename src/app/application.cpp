#include "cgpui/app/application.hpp"

#include <utility>

namespace cgpui {

Application::Application(
    std::unique_ptr<PlatformApplication> platform_application)
    : platform_application_(std::move(platform_application)) {}

Application::Application(Application&&) noexcept = default;

Application& Application::operator=(Application&&) noexcept = default;

Application::~Application() = default;

Result<Application> Application::create() {
  auto platform_application = create_platform_application();
  if (!platform_application) {
    return std::unexpected(platform_application.error());
  }
  return Application(std::move(*platform_application));
}

PlatformApplication& Application::platform_application() {
  return *platform_application_;
}

const PlatformApplication& Application::platform_application() const {
  return *platform_application_;
}

int Application::run(
    View& view,
    AppRendererFactory renderer_factory,
    AppRunnerOptions options) {
  return run_app(
      *platform_application_,
      view,
      std::move(renderer_factory),
      std::move(options));
}

} // namespace cgpui
