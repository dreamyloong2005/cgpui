#include "ui_internal.hpp"

namespace cgpui {

WindowOptions& WindowOptions::title(std::string title_value) {
  descriptor.title = std::move(title_value);
  return *this;
}

WindowOptions& WindowOptions::size(Size size_value) {
  descriptor.size = size_value;
  return *this;
}

WindowOptions& WindowOptions::size(float width, float height) {
  descriptor.size = Size{.width = width, .height = height};
  return *this;
}

WindowOptions& WindowOptions::titlebar_visible(bool visible) {
  descriptor.chrome.titlebar_visible = visible;
  return *this;
}

WindowOptions& WindowOptions::decorations(bool enabled) {
  descriptor.chrome.decorations = enabled;
  return *this;
}

WindowOptions& WindowOptions::resizable(bool enabled) {
  descriptor.chrome.resizable = enabled;
  return *this;
}

WindowOptions& WindowOptions::transparent(bool enabled) {
  descriptor.chrome.transparent_background = enabled;
  return *this;
}

WindowDescriptor WindowOptions::to_descriptor() const {
  return descriptor;
}

AppOpenedWindow AppContext::open_window(WindowOptions options) const {
  return runtime.open_window(std::move(options));
}

AppOpenedWindow AppContext::open_window(
    WindowOptions options,
    std::unique_ptr<View> root_view) const {
  return runtime.open_window(std::move(options), std::move(root_view));
}

NativeMenuInstallation AppContext::install_native_menu(
    NativeMenuModel menu) const {
  return runtime.install_native_menu(std::move(menu));
}

NativeFileDialogResult AppContext::show_native_file_dialog(
    NativeFileDialogOptions options) const {
  return runtime.show_native_file_dialog(std::move(options));
}

void AppContext::register_command_palette_entry(
    CommandPaletteEntry entry) const {
  runtime.register_command_palette_entry(std::move(entry));
}

std::span<const CommandPaletteEntry> AppContext::command_palette_entries()
    const {
  return runtime.command_palette_entries();
}

std::vector<CommandPaletteEntry> AppContext::command_palette_entries_for_group(
    std::string_view group) const {
  return runtime.command_palette_entries_for_group(group);
}

ActionDispatchResult AppContext::dispatch_command_palette_entry(
    const CommandPaletteEntry& entry) const {
  return runtime.dispatch_command_palette_entry(entry);
}

ActionDispatchResult AppContext::dispatch_command_palette_action(
    std::string action_name) const {
  return runtime.dispatch_command_palette_action(std::move(action_name));
}

void AppContext::set_app_theme(Theme theme) const {
  runtime.set_app_theme(std::move(theme));
}

void AppContext::set_window_theme(
    WindowRuntimeId runtime_id,
    Theme theme) const {
  runtime.set_window_theme(runtime_id, std::move(theme));
}

bool AppContext::clear_window_theme(WindowRuntimeId runtime_id) const {
  return runtime.clear_window_theme(runtime_id);
}

std::optional<Color> AppContext::theme_color(
    WindowRuntimeId runtime_id,
    const ThemeTokenId& id) const {
  return runtime.theme_color(runtime_id, id);
}

std::optional<float> AppContext::theme_spacing(
    WindowRuntimeId runtime_id,
    const ThemeTokenId& id) const {
  return runtime.theme_spacing(runtime_id, id);
}


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
