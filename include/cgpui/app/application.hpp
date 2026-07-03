#pragma once

#include "cgpui/platform/platform_application.hpp"
#include "cgpui/ui/runtime_rendering.hpp"

#include <memory>

namespace cgpui {

class Application {
 public:
  explicit Application(std::unique_ptr<PlatformApplication> platform_application);
  Application(Application&&) noexcept;
  Application& operator=(Application&&) noexcept;
  Application(const Application&) = delete;
  Application& operator=(const Application&) = delete;
  ~Application();

  [[nodiscard]] static Result<Application> create();

  [[nodiscard]] PlatformApplication& platform_application();
  [[nodiscard]] const PlatformApplication& platform_application() const;

  [[nodiscard]] int run(
      View& view,
      AppRendererFactory renderer_factory,
      AppRunnerOptions options = {});

 private:
  std::unique_ptr<PlatformApplication> platform_application_;
};

} // namespace cgpui
