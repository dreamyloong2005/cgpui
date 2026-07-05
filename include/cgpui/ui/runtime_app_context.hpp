#pragma once

#include "cgpui/app/app_facade.hpp"
#include "cgpui/ui/action.hpp"
#include "cgpui/ui/runtime_actions.hpp"
#include "cgpui/ui/runtime_window_options.hpp"

#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace cgpui {

struct AppContext {
  WindowRuntime& runtime;

  [[nodiscard]] App app() const;
  [[nodiscard]] AppOpenedWindow open_window(WindowOptions options) const;
  [[nodiscard]] AppOpenedWindow open_window(
      WindowOptions options,
      std::unique_ptr<View> root_view) const;
  [[nodiscard]] Result<AppOpenedWindow> try_open_window(
      WindowOptions options) const;
  [[nodiscard]] Result<AppOpenedWindow> try_open_window(
      WindowOptions options,
      std::unique_ptr<View> root_view) const;
  [[nodiscard]] NativeMenuInstallation install_native_menu(
      NativeMenuModel menu) const;
  [[nodiscard]] Result<NativeMenuInstallation> try_install_native_menu(
      NativeMenuModel menu) const;
  [[nodiscard]] NativeFileDialogResult show_native_file_dialog(
      NativeFileDialogOptions options) const;
  [[nodiscard]] Result<NativeFileDialogResult> try_show_native_file_dialog(
      NativeFileDialogOptions options) const;
  void register_command_palette_entry(CommandPaletteEntry entry) const;
  template <Action T>
  void register_command_palette_entry(CommandPaletteEntry entry) const;
  [[nodiscard]] std::span<const CommandPaletteEntry> command_palette_entries()
      const;
  [[nodiscard]] std::vector<CommandPaletteEntry>
  command_palette_entries_for_group(std::string_view group) const;
  [[nodiscard]] ActionDispatchResult dispatch_command_palette_entry(
      const CommandPaletteEntry& entry) const;
  [[nodiscard]] ActionDispatchResult dispatch_command_palette_action(
      std::string action_name) const;
  void set_app_theme(Theme theme) const;
  void set_window_theme(WindowRuntimeId runtime_id, Theme theme) const;
  [[nodiscard]] bool clear_window_theme(WindowRuntimeId runtime_id) const;
  [[nodiscard]] std::optional<Color> theme_color(
      WindowRuntimeId runtime_id,
      const ThemeTokenId& id) const;
  [[nodiscard]] std::optional<float> theme_spacing(
      WindowRuntimeId runtime_id,
      const ThemeTokenId& id) const;

  template <typename T>
  void set_global(T global_value) const;

  template <typename T>
  [[nodiscard]] const T* global() const;

  template <typename T, typename Update>
  bool update_global(Update&& update) const;
};

} // namespace cgpui
