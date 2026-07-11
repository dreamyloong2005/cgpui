#include "cgpui/platform/platform_application.hpp"

namespace {
class ReopenApplication final : public cgpui::PlatformApplication {
 public:
  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_window(
      const cgpui::WindowDescriptor&, cgpui::PlatformEventCallback) override {
    return std::unexpected(cgpui::Error{});
  }
  int run() override { return 0; }
  void quit() override { quit_count += 1; }
  cgpui::PlatformReopenResult request_reopen() override {
    return dispatch_reopen("test");
  }
  int quit_count = 0;
};
} // namespace

int main() {
  ReopenApplication app;
  auto missing = app.request_reopen();
  if (!missing.supported || missing.requested || missing.backend != "test" ||
      missing.error_message != "reopen callback not registered") return 1;
  int reopen_count = 0;
  app.set_reopen_callback([&] { reopen_count += 1; });
  auto requested = app.request_reopen();
  if (!requested.supported || !requested.requested || reopen_count != 1 ||
      !requested.error_message.empty()) return 2;
  app.set_reopen_callback({});
  if (app.request_reopen().requested || reopen_count != 1) return 3;
  app.quit();
  app.quit();
  if (app.quit_count != 2) return 4;
  return 0;
}
