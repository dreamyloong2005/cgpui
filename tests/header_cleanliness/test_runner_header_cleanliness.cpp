#include "cgpui/ui/test_runner.hpp"

#include <type_traits>

void header_test(cgpui::TestApp&, cgpui::TestRunSeed) {}

static_assert(std::is_aggregate_v<cgpui::TestRunOptions>);
static_assert(std::is_aggregate_v<cgpui::TestRunSummary>);
static_assert(requires {
  cgpui::run_test("header", &header_test, cgpui::TestRunOptions{});
});

int main() { return 0; }
