#pragma once

#include "cgpui/core/event_platform.hpp"

#include <cstddef>
#include <string>

namespace cgpui {

enum class PlatformDiagnosticKind {
  unknown,
  clipboard,
  drag_drop,
  ime,
  accessibility,
  window_lifecycle,
  menu,
  file_dialog,
  message_dialog,
  open_url,
  reopen,
  window_chrome,
};

struct PlatformDiagnosticEvent {
  PlatformDiagnosticKind kind = PlatformDiagnosticKind::unknown;
  EventKind event_kind = EventKind::unknown;
  std::string backend;
  std::string operation;
  bool supported = true;
  bool succeeded = true;
  bool cancelled = false;
  std::size_t value_count = 0;
  int sequence = 0;
};

} // namespace cgpui
