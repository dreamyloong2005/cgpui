// Sets a process environment variable for the rest of a test program.
// A test program states the environment it needs itself: mcpp runs every
// test with the environment of `mcpp test`, with no per-test variables.
#pragma once

#include <cstdlib>

inline bool set_test_environment(const char* name, const char* value) {
#if defined(_WIN32)
  return _putenv_s(name, value) == 0;
#else
  return setenv(name, value, 1) == 0;
#endif
}
