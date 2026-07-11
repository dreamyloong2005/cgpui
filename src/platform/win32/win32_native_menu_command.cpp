#include "win32_native_menu_accelerator_table_internal.hpp"
#include "win32_native_menu_command_internal.hpp"

#include <utility>

namespace cgpui {
namespace {

void append_command_actions(
    const std::vector<NativeMenuItem>& items,
    std::vector<std::string>& action_names) {
  for (const NativeMenuItem& item : items) {
    if (item.kind == NativeMenuItemKind::submenu) {
      append_command_actions(item.children, action_names);
    } else if (item.kind == NativeMenuItemKind::command) {
      action_names.push_back(item.action_name);
    }
  }
}

} // namespace

Win32NativeMenuCommandMap::Win32NativeMenuCommandMap(
    std::vector<std::string> action_names)
    : action_names_(std::move(action_names)) {}

std::shared_ptr<const Win32NativeMenuCommandMap>
Win32NativeMenuCommandMap::build(const NativeMenuModel& model) {
  std::vector<std::string> action_names;
  action_names.reserve(native_menu_item_count(model));
  append_command_actions(model.items, action_names);
  return std::shared_ptr<const Win32NativeMenuCommandMap>(
      new Win32NativeMenuCommandMap(std::move(action_names)));
}

std::optional<NativeMenuCommand> Win32NativeMenuCommandMap::resolve(
    UINT command_id,
    NativeMenuCommandSource source) const {
  if (command_id < win32_native_menu_first_command_id) return std::nullopt;
  const std::size_t index = command_id - win32_native_menu_first_command_id;
  if (index >= action_names_.size() || action_names_[index].empty()) {
    return std::nullopt;
  }
  return NativeMenuCommand{
      .command_id = command_id,
      .action_name = action_names_[index],
      .source = source,
  };
}

} // namespace cgpui
