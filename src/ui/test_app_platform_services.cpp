#include "test_app_internal.hpp"

#include <utility>

namespace cgpui::detail {

PlatformMenuInstallationResult TestPlatformApplication::install_native_menu(
    NativeMenuModel menu) {
  service_snapshot_.native_menu_install_count += 1;
  const std::size_t accelerator_count = native_menu_accelerator_count(menu);
  return PlatformMenuInstallationResult{
      .supported = true,
      .backend = "test",
      .menu_count = menu.items.size(),
      .item_count = native_menu_item_count(menu),
      .accelerator_count = accelerator_count,
      .registered_accelerator_count = accelerator_count,
  };
}

NativeFileDialogResult TestPlatformApplication::show_native_file_dialog(
    NativeFileDialogOptions options) {
  service_snapshot_.file_dialog_count += 1;
  NativeFileDialogResult result{
      .backend = "test",
      .kind = options.kind,
      .filter_count = options.filters.size(),
  };
  if (path_responses_.empty()) {
    result.error_message = "no simulated path prompt response";
    return result;
  }
  std::optional<std::vector<std::string>> paths =
      std::move(path_responses_.front());
  path_responses_.pop_front();
  result.supported = true;
  if (paths.has_value()) {
    result.paths = std::move(*paths);
    result.accepted = !result.paths.empty();
  }
  return result;
}

NativeMessageDialogResult TestPlatformApplication::show_native_message_dialog(
    NativeMessageDialogOptions) {
  service_snapshot_.message_dialog_count += 1;
  if (prompt_responses_.empty()) {
    return NativeMessageDialogResult{
        .backend = "test",
        .error_message = "no simulated prompt response",
    };
  }
  const NativeMessageDialogResponse response = prompt_responses_.front();
  prompt_responses_.pop_front();
  return NativeMessageDialogResult{
      .supported = true,
      .accepted = response == NativeMessageDialogResponse::ok ||
          response == NativeMessageDialogResponse::yes,
      .backend = "test",
      .response = response,
  };
}

PlatformOpenUrlResult TestPlatformApplication::open_url(std::string url) {
  service_snapshot_.open_url_count += 1;
  service_snapshot_.opened_url = std::move(url);
  return PlatformOpenUrlResult{
      .supported = true,
      .opened = !service_snapshot_.opened_url->empty(),
      .backend = "test",
  };
}

PlatformReopenResult TestPlatformApplication::request_reopen() {
  service_snapshot_.reopen_count += 1;
  return PlatformReopenResult{
      .supported = true,
      .requested = true,
      .backend = "test",
  };
}

void TestPlatformApplication::simulate_path_prompt_response(
    std::optional<std::vector<std::string>> paths) {
  path_responses_.push_back(std::move(paths));
}

void TestPlatformApplication::simulate_prompt_answer(
    NativeMessageDialogResponse response) {
  prompt_responses_.push_back(response);
}

std::optional<std::string> TestPlatformApplication::opened_url() const {
  return service_snapshot_.opened_url;
}

TestPlatformServiceSnapshot TestPlatformApplication::service_snapshot() const {
  TestPlatformServiceSnapshot snapshot = service_snapshot_;
  snapshot.pending_path_responses = path_responses_.size();
  snapshot.pending_prompt_responses = prompt_responses_.size();
  return snapshot;
}

} // namespace cgpui::detail
