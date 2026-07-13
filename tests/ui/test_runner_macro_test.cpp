#include "cgpui/ui/test_runner.hpp"

#include <stdexcept>

CGPUI_TEST(
    test_macro_injects_contexts,
    cgpui::TestApp& first,
    cgpui::TestApp& second,
    cgpui::TestRunSeed seed) {
  if (first.read_from_clipboard().has_value() ||
      second.read_from_clipboard().has_value() ||
      !first.write_to_clipboard("macro isolation") ||
      second.read_from_clipboard().has_value()) {
    throw std::runtime_error("macro TestApp injection was not isolated");
  }
  if (seed.value != 0) throw std::runtime_error("unexpected default seed");
}
