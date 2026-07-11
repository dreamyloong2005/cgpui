#include "cgpui/cgpui.hpp"

#include <expected>
#include <memory>
#include <optional>
#include <string_view>
#include <type_traits>
#include <utility>

namespace {

static_assert(std::is_same_v<
              decltype(std::declval<cgpui::WindowRuntime&>()
                           .try_show_native_message_dialog({})),
              cgpui::Result<cgpui::NativeMessageDialogResult>>);
static_assert(std::is_same_v<
              decltype(std::declval<cgpui::WindowRuntime&>().try_open_url({})),
              cgpui::Result<cgpui::PlatformOpenUrlResult>>);
static_assert(std::is_same_v<
              decltype(std::declval<cgpui::WindowRuntime&>()
                           .try_request_reopen()),
              cgpui::Result<cgpui::PlatformReopenResult>>);
static_assert(std::is_same_v<
              decltype(std::declval<cgpui::AppContext&>()
                           .try_show_native_message_dialog({})),
              cgpui::Result<cgpui::NativeMessageDialogResult>>);
static_assert(std::is_same_v<
              decltype(std::declval<cgpui::AppContext&>().try_open_url({})),
              cgpui::Result<cgpui::PlatformOpenUrlResult>>);
static_assert(std::is_same_v<
              decltype(std::declval<cgpui::AppContext&>()
                           .try_request_reopen()),
              cgpui::Result<cgpui::PlatformReopenResult>>);

class ServiceFrame final : public cgpui::RenderFrame {
 public:
  void clear(cgpui::Color) override {}
  void draw_rect(const cgpui::SolidRect&) override {}
  cgpui::Result<void> present() override { return {}; }
};

class ServiceRenderer final : public cgpui::Renderer {
 public:
  cgpui::Result<void> resize(cgpui::Size, cgpui::DpiScale) override {
    return {};
  }
  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    return std::make_unique<ServiceFrame>();
  }
};

class ServiceView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}
};

class ServiceApplication final : public cgpui::PlatformApplication {
 public:
  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_window(
      const cgpui::WindowDescriptor&,
      cgpui::PlatformEventCallback) override {
    return std::unexpected(cgpui::Error{
        .code = cgpui::ErrorCode::window_creation_failed,
        .message = "window creation is outside this test"});
  }

  cgpui::NativeMessageDialogResult show_native_message_dialog(
      cgpui::NativeMessageDialogOptions) override {
    ++message_count;
    return message_result;
  }
  cgpui::PlatformOpenUrlResult open_url(std::string) override {
    ++open_url_count;
    return open_url_result;
  }
  cgpui::PlatformReopenResult request_reopen() override {
    ++reopen_count;
    return reopen_result;
  }
  int run() override { return 0; }
  void quit() override {}

  cgpui::NativeMessageDialogResult message_result{
      .supported = true,
      .accepted = true,
      .backend = "message-supported",
      .response = cgpui::NativeMessageDialogResponse::yes,
  };
  cgpui::PlatformOpenUrlResult open_url_result{
      .supported = true,
      .opened = true,
      .backend = "url-supported",
  };
  cgpui::PlatformReopenResult reopen_result{
      .supported = true,
      .requested = true,
      .backend = "reopen-supported",
  };
  int message_count = 0;
  int open_url_count = 0;
  int reopen_count = 0;
};

