#include "macos_application_internal.hpp"

namespace cgpui {

int macos_run_event_loop(bool& launched) {
  if (!launched) {
    [NSApp finishLaunching];
    launched = true;
  }
  [NSApp run];
  return 0;
}

void macos_stop_event_loop() { [NSApp stop:nil]; }

int MacOSApplication::run() {
  running_ = true;
  return macos_run_event_loop(launched_);
}

void MacOSApplication::quit() {
  running_ = false;
  macos_stop_event_loop();
  [NSApp terminate:nil];
}

}  // namespace cgpui
