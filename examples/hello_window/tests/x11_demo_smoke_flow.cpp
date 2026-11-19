#include "hello_window_case.hpp"

int main() {
  return run_hello_window_case({
      {"CGPUI_LINUX_BACKEND", "x11"},
      {"CGPUI_DEMO_SMOKE_FLOW", "1"},
      {"CGPUI_CLOSE_AFTER_FIRST_FRAME", "0"},
  });
}
