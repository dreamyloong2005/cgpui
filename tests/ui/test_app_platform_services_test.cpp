#include "cgpui/ui/runtime_app_context.hpp"
#include "cgpui/ui/runtime_context.hpp"
#include "cgpui/ui/test_app.hpp"

#include <memory>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

namespace {

class ServiceView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::WindowRuntimeContext& context) override {
    if (!std::holds_alternative<cgpui::KeyboardKey>(event)) {
      return cgpui::EventResult::unhandled();
    }
    cgpui::NativeMenuModel menu;
    menu.items.push_back(cgpui::NativeMenuItem{
        .title = "Test Menu",
        .action_name = "test.menu",
    });
    auto menu_result = context.app_context().try_install_native_menu(
        std::move(menu));
    if (menu_result) installed_menu = *menu_result;

    auto file_result = context.app_context().try_show_native_file_dialog(
        cgpui::NativeFileDialogOptions{
            .kind = cgpui::NativeFileDialogKind::open_files,
            .title = "Choose files",
            .filters = {{.name = "Text", .extensions = {"txt"}}},
        });
    file_available = file_result.has_value();
    selected_files = file_result ? std::optional{*file_result} : std::nullopt;

    auto prompt_result = context.app_context().try_show_native_message_dialog(
        cgpui::NativeMessageDialogOptions{
            .kind = cgpui::NativeMessageDialogKind::question,
            .buttons = cgpui::NativeMessageDialogButtons::yes_no,
            .title = "Continue?",
            .message = "Run the service test?",
        });
    prompt_available = prompt_result.has_value();
    prompt = prompt_result ? std::optional{*prompt_result} : std::nullopt;

    auto result = context.app_context().try_open_url(
        "https://example.test/test-app-services");
    if (result) opened = *result;
    auto reopen_result = context.app_context().try_request_reopen();
    if (reopen_result) reopen = *reopen_result;
    return cgpui::EventResult::consumed_event();
  }

  std::optional<cgpui::NativeMenuInstallation> installed_menu;
  std::optional<cgpui::NativeFileDialogResult> selected_files;
  std::optional<cgpui::NativeMessageDialogResult> prompt;
  std::optional<cgpui::PlatformOpenUrlResult> opened;
  std::optional<cgpui::PlatformReopenResult> reopen;
  bool file_available = false;
  bool prompt_available = false;
};

} // namespace

int main() {
  cgpui::TestApp first;
  cgpui::TestApp second;
  if (first.read_from_clipboard().has_value() ||
      second.read_from_clipboard().has_value() || first.opened_url().has_value()) {
    return 1;
  }
  if (!first.write_to_clipboard("test clipboard") ||
      first.read_from_clipboard() != std::optional<std::string>{"test clipboard"} ||
      second.read_from_clipboard().has_value()) {
    return 2;
  }

  cgpui::TestAppWindow window = first.open_window(
      cgpui::WindowOptions{}, std::make_unique<ServiceView>());
  ServiceView* view = window.root_view_as<ServiceView>();
  if (view == nullptr) return 3;

  first.simulate_path_prompt_response(
      std::vector<std::string>{"alpha.txt", "beta.txt"});
  first.simulate_prompt_answer(cgpui::NativeMessageDialogResponse::yes);
  window.dispatch_keystroke(cgpui::KeyboardKey{.key_code = 'S'});
  if (!view->installed_menu.has_value() ||
      !view->installed_menu->platform.supported ||
      view->installed_menu->platform.item_count != 1 ||
      !view->selected_files.has_value() || !view->selected_files->accepted ||
      view->selected_files->paths !=
          std::vector<std::string>{"alpha.txt", "beta.txt"} ||
      !view->prompt.has_value() || !view->prompt->accepted ||
      view->prompt->response != cgpui::NativeMessageDialogResponse::yes ||
      !view->opened.has_value() || !view->opened->supported ||
      !view->opened->opened || view->opened->backend != "test" ||
      !view->reopen.has_value() || !view->reopen->requested ||
      first.opened_url() !=
          std::optional<std::string>{"https://example.test/test-app-services"} ||
      second.opened_url().has_value()) {
    return 4;
  }

  cgpui::TestPlatformServiceSnapshot snapshot =
      first.platform_service_snapshot();
  if (snapshot.native_menu_install_count != 1 ||
      snapshot.file_dialog_count != 1 || snapshot.message_dialog_count != 1 ||
      snapshot.open_url_count != 1 || snapshot.reopen_count != 1 ||
      snapshot.pending_path_responses != 0 ||
      snapshot.pending_prompt_responses != 0) {
    return 5;
  }

  first.simulate_path_prompt_response(std::nullopt);
  first.simulate_prompt_answer(cgpui::NativeMessageDialogResponse::cancel);
  window.dispatch_keystroke(cgpui::KeyboardKey{.key_code = 'C'});
  if (!view->file_available || !view->selected_files.has_value() ||
      view->selected_files->accepted || !view->selected_files->paths.empty() ||
      !view->prompt_available || !view->prompt.has_value() ||
      view->prompt->accepted ||
      view->prompt->response != cgpui::NativeMessageDialogResponse::cancel) {
    return 6;
  }

  window.dispatch_keystroke(cgpui::KeyboardKey{.key_code = 'N'});
  snapshot = first.platform_service_snapshot();
  if (view->file_available || view->selected_files.has_value() ||
      view->prompt_available || view->prompt.has_value() ||
      snapshot.native_menu_install_count != 3 ||
      snapshot.file_dialog_count != 3 || snapshot.message_dialog_count != 3 ||
      snapshot.open_url_count != 3 || snapshot.reopen_count != 3) {
    return 7;
  }
  return 0;
}
