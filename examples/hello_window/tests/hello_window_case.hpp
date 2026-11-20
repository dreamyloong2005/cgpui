// One hello_window smoke case: set the environment variables the case names,
// then run examples/hello_window/main.cpp. Each case is its own test program
// (mcpp gives a test no per-case environment).
#pragma once

// main.cpp's own includes come first, so the rename below reaches only its
// main function.
#include "cgpui/cgpui.hpp"

#include <cstdlib>
#include <expected>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

#define main hello_window_main
#include "../main.cpp"
#undef main
// The Windows ASan build compiles with /Dmain=cgpui_asan_main
// (scripts/dev/asan_entry_windows.cpp); the #undef above removed it, so the
// case's own main is renamed again.
#if defined(_WIN32) && defined(CGPUI_ASAN)
#define main cgpui_asan_main
#endif

inline int run_hello_window_case(
    std::initializer_list<std::pair<const char*, const char*>> environment) {
  for (const auto& [name, value] : environment) {
#if defined(_WIN32)
    if (_putenv_s(name, value) != 0) {
      return 90;
    }
#else
    if (setenv(name, value, 1) != 0) {
      return 90;
    }
#endif
  }
  return hello_window_main();
}
