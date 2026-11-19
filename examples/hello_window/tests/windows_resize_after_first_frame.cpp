#include "hello_window_case.hpp"

int main() {
  return run_hello_window_case({
      {"CGPUI_RESIZE_AFTER_FIRST_FRAME", "1"},
  });
}
