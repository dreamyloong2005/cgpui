// Explicit experiment executable: intentionally absent from [test] discover.
// A native rejection or finite pending result is data, not universal lack of support.
#include <cstdio>
#include <string_view>
#ifdef _WIN32
#include "dx12_composition_probe_test_support.hpp"
#endif

int main(int argc, char** argv) {
#ifdef _WIN32
  if (argc == 2 && std::string_view(argv[1]) == "--hidden")
    return cgpui_test::run_composition_probe(false);
  if (argc == 2 && std::string_view(argv[1]) == "--visible")
    return cgpui_test::run_composition_probe(true);
  std::fprintf(stderr, "usage: dx12_composition_pacing_probe --hidden|--visible\n");
#else
  (void)argc; (void)argv;
  std::fprintf(stderr, "composition pacing probe requires native Windows; NOT measured\n");
#endif
  return 64;
}
