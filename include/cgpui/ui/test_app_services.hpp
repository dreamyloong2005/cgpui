#pragma once

#include "cgpui/platform/platform_message_dialog.hpp"

#include <cstddef>
#include <optional>
#include <string>

namespace cgpui {

struct TestPlatformServiceSnapshot {
  std::size_t native_menu_install_count = 0;
  std::size_t file_dialog_count = 0;
  std::size_t message_dialog_count = 0;
  std::size_t open_url_count = 0;
  std::size_t reopen_count = 0;
  std::size_t quit_count = 0;
  std::size_t pending_path_responses = 0;
  std::size_t pending_prompt_responses = 0;
  std::optional<std::string> opened_url;
};

} // namespace cgpui