struct Fixture {
  ServiceApplication app;
  ServiceView view;
  ServiceRenderer renderer;
  cgpui::WindowRuntime runtime{
      app,
      view,
      [this](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<cgpui::Renderer*> { return &renderer; }};
};

int test_unsupported_attempts_preserve_last_supported_results() {
  Fixture fixture;
  cgpui::AppContext context{.runtime = fixture.runtime};
  if (!context.try_show_native_message_dialog({}).has_value() ||
      !context.try_open_url("https://example.com").has_value() ||
      !context.try_request_reopen().has_value()) {
    return 1;
  }

  fixture.app.message_result = {
      .supported = false,
      .backend = "message-unsupported",
      .error_message = "message unavailable",
  };
  fixture.app.open_url_result = {
      .supported = false,
      .backend = "url-unsupported",
      .error_message = "URL unavailable",
  };
  fixture.app.reopen_result = {
      .supported = false,
      .backend = "reopen-unsupported",
      .error_message = "reopen unavailable",
  };
  const auto message = context.try_show_native_message_dialog({});
  const auto url = context.try_open_url("https://example.com/unsupported");
  const auto reopen = context.try_request_reopen();
  if (message.has_value() || url.has_value() || reopen.has_value() ||
      message.error().code != cgpui::ErrorCode::unsupported_platform ||
      url.error().code != cgpui::ErrorCode::unsupported_platform ||
      reopen.error().code != cgpui::ErrorCode::unsupported_platform ||
      message.error().message != "message unavailable" ||
      url.error().message != "URL unavailable" ||
      reopen.error().message != "reopen unavailable") {
    return 2;
  }
  if (fixture.runtime.native_message_dialog_result().backend !=
          "message-supported" ||
      fixture.runtime.open_url_result().backend != "url-supported" ||
      fixture.runtime.reopen_result().backend != "reopen-supported" ||
      fixture.app.message_count != 2 || fixture.app.open_url_count != 2 ||
      fixture.app.reopen_count != 2) {
    return 3;
  }
  return 0;
}

int test_supported_incomplete_results_remain_values() {
  Fixture fixture;
  fixture.app.message_result.accepted = false;
  fixture.app.message_result.response =
      cgpui::NativeMessageDialogResponse::cancel;
  fixture.app.open_url_result.opened = false;
  fixture.app.open_url_result.error_message = "launch failed";
  fixture.app.reopen_result.requested = false;
  fixture.app.reopen_result.error_message = "callback missing";

  const auto message = fixture.runtime.try_show_native_message_dialog({});
  const auto url = fixture.runtime.try_open_url("invalid");
  const auto reopen = fixture.runtime.try_request_reopen();
  if (!message.has_value() || message->accepted ||
      message->response != cgpui::NativeMessageDialogResponse::cancel ||
      !url.has_value() || url->opened || !reopen.has_value() ||
      reopen->requested) {
    return 4;
  }
  return 0;
}

int test_platform_service_diagnostics_are_typed_and_bounded() {
  Fixture fixture;
  (void)fixture.runtime.try_show_native_message_dialog({});
  (void)fixture.runtime.try_open_url("https://example.com");
  (void)fixture.runtime.try_request_reopen();
  const auto initial = fixture.runtime.platform_diagnostics();
  if (initial.size() != 3 ||
      initial[0].kind != cgpui::PlatformDiagnosticKind::message_dialog ||
      initial[1].kind != cgpui::PlatformDiagnosticKind::open_url ||
      initial[2].kind != cgpui::PlatformDiagnosticKind::reopen) {
    return 5;
  }

  fixture.app.open_url_result = {
      .supported = false,
      .backend = "url-unsupported",
  };
  for (int attempt = 0; attempt < 40; ++attempt) {
    (void)fixture.runtime.try_open_url("https://example.com/unsupported");
  }
  const auto bounded = fixture.runtime.platform_diagnostics();
  if (bounded.size() != 32 || bounded.front().sequence != 12 ||
      bounded.back().sequence != 43 ||
      bounded.back().kind != cgpui::PlatformDiagnosticKind::open_url ||
      bounded.back().operation != "try-open-url" ||
      bounded.back().supported || bounded.back().succeeded) {
    return 6;
  }
  return 0;
}

} // namespace

int main() {
  if (const int result =
          test_unsupported_attempts_preserve_last_supported_results();
      result != 0) {
    return result;
  }
  if (const int result = test_supported_incomplete_results_remain_values();
      result != 0) {
    return result;
  }
  return test_platform_service_diagnostics_are_typed_and_bounded();
}
