#pragma once

#include <cstdint>
#include <string>

namespace cgpui {

enum class NativeMenuCommandSource {
  menu,
  accelerator,
};

struct NativeMenuCommand {
  std::uint32_t command_id = 0;
  std::string action_name;
  NativeMenuCommandSource source = NativeMenuCommandSource::menu;
};

} // namespace cgpui
