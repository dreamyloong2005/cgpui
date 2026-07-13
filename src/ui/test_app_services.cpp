#include "test_app_internal.hpp"

#include <utility>

namespace cgpui::detail {

bool TestAppState::write_to_clipboard(std::string_view text) {
  return runtime.write_clipboard_text(text);
}

std::optional<std::string> TestAppState::read_from_clipboard() {
  return runtime.read_clipboard_text();
}

std::optional<std::string> TestAppState::opened_url() const {
  return application.opened_url();
}

void TestAppState::simulate_path_prompt_response(
    std::optional<std::vector<std::string>> paths) {
  application.simulate_path_prompt_response(std::move(paths));
}

void TestAppState::simulate_prompt_answer(
    NativeMessageDialogResponse response) {
  application.simulate_prompt_answer(response);
}

TestPlatformServiceSnapshot TestAppState::service_snapshot() const {
  return application.service_snapshot();
}

} // namespace cgpui::detail

namespace cgpui {

bool TestApp::write_to_clipboard(std::string_view text) const {
  return state_->write_to_clipboard(text);
}

std::optional<std::string> TestApp::read_from_clipboard() const {
  return state_->read_from_clipboard();
}

std::optional<std::string> TestApp::opened_url() const {
  return state_->opened_url();
}

void TestApp::simulate_path_prompt_response(
    std::optional<std::vector<std::string>> paths) const {
  state_->simulate_path_prompt_response(std::move(paths));
}

void TestApp::simulate_prompt_answer(
    NativeMessageDialogResponse response) const {
  state_->simulate_prompt_answer(response);
}

TestPlatformServiceSnapshot TestApp::platform_service_snapshot() const {
  return state_->service_snapshot();
}

} // namespace cgpui
