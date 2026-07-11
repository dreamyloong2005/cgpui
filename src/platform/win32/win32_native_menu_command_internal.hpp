#pragma once

#include "cgpui/core/event_native_menu.hpp"
#include "cgpui/platform/platform_native_menu.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace cgpui {

class Win32NativeMenuCommandMap {
 public:
  [[nodiscard]] static std::shared_ptr<const Win32NativeMenuCommandMap> build(
      const NativeMenuModel& model);
  [[nodiscard]] std::optional<NativeMenuCommand> resolve(
      UINT command_id,
      NativeMenuCommandSource source) const;

 private:
  explicit Win32NativeMenuCommandMap(std::vector<std::string> action_names);
  std::vector<std::string> action_names_;
};

} // namespace cgpui
