#include "test_app_internal.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace cgpui::detail {

TestAppState::TestAppState()
    : parent_window(
          WindowDescriptor{.title = "CGPUI Test Root", .size = {1.0F, 1.0F}},
          {}),
      runtime(
          application,
          root_view,
          [this](const RenderSurfaceDescriptor&) -> Result<Renderer*> {
            auto renderer = std::make_unique<TestRenderer>();
            Renderer* result = renderer.get();
            renderers.push_back(std::move(renderer));
            return result;
          }) {
  WindowRuntimeRecord* root_record =
      runtime.find_window_runtime_record(runtime.root_window_runtime_id_);
  root_record->descriptor =
      WindowDescriptor{.title = "CGPUI Test Root", .size = {1.0F, 1.0F}};
  root_record->framebuffer_size = {1.0F, 1.0F};
  root_record->viewport_size = {1.0F, 1.0F};
  root_record->scale = DpiScale{1.0F};
  root_record->window = &parent_window;
  root_record->renderer = &parent_renderer;
  root_record->owns_window = false;
  root_record->owns_renderer = false;
  root_record->active = true;
  runtime.window_ = &parent_window;
  runtime.renderer_ = &parent_renderer;
}

} // namespace cgpui::detail

namespace cgpui {

TestAppWindow::TestAppWindow(
    std::shared_ptr<detail::TestAppState> state,
    WindowRuntimeId runtime_id)
    : state_(std::move(state)), runtime_id_(runtime_id) {}

WindowRuntimeId TestAppWindow::runtime_id() const { return runtime_id_; }

ViewId TestAppWindow::root_view_id() const {
  return state_->runtime.window_runtime_record(runtime_id_)->root_view_id;
}

WindowDescriptor TestAppWindow::descriptor() const {
  return state_->runtime.window_runtime_record(runtime_id_)->descriptor;
}

Size TestAppWindow::viewport_size() const {
  return state_->runtime.window_runtime_record(runtime_id_)->viewport_size;
}

DpiScale TestAppWindow::scale() const {
  return state_->runtime.window_runtime_record(runtime_id_)->scale;
}

bool TestAppWindow::active() const {
  const WindowRuntimeRecord* record =
      state_->runtime.window_runtime_record(runtime_id_);
  return record != nullptr && record->active;
}

Window TestAppWindow::window() const {
  return Window(state_->runtime, runtime_id_);
}

View* TestAppWindow::root_view() {
  return state_->runtime.find_view(root_view_id());
}

const View* TestAppWindow::root_view() const {
  const WindowRuntime& runtime = state_->runtime;
  return runtime.find_view(root_view_id());
}

TestApp::TestApp() : state_(std::make_shared<detail::TestAppState>()) {}

TestApp::TestApp(TestApp&&) noexcept = default;

TestApp& TestApp::operator=(TestApp&&) noexcept = default;

TestApp::~TestApp() = default;

TestAppWindow TestApp::open_window(
    WindowOptions options,
    std::unique_ptr<View> root_view) {
  auto result = try_open_window(std::move(options), std::move(root_view));
  if (!result) throw std::runtime_error(result.error().message);
  return std::move(*result);
}

Result<TestAppWindow> TestApp::try_open_window(
    WindowOptions options,
    std::unique_ptr<View> root_view) {
  if (root_view == nullptr) {
    return std::unexpected(Error{
        .code = ErrorCode::invalid_argument,
        .message = "TestApp requires a root view"});
  }
  auto result =
      state_->runtime.try_open_window(std::move(options), std::move(root_view));
  if (!result) return std::unexpected(result.error());
  return TestAppWindow(state_, result->runtime_id);
}

std::size_t TestApp::window_count() const {
  return state_->runtime.app_opened_windows().size();
}

std::optional<TestAppWindow> TestApp::window(
    WindowRuntimeId runtime_id) const {
  const auto opened = state_->runtime.app_opened_windows();
  const auto match = std::find_if(
      opened.begin(), opened.end(), [runtime_id](const AppOpenedWindow& item) {
        return item.runtime_id == runtime_id;
      });
  if (match == opened.end()) {
    return std::nullopt;
  }
  return TestAppWindow(state_, runtime_id);
}

} // namespace cgpui
